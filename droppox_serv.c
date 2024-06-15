#include "dp_lib/dp_lib_serv.h"
#include "dp_lib/dp_lib_common.h"

int main(void) {
    const char *msg = "*** DropPox Server v1.0 ***\n1. Start Server\n2. Admin Menu\n3. Exit\n";

    while(1) {
        int sel = getintsel(msg, 1, 3); // 1~3 사이 값을 받음이 보장됨

        if(sel == 1) {
            int stat = startmenu();

            if(stat == 0) {
                break;
            }
            else if(stat == 1) {
                continue;
            }
            else {
                help();
                return -1;
            }
        }
        else if(sel == 2) {
            adminmenu();
        }
        else { // 3 선택 - 사용자가 취소한 경우
            printf("* User canceled, abort *\n");
            return 1;
        }
    }

    system("clear");

    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo *bind_address;
    getaddrinfo(0, "6880", &hints, &bind_address); // 'D' = 68, 'P' = 80;


    printf("Creating socket...\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family,
                           bind_address->ai_socktype, bind_address->ai_protocol);
    if(!ISVALIDSOCKET(socket_listen)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Binding socket to local address...\n");
    if(bind(socket_listen, bind_address->ai_addr,
            bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(bind_address);


    printf("Listening...\n");
    if(listen(socket_listen, 1) < 0) { // 유일한 클라이언트
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Waiting for connections...\n");

    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    SOCKET sock_clnt = accept(socket_listen,
                                  (struct sockaddr *) &client_address, &client_len);
    if(!ISVALIDSOCKET(sock_clnt)) {
        fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    char address_buffer[100];
    getnameinfo((struct sockaddr *) &client_address,
                client_len, address_buffer, sizeof(address_buffer), 0, 0,
                NI_NUMERICHOST);
    printf("New connection from %s\n", address_buffer);


    while(1) {
        if(inotmsghdlr(sock_clnt) == -1){
            fprintf(stderr, "Something Wrong\n");
        }
    }

    CLOSESOCKET(socket_listen); // 서버가 최종적으로 종료될 떄, socket_listen을 닫는다.

    printf("Finished.\n");

    return 0;
}