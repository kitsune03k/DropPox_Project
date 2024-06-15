#ifndef DROPPOX_SERV
#define DROPPOX_SERV

#include "dp_lib_common.h"

typedef struct{
    int dno;
    char name[MBUF];
    char path[LBUF];
} dirdir;

void getdpsvpath(char *);

void getdpsvbkuppath(char *);

int servcheck(void);


int startmenu(void);

int initnew(void);


void adminmenu(void);

int backup(void); // adminmenu 1

int restore(void); // adminmenu 2

int clear(void); // adminmenu 3

int dscsort(const void *, const void *); // 정렬 시 사용


int inotmsghdlr(SOCKET);

int receiver1();

int receiver2();

int receiver3();

#endif