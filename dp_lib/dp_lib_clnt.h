#ifndef DROPPOX_SERV
#define DROPPOX_SERV

#include "dp_lib_common.h"

void getdpclpath(char *);

void getdpclbkuppath(char *);

int init(void);


int inotevthdlr(SOCKET, char*, int);

int sender12(DP_MSG *, DP_MSG *, SOCKET, char *);

int sender3(DP_MSG *, DP_MSG *, SOCKET);

#endif