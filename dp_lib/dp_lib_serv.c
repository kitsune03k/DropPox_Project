#include "dp_lib_serv.h"

void getdpsvpath(char *buf) {
    memset(buf, 0, MBUF);

    char home[MBUF];
    memset(home, 0, MBUF);
    gethomepath(home);

    sprintf(buf, "%s/DropPoxServ", home);
}

void getdpsvbkuppath(char *buf) {
    memset(buf, 0, MBUF);

    char home[MBUF];
    memset(home, 0, MBUF);
    gethomepath(home);

    sprintf(buf, "%s/DropPoxServ_BKUP", home);
}


int servcheck(void) { // 데이터 있으면 0, 없으면 -1
    char dpsv[MBUF];
    getdpsvpath(dpsv);

    if(!isdexist(dpsv)) {
        return -1;
    }

    return 0;
}

int startmenu(void) {
    char dpsv[MBUF];
    getdpsvpath(dpsv);

    int prevData;

    const char *msg = "[Start Server]\n1. Init server with previous data\n2. Init server with new data\n3. Back to "
                      "previous menu\n";

    while(1) {
        prevData = servcheck();

        int sel = getintsel(msg, 1, 3);

        if(sel == 1) { // 기존의 데이터로 시작하려는데
            if(prevData == 0) { // servcheck에서 데이터 있다 할때
                return 0;
            }
            else {
                printf("** Data not exist! **\nRecover in [Admin Menu] or init with new data\n");
            }
        }
        else if(sel == 2) { // 새로운 데이터로 시작하려는데
            if(prevData != 0) { // servcheck에서 데이터 없다 할때
                return initnew(); // 결국 문제 없으면 return 0
            }
            else {
                printf("** Data already exist! **\nBack up and clean in [Admin Menu]\n");
            }
        }
        else { // sel == 3
            return 1;
        }

        getenter();
    }
}

int initnew() { // 새로 만듦 -- 기존 데이터 없는 상황 전제됨
    char dpsv[MBUF];
    getdpsvpath(dpsv);

    if(mdir(dpsv) == -1) {
        return -1;
    }

    return 0;
}

void adminmenu(void) {
    char *msg = "[Admin Mode]\n1. Backup\n2. Restore\n3. Clear\n4. Back to previous menu\n";

    while(1) {
        int sel = getintsel(msg, 1, 4);
        if(sel == 1) {
            backup();
        }
        else if(sel == 2) {
            restore();
        }
        else if(sel == 3) {
            clear();
        }
        else {
            break;
        }
    }
}

int backup(void) {
    char src[MBUF];
    getdpsvpath(src);

    if(!isdexist(src)) {
        fprintf(stderr, "** Nothing to backup! **\n");
        getenter();
        return -1;
    }

    char dpsvbkup[MBUF];
    getdpsvbkuppath(dpsvbkup);

    if(!isdexist(dpsvbkup)) { // ~/DropPoxServ_BKUP 없는 처음이라면 하나 만들어줌
        mdir(dpsvbkup);
    }

    char time[SBUF];
    getcurrtime(time);

    char dst[MBUF];
    memset(dst, 0, MBUF);
    sprintf(dst, "%s/%s", dpsvbkup, time);

    if(move(src, dst)) {
        return -1;
    }

    printf("Backup complete!\nData saved in %s\n", dst);
    getenter();

    return 0;
}

int restore(void) {
    char dpsv[MBUF];
    getdpsvpath(dpsv);

    char dpsvbkup[MBUF];
    getdpsvbkuppath(dpsvbkup);

    if(!isdexist(dpsvbkup)) { // ~/DropPox_BKUP 없을 경우
        fprintf(stderr, "** Nothing to Restore! **\n");
        return -1;
    }

    struct dirent *entry;
    DIR *dp = opendir(dpsvbkup);

    dirdir subdirs[MAX_SUBDIR];
    memset(subdirs, 0, sizeof(dirdir) * MAX_SUBDIR);

    printf("** Backup List **\n");
    int count = 0;
    while((entry = readdir(dp)) != NULL) {
        if(strncmp(entry->d_name, ".", 1) == 0) {
            continue;
        }

        sprintf(subdirs[count].name, "%s", entry->d_name);
        sprintf(subdirs[count].path, "%s/%s", dpsvbkup, subdirs[count].name);

        count++;
    }

    closedir(dp);

    qsort(subdirs, count, sizeof(dirdir), dscsort);

    for(int i = 0; i < count; i++) {
        printf("[%4d] %s\n", i + 1, subdirs[i].name);
    }

    char *msg = ("Select data to recover, previous data will be deleted\n0 to back to previous menu\n");
    int sel = getintsel(msg, 1, count);
    if(sel == 0) {
        return 0;
    }
    else {
        rmrf(dpsv);
        cprf(subdirs[sel - 1].path, dpsv);
    }

    printf("Data recovered with data [%s]\n", subdirs[sel - 1].name);

    return 0;
}

int clear(void) {
    char dpsv[MBUF];
    getdpsvpath(dpsv);

    char *msg = "** ALL DATA WILL BE LOST **\nContinue?\n";
    if(getynsel(msg)) {
        if(rmrf(dpsv)) {
            return -1;
        }
        printf("Data cleared!\nStart with new data in [Start Server]\n");
    }
    else {
        printf("Clear cancelled.\n");
    }

    getenter();

    return 0;
}

int dscsort(const void *dd1, const void *dd2) {
    const dirdir *x1 = (const dirdir *) dd1;
    const dirdir *x2 = (const dirdir *) dd2;

    int cmprslt = strcmp(x1->name, x2->name);
    if(cmprslt == 0) {
        return 0;
    }
    else {
        if(cmprslt > 0) {
            return -1;
        }
        else {
            return 1;
        }
    }
}


int inotmsghdlr(SOCKET sock_clnt) {
    // 2. 서버에서 이벤트가 발생한 파일이 어떤 상태인지 메세지 받음
    char rbuf1[LBUF];
    memset(rbuf1, 0, LBUF);
    recv(sock_clnt, rbuf1, LBUF, 0);
    DP_MSG r1;
    parser(rbuf1, &r1);

    char dpsv[MBUF];
    getdpsvpath(dpsv);
    char filepath[MBUF];
    memset(filepath, 0, MBUF);
    sprintf(filepath, "%s/%s", dpsv, r1.name);
    char hash[65];
    gethash(filepath, hash);

    // 3. 서버가 가진 파일의 정보를 보냄
    DP_MSG s1;
    setter(&s1, r1.cmd, S_RES, r1.name, hash);

    char sbuf1[LBUF];
    iparsr(&s1, sbuf1);
    send(sock_clnt, sbuf1, LBUF, 0);

    // 6. 클라이언트가 서버에 할 작업을 통보받음
    char rbuf2[LBUF];
    memset(rbuf2, 0, LBUF);
    recv(sock_clnt, rbuf2, LBUF, 0);
    DP_MSG r2;
    parser(rbuf2, &r2);

    // 7. 서버가 요청 들어온 작업을 준비하고 메세지 보냄 or 진행하고 종료
    DP_MSG s2;
    setter(&s2, r2.cmd, S_RES, r2.name, r2.hash);
    char sbuf2[LBUF];
    iparsr(&s2, sbuf2);

    FILE * fp;
    if(r2.cmd == C_MODIFY) {
        fp = fopen(filepath, "wb");
    }
    else if(r2.cmd == C_DELETE) {
        if(rmrf(filepath) == 0) {
            printf("File Deleted: %s\n", r2.name);
            return 0; /** 삭제 시 여기서 종료 **/
        }
        else {
            return -1;
        }
    }
    else { // C_EXIT
        return 0;
    }
    send(sock_clnt, sbuf2, LBUF, 0);

    // 10. losslessrecv();
    losslessrecv(fp, sock_clnt);
    fclose(fp);

    // 11. 받은 파일 해시 계산하여 전송
    char hashverify[65];
    gethash(filepath, hashverify);

    DP_MSG s3;
    setter(&s3, C_INFO, S_REQ, r1.name, hashverify);
    char sbuf3[LBUF];
    iparsr(&s3, sbuf3);
    send(sock_clnt, sbuf3, LBUF, 0);

    printf("File Created or Modified: %s\n", r2.name);
    return 0;
}