#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "../kernel/net/sockio.h"
#include "../kernel/net/if.h"

#define PF_UNSPEC   0
#define PF_LOCAL    1
#define PF_INET     2

#define AF_UNSPEC   PF_UNSPEC
#define AF_LOCAL    PF_LOCAL
#define AF_INET     PF_INET

#define SOCK_STREAM 1
#define SOCK_DGRAM  2

#define IPPROTO_TCP 0
#define IPPROTO_UDP 0

#define INADDR_ANY ((ip_addr_t)0)

typedef uint32_t ip_addr_t;

struct socket {
    int type;
    int desc;
};

struct sockaddr {
    unsigned short sa_family;
    char sa_data[14];
};

struct sockaddr_in {
    unsigned short sin_family;
    uint16_t sin_port;
    ip_addr_t sin_addr;
};

#define IFNAMSIZ 16

struct ifreq {
    char ifr_name[IFNAMSIZ]; /* Interface name */
    union {
        struct sockaddr ifr_addr;
        struct sockaddr ifr_dstaddr;
        struct sockaddr ifr_broadaddr;
        struct sockaddr ifr_netmask;
        struct sockaddr ifr_hwaddr;
        short           ifr_flags;
        int             ifr_ifindex;
        int             ifr_metric;
        int             ifr_mtu;
//      struct ifmap    ifr_map;
        char            ifr_slave[IFNAMSIZ];
        char            ifr_newname[IFNAMSIZ];
        char           *ifr_data;
    };
};

extern int
ip_addr_pton (const char *p, ip_addr_t *n);
extern char *
ip_addr_ntop (const ip_addr_t *n, char *p, size_t size);
extern uint16_t
hton16(uint16_t h);
extern uint16_t
ntoh16(uint16_t n);
extern uint32_t
hton32(uint32_t h);
extern uint32_t
ntoh32(uint32_t n);
extern int
ioctl(int fd, int req, void *arg);

extern int
socket(int domain, int type, int protocol);
extern int
soclose(int soc);
#define close soclose
extern int
recvfrom(int soc, char *buf, size_t size, struct sockaddr *addr, int *salen);
extern int
sendto(int soc, char *buf, size_t size, struct sockaddr *addr, int salen);
extern int
bind(int soc, struct sockaddr *addr, int salen);
extern int
listen(int soc, int backlog);
extern int
accept(int soc, struct sockaddr *addr, int *salen);
extern int
connect(int soc, struct sockaddr *addr, int salen);
extern int
recv(int soc, char *buf, size_t size);
extern int
send(int soc, char *buf, size_t size);

#ifdef __cplusplus
}
#endif
