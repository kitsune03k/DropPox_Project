#include "dp_lib/dp_lib_clnt.h"
#include "dp_lib/dp_lib_common.h"

int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "usage: droppox_clnt <server ip>\n");
        return 1;
    }

    printf("*** DropPox Client v1.0 ***\n");
    if(init()) {
        printf("User cancelled, abort\n");
        return 1;
    }


/** INOTIFY INIT **/
    int fd, wd;

    fd = inotify_init();
    if(fd < 0) {
        fprintf(stderr, "** inotify init error\n");
        return -1;
    }

    char dpclpath[MBUF];
    memset(dpclpath, 0, MBUF);
    getdpclpath(dpclpath);

    wd = inotify_add_watch(fd, dpclpath, IN_MODIFY | IN_DELETE);
    if(wd < 0) {
        fprintf(stderr, "** inotify watch error\n");
        return -1;
    }

    printf("Watching directory for events...\n");
/** INOTIFY INIT END **/


    printf("Configuring remote address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *peer_address;
    if(getaddrinfo(argv[1], "6880", &hints, &peer_address)) {  // 'D' = 68, 'P' = 80;
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Remote address is: ");
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
                address_buffer, sizeof(address_buffer),
                service_buffer, sizeof(service_buffer),
                NI_NUMERICHOST);
    printf("%s %s\n", address_buffer, service_buffer);


    printf("Creating socket...\n");
    SOCKET sock_peer;
    sock_peer = socket(peer_address->ai_family,
                       peer_address->ai_socktype, peer_address->ai_protocol);
    if(!ISVALIDSOCKET(sock_peer)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    printf("Connecting...\n");
    if(connect(sock_peer,
               peer_address->ai_addr, peer_address->ai_addrlen)) {
        fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(peer_address);

    printf("Connected.\n");

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;


    char inotbuf[INOT_BUF_LEN]; // 최대 1000개까지 이벤트 쌓을 수 있음

    while(1) {
        int length = read(fd, inotbuf, INOT_BUF_LEN);
        if(length < 0) {
            fprintf(stderr, "** Watch Error **\n");
            return -1;
        }

        /** INOTIFY WATCH **/
        struct inotify_event *event;
        for(char *ptr = inotbuf; ptr < inotbuf + length; \
            ptr += INOT_EVENT_SIZE + event->len) {
            event = (struct inotify_event *) ptr;
            if(event->len) {
                if(event->mask & IN_MODIFY) {
                    printf("iNotify : File modified: %s\n", event->name);
                    if(inotevthdlr(sock_peer, event->name, 2) == -1) {
                        fprintf(stderr, "** Integrity Violation at %s **\n", event->name);
                        continue;
                    }
                }
                if(event->mask & IN_DELETE) {
                    printf("iNotify : File deleted: %s\n", event->name);
                    if(inotevthdlr(sock_peer, event->name, 3) == -1) {
                        fprintf(stderr, "** Integrity Violation at %s **\n", event->name);
                        continue;
                    }
                }
                printf("File synchronized to server: %s\n", event->name);
            }
        }
    }

    printf("Closing socket...\n");
    CLOSESOCKET(sock_peer);

    printf("Close inotify variables...\n");
    inotify_rm_watch(fd, wd
    );
    close(fd);

    printf("Finished.\n");

    return 0;
}