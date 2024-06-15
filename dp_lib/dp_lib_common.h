#ifndef DROPPOX_COMMON
#define DROPPOX_COMMON

// system header
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <openssl/evp.h>
#include <fcntl.h>

// network header
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// io header
#include <stdio.h>
#include <string.h>

// macros
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

#define SBUF    1<<6    // 64
#define MBUF    1<<8   // 256
#define LBUF    1<<10   // 1024

#define INOT_EVENT_SIZE (sizeof(struct inotify_event))
#define INOT_BUF_LEN    (1024 * (INOT_EVENT_SIZE + 16))
#define MAX_SUBDIR      1000

#define BUFFER_SIZE 4096

// command code
#define C_INFO      11
#define C_CREATE    22
#define C_MODIFY    33
#define C_DELETE    44
#define C_EXIT      99

// sync code (to synchronize message order)
#define S_REQ    111
#define S_RES    222

typedef struct __DP_MSG{
    int cmd;            // command mode
    int sync;           // sync code
    char name[MBUF];    // 파일 이름, 서버와 클라이언트는 각각의 DPPATH에서 찾을 것임.
    char hash[65];      // sha256sum 해시값 - 없으면 '0' 65개
} DP_MSG;

/*****************************
 ** 선택지 입력 관련 함수들 **
 *****************************/

int getintsel(const char *, int, int);

int getynsel(const char *);

/*****************************
 ** 값 얻어오는 함수들 **
 *****************************/

void getcurrtime(char *);

void gethomepath(char *);

void gethash(const char*, char*);

/*****************************
 ** directory 관련 함수들 **
 *****************************/

int isdexist(const char *);

int isfexist(const char *);

int mdir(const char *);

int move(const char *, const char *);

int cprf(const char *, const char *);

int rmrf(const char *);

/*****************************
 ** 전송 관련 함수들 **
 *****************************/

int setter(DP_MSG *, int, int, char *, char *);

int iparsr(DP_MSG *, char *);

int parser(const char *, DP_MSG *);

int checker(DP_MSG *);

int losslesssend(FILE *, SOCKET);

int losslessrecv(FILE *, SOCKET);

/*****************************
 ** 그외 **
 *****************************/

void help(void);

void getenter(void);

#endif // DROPPOX_COMMON