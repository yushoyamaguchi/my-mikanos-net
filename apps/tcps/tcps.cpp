#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../syscall.h"
#include "../socket.hpp"

extern "C" int
main(int argc, char *argv[])
{
    int soc, acc, peerlen, ret;
    struct sockaddr_in self, peer;
    unsigned char *addr;
    char buf[2048];

    printf("Starting TCP Echo Server\n");
    soc = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (soc == 1) {
        printf("socket: failure\n");
        exit(-1);
    }
    printf("socket: success, soc=%d\n", soc);
    self.sin_family = AF_INET;
    self.sin_addr = INADDR_ANY;
    self.sin_port = hton16(7);
    if (bind(soc, (struct sockaddr *)&self, sizeof(self)) == -1) {
        printf("bind: failure\n");
        close(soc);
        exit(-1);
    }
    addr = (unsigned char *)&self.sin_addr;
    printf("bind: success, self=%d.%d.%d.%d:%d\n", addr[0], addr[1], addr[2], addr[3], ntoh16(self.sin_port));
    if (listen(soc, 10) == -1) {
        printf("listen: failure\n");
        close(soc);
        exit(-1);
    }
    printf("waiting for connection...\n");
    peerlen = sizeof(peer);
    acc = accept(soc,(struct sockaddr *)&peer, &peerlen);
    if (acc == -1) {
        printf("accept: failure\n");
        close(soc);
        exit(-1);
    }
    addr = (unsigned char *)&peer.sin_addr;
    printf("connection established: %d.%d.%d.%d:%d\n", addr[0], addr[1], addr[2], addr[3], ntoh16(peer.sin_port));
    while (1) {
        ret = recv(acc, buf, sizeof(buf));
        if (ret <= 0) {
            printf("EOF\n");
            close(acc);
            break;
        }
        printf("recv: %d bytes data received\n", ret);
        //hexdump(buf, ret);
        send(acc, buf, ret);
    }
    close(soc);  
    exit(-1);
}
