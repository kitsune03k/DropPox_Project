#include "dp_lib_common.h"

/*****************************
 ** 선택지 입력 관련 함수들 **
 *****************************/

/** getintsel, getynsel argv msg 꼭 마지막 개행된 상태로 받을 것 **/
int getintsel(const char *msg, int start, int end) {
    // atoi 함수가 0으로 예외처리가 안되는것은 익히 알고있음
    // 따라서 return 값의 범위인 [start, end] 사이에 0이 포함되면 안된다
    // 0과 1을 return 해야 하는 경우 아래 getynsel을 사용

    system("clear");

    char cSel[MBUF];
    int iSel;

    while(1) {
        memset(cSel, 0, MBUF);

        printf("%s", msg);
        scanf("%s", cSel);
        getchar();

        iSel = atoi(cSel);

        if(!iSel) { // atoi 결과가 0일때 -- 0은 무조건 오류로 간주
            printf("Input Correctly!\n\n");
        }
        else {
            if(iSel < start || iSel > end) { // 범위에 안 맞을 경우
                printf("Input Correctly!\n\n");
            }
            else {
                return iSel;
            }
        }
    }
}

/** 다른 함수들과 다르게, user context 함수들은 값 그 자체에 의미 부여
 *  따라서 0이 문제 없음이 아닌 진짜 0=false, 1이 문제 있음이 아닌 진짜 1=true **/
int getynsel(const char *msg) {
    system("clear");

    char cSel[MBUF];

    while(1) {
        memset(cSel, 0, MBUF);

        printf("%s", msg);
        printf("(Y/N) to continue or abort\n");
        scanf("%s", cSel);
        getchar();

        if(!strcasecmp(cSel, "y") || !strcasecmp(cSel, "n")) { // y, Y, n, N일 경우
            if(!strcasecmp(cSel, "y")) { // y, Y일 경우
                return 1;
            }
            else { // n, N일 경우
                return 0;
            }
        }
        else {
            printf("Input Correctly!\n\n");
        }
    }
}


/*****************************
 ** 값 얻어오는 함수들 **
 *****************************/

// 현재시간 지정 양식대로 버퍼에 작성
void getcurrtime(char *buf) {
    memset(buf, 0, SBUF);
    time_t linuxTime;
    time(&linuxTime);
    struct tm *localTime = localtime(&linuxTime);

    strftime(buf, SBUF, "%y%m%d_%H%M%S", localTime);
}

// 홈 env path 기록
void gethomepath(char *buf) {
    const char *homeEnv = getenv("HOME");

    memset(buf, 0, MBUF);
    strcpy(buf, homeEnv);
}

// sha256hash 값 기록
void gethash(const char *filepath, char *outputBuffer) {
    if(!isfexist(filepath)) {
        memset(outputBuffer, 0, 65);
        return;
    }

    FILE *file = fopen(filepath, "rb");
    if(!file) {
        perror("File opening failed");
        return;
    }

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if(mdctx == NULL) {
        perror("Failed to create MD context");
        fclose(file);
        return;
    }

    if(EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) {
        perror("Failed to initialize digest");
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        return;
    }

    const int bufSize = 32768;
    unsigned char *buffer = malloc(bufSize);
    if(!buffer) {
        perror("Unable to allocate buffer");
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        return;
    }

    size_t bytesRead = 0;
    while((bytesRead = fread(buffer, 1, bufSize, file)) > 0) {
        if(EVP_DigestUpdate(mdctx, buffer, bytesRead) != 1) {
            perror("Failed to update digest");
            free(buffer);
            EVP_MD_CTX_free(mdctx);
            fclose(file);
            return;
        }
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;
    if(EVP_DigestFinal_ex(mdctx, hash, &lengthOfHash) != 1) {
        perror("Failed to finalize digest");
        free(buffer);
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        return;
    }

    EVP_MD_CTX_free(mdctx);
    fclose(file);
    free(buffer);

    for(unsigned int i = 0; i < lengthOfHash; i++) {
        sprintf((char *) &outputBuffer[i * 2], "%02x", hash[i]);
    }
    outputBuffer[64] = 0;
}


/*****************************
 ** directory 관련 함수들 **
 *****************************/

// 폴더 있는지 확인
int isdexist(const char *path) {
    struct stat buf;

    // stat은 파일이든 폴더든 일단 경로상 있기만 하면 return 0
    if(stat(path, &buf) == -1) {
        return 0;
    }

    if(S_ISDIR(buf.st_mode) && !S_ISREG(buf.st_mode)) {
        return 1;
    }
    else {
        return -1;
    }
}

// 파일 있는지 확인
int isfexist(const char *path) {
    struct stat buf;

    // stat은 파일이든 폴더든 일단 경로상 있기만 하면 return 0
    if(stat(path, &buf) == -1) {
        return 0;
    }

    if(S_ISREG(buf.st_mode) && !S_ISDIR(buf.st_mode)) {
        return 1;
    }
    else {
        return 0;
    }
}

/** system() 함수들 원칙
 * 문제 없을 경우 0, 문제 있을 경우 -1 return
 * -> 다른 함수에서 일관성
 *
 *  ex) exception handler
 *
 *  if(somefx()){ // somefx()가 0이 아닌 값(=문제 있음)을 return할 경우
 *      ~~예외처리~~;
 *      return -1; // 문제 있음 반환
 *  }
 *
 **/

int mdir(const char *path) {
    char cmd[MBUF];
    memset(cmd, 0, MBUF);
    sprintf(cmd, "mkdir -m 755 %s", path);

    if(system(cmd)) {
        fprintf(stderr, "mkdir error\n");
        return -1;
    }

    return 0;
}

int move(const char *pathsrc, const char *pathdst) {
    char cmd[MBUF];
    memset(cmd, 0, MBUF);
    sprintf(cmd, "mv %s %s/", pathsrc, pathdst);

    if(system(cmd)) {
        fprintf(stderr, "copydir error\n");
        return -1;
    }

    return 0;
}

int cprf(const char *pathsrc, const char *pathdst) {
    char cmd[MBUF];
    memset(cmd, 0, MBUF);
    sprintf(cmd, "cp -rf %s/ %s/", pathsrc, pathdst);

    if(system(cmd)) {
        fprintf(stderr, "cp error\n");
        return -1;
    }

    return 0;
}

int rmrf(const char *pathrm) {
    char cmd[MBUF];
    memset(cmd, 0, MBUF);
    sprintf(cmd, "rm -rf %s", pathrm);

    if(system(cmd)) {
        fprintf(stderr, "rm error\n");
        return -1;
    }

    return 0;
}


/*****************************
 ** 전송 관련 함수들 **
 *
 * DP_MSG 구조
 * --------------------------
 * |int     cmd        |\lf|
 * |int     status      |\lf|
 * |char    name[256]   |\lf|
 * |char    hash[65]        |
 * --------------------------
 *
 *****************************/



// 주어진 정보 DP_MSG에 작성
int setter(DP_MSG *dm, int c, int s, char *n, char *h) {
    memset(dm, 0, sizeof(DP_MSG));

    dm->cmd = c;
    dm->sync = s;
    strcpy(dm->name, n);
    strcpy(dm->hash, h);

    return 0;
}

// 구조체 -> 메세지 buffer (역파서)
int iparsr(DP_MSG *dm, char *wbuf) {
    memset(wbuf, 0, LBUF);
    sprintf(wbuf, "%d\n%d\n%s\n%s", dm->cmd, dm->sync, dm->name, dm->hash);
    return 0;
}

// 메세지 buffer -> 구조체 (파서)
int parser(const char *rbuf, DP_MSG *dm) {
    memset(dm, 0, sizeof(DP_MSG));
    sscanf(rbuf, "%d\n%d\n%s\n%s", &dm->cmd, &dm->sync, dm->name, dm->hash);

    return 0;
}

// 해시값으로 원격지에 파일 있는지 확인
int checker(DP_MSG *dm) {
    for(int i = 0; i < 64; i++) {
        if(dm->hash[i] != 0) {
            return 1;
        } // 해시값이 하나라도 '0'이 아니면 파일 있음
    }
    return 0; // 해시값이 전부 '0'이면 파일 없음
}

unsigned long long getfilesize(FILE* fp){
    unsigned long long fpos;
    unsigned long long fsize;
    fpos=ftell(fp);

    fseek(fp, 0, SEEK_END);
    fsize=ftell(fp);
    fseek(fp, fpos, SEEK_SET);

    return fsize;
}

int losslesssend(FILE * fp, SOCKET s){
    unsigned long long fsize = getfilesize(fp);

    char sbuf[21];
    memset(sbuf, 0, 21);
    sprintf(sbuf, "%llu", fsize);
    send(s, sbuf, 21, 0);

    char rbuf[21];
    memset(rbuf, 0, 21);
    recv(s, rbuf, 21, 0);

    char one[1];
    if(strncmp(sbuf, rbuf, 21) == 0){
        for(int i=0; i<fsize; i++){
            fread(one, 1, 1, fp);
            send(s, one, 1, 0);
        }
        return 0;
    }
    else{
        return -1;
    }
}

int losslessrecv(FILE * fp, SOCKET s){
    unsigned long long fsize = 0;

    char rbuf[21];
    memset(rbuf, 0, 21);
    recv(s, rbuf, 21, 0);
    sscanf(rbuf, "%llu", &fsize);

    char sbuf[21];
    memset(sbuf, 0, 21);
    sprintf(sbuf, "%llu", fsize);
    send(s, sbuf, 21, 0);

    char one[1];
    for(int i=0; i<fsize; i++){
        recv(s, one, 1, 0);
        fwrite(one, 1, 1, fp);
    }

    return 0;
}

/*****************************
 ** 그외 **
 *****************************/

void getenter(void) {
    printf("Press ENTER to continue");

    while(getchar() != '\n') {
    }
}

void help(void) {
    fprintf(stderr, "** CRITICAL ERROR **\nsupport contact : kitsune03k@icloud.com");
}