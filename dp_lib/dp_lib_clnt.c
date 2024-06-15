#include "dp_lib_clnt.h"

void getdpclpath(char *buf) {
    memset(buf, 0, MBUF);

    char home[MBUF];
    memset(home, 0, MBUF);
    gethomepath(home);

    sprintf(buf, "%s/DropPox", home);
}

void getdpclbkuppath(char *buf) {
    memset(buf, 0, MBUF);

    char home[MBUF];
    memset(home, 0, MBUF);
    gethomepath(home);

    sprintf(buf, "%s/DropPox_BKUP", home);
}

int init(void) {
    char *msg = "Start client?\n";
    int sel = getynsel(msg);

    if(!sel) {
        return 1;
    }

    char dpclbkup[MBUF];
    getdpclbkuppath(dpclbkup);

    if(!isdexist(dpclbkup)) { // ~/DropPox_BKUP 없는 처음이라면 하나 만들어줌
        mdir(dpclbkup);
    }

    char src[MBUF];
    getdpclpath(src);

    if(isdexist(src)) { // ~/DropPox 있으면
        char now[SBUF];
        memset(now, 0, SBUF);
        getcurrtime(now);

        char dst[MBUF];
        memset(dst, 0, MBUF);
        sprintf(dst, "%s/%s", dpclbkup, now);

        if(move(src, dst)) {
            return -1; // 폴더 그대로 이동, ~/DropPox 새로 만들어줘야함
        }

        printf("Previous data saved in %s\n", dst);
    }

    if(mdir(src) == -1) {
        return -1;
    }


    return 0;
}


int inotevthdlr(SOCKET sock_peer, char *fname, int mode) {
    char dpclpath[MBUF];
    getdpclpath(dpclpath);

    char filepath[MBUF];
    memset(filepath, 0, MBUF);
    sprintf(filepath, "%s/%s", dpclpath, fname);

    char hash[65];
    DP_MSG s1;
    gethash(filepath, hash);
    setter(&s1, C_INFO, S_REQ, fname, hash);

    // 1. 새로운 이벤트와 관련된 파일의 정보 서버로 전송
    char sbuf1[LBUF];
    iparsr(&s1, sbuf1);
    send(sock_peer, sbuf1, LBUF, 0);

    // 4. 해당 파일이 서버에서 어떤 상태인지 받음
    char rbuf1[LBUF];
    memset(rbuf1, 0, LBUF);
    recv(sock_peer, rbuf1, LBUF, 0);
    DP_MSG r1;
    parser(rbuf1, &r1);

    if(mode < 3) {
        return sender12(&s1, &r1, sock_peer, filepath);
    }
    else {
        return sender3(&s1, &r1, sock_peer);
    }
}

// C_CREATE, C_MODIFY
int sender12(DP_MSG *s1, DP_MSG *r1, SOCKET sock_peer, char *filepath) {
    // 5. 서버에게 할 작업을 암시
    DP_MSG s2;
    char sbuf2[LBUF];

    if(checker(r1)) { // 서버에 파일 있으면
        if(strncmp(s1->hash, r1->hash, 64) == 0) { // 같은 파일이면 C->EXIT
            setter(&s2, C_EXIT, S_REQ, s1->name, s1->hash);
            iparsr(&s2, sbuf2);
            send(sock_peer, sbuf2, LBUF, 0);
            return 0; // 서버는 해당 메세지 받으면 종료할것임, 따라서 클라이언트는 그냥 종료해도 됨
        }
        else { // 다른 파일이면 -> C_MODIFY
            setter(&s2, C_MODIFY, S_REQ, s1->name, s1->hash);
        }
    }
    else { // 서버에 파일 없으면 -> C_MODIFY
        setter(&s2, C_MODIFY, S_REQ, s1->name, s1->hash);
    }
    iparsr(&s2, sbuf2);
    send(sock_peer, sbuf2, LBUF, 0);

    // 8. 서버로부터 준비 상태 보고받음
    char rbuf2[LBUF];
    memset(rbuf2, 0, LBUF);
    recv(sock_peer, rbuf2, LBUF, 0);
    DP_MSG r2;
    parser(rbuf2, &r2);

    // 9. losslesssend()
    FILE *fp = fopen(filepath, "rb");
    losslesssend(fp, sock_peer);
    fclose(fp);

    // 12. 해시값 비교
    char rbuf3[LBUF];
    memset(rbuf3, 0, LBUF);
    recv(sock_peer, rbuf3, LBUF, 0);

    DP_MSG r3;
    parser(rbuf3, &r3);

    char local[65], remote[65];
    strncpy(local, s1->hash, 64);
    strncpy(remote, r3.hash, 64);
    local[64] = 0;
    remote[64] = 0;

    if(strcmp(local, remote) == 0) {
        return 0;
    }
    else {
        return -1;
    }
}

// C_DELETE
int sender3(DP_MSG *s1, DP_MSG *r1, SOCKET sock_peer) {
    // 5. 서버에게 할 작업을 암시
    DP_MSG s2;
    char sbuf2[LBUF];

    if(!checker(r1)) { // 서버에 파일 없으면
        setter(&s2, C_EXIT, S_REQ, s1->name, s1->hash);
        iparsr(&s2, sbuf2);
        send(sock_peer, sbuf2, LBUF, 0);
        return 0; // 서버는 해당 메세지 받으면 종료할것임, 따라서 클라이언트는 그냥 종료해도 됨
    }
    else { // 서버에 파일 있으면 - 해시값 같던 말던 무조건 날림
        setter(&s2, C_DELETE, S_REQ, s1->name, s1->hash);
    }
    iparsr(&s2, sbuf2);
    send(sock_peer, sbuf2, LBUF, 0);

    return 0;
}
