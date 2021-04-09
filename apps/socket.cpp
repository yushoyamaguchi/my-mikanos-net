#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "syscall.h"
#include "socket.hpp"

int
ip_addr_pton (const char *p, ip_addr_t *n) {
    char *sp, *ep;
    int idx;
    long ret;

    sp = (char *)p;
    for (idx = 0; idx < 4; idx++) {
        ret = strtol(sp, &ep, 10);
        if (ret < 0 || ret > 255) {
            return -1;
        }
        if (ep == sp) {
            return -1;
        }
        if ((idx == 3 && *ep != '\0') || (idx != 3 && *ep != '.')) {
            return -1;
        }
        ((uint8_t *)n)[idx] = ret;
        sp = ep + 1;
    }
    return 0;
}

char *
ip_addr_ntop (const ip_addr_t *n, char *p, size_t size) {
    uint8_t *ptr;

    ptr = (uint8_t *)n;
    snprintf(p, size, "%d.%d.%d.%d",
        ptr[0], ptr[1], ptr[2], ptr[3]);
    return p;
}

#ifndef __BIG_ENDIAN
#define __BIG_ENDIAN 4321
#endif
#ifndef __LITTLE_ENDIAN
#define __LITTLE_ENDIAN 1234
#endif

static int endian;

static int
byteorder(void) {
    uint32_t x = 0x00000001;

    return *(uint8_t *)&x ? __LITTLE_ENDIAN : __BIG_ENDIAN;
}

static uint16_t
byteswap16(uint16_t v)
{
    return (v & 0x00ff) << 8 | (v & 0xff00 ) >> 8;
}

static uint32_t
byteswap32(uint32_t v)
{
    return (v & 0x000000ff) << 24 | (v & 0x0000ff00) << 8 | (v & 0x00ff0000) >> 8 | (v & 0xff000000) >> 24;
}

uint16_t
hton16(uint16_t h)
{
    if (!endian) {
        endian = byteorder();
    }
    return endian == __LITTLE_ENDIAN ? byteswap16(h) : h;
}

uint16_t
ntoh16(uint16_t n)
{
    if (!endian) {
        endian = byteorder();
    }
    return endian == __LITTLE_ENDIAN ? byteswap16(n) : n;
}

uint32_t
hton32(uint32_t h)
{
    if (!endian) {
        endian = byteorder();
    }
    return endian == __LITTLE_ENDIAN ? byteswap32(h) : h;
}

uint32_t
ntoh32(uint32_t n)
{
    if (!endian) {
        endian = byteorder();
    }
    return endian == __LITTLE_ENDIAN ? byteswap32(n) : n;
}

int
ioctl(int fd, int req, void *arg)
{
    auto [ret, err] = SyscallSocketIOCTL(fd, req, arg);

    return ret;
}

int
socket(int domain, int type, int protocol)
{
    auto [ret, err] = SyscallSocketOpen(domain, type, protocol);

    return ret;
}

int
soclose(int soc)
{
    SyscallSocketClose(soc);
    return 0;
}

int
recvfrom(int soc, char *buf, size_t size, struct sockaddr *addr, int *salen)
{
    auto [ret, err] = SyscallSocketRecvFrom(soc, buf, size, addr, salen);

    return ret;
}

int
sendto(int soc, char *buf, size_t size, struct sockaddr *addr, int salen)
{
    auto [ret, err] = SyscallSocketSendTo(soc, buf, size, addr, salen);

    return ret;
}

int
bind(int soc, struct sockaddr *addr, int salen)
{
    auto [ret, err] = SyscallSocketBind(soc, addr, salen);

    return ret;
}

int
listen(int soc, int backlog)
{
    auto [ret, err] = SyscallSocketListen(soc, backlog);
    return ret;
}

int
accept(int soc, struct sockaddr *addr, int *salen)
{
    auto [ret, err] = SyscallSocketAccept(soc, addr, salen);

    return ret;
}

int
connect(int soc, struct sockaddr *addr, int salen)
{
    auto [ret, err] = SyscallSocketConnect(soc, addr, salen);

    return ret;
}

int
recv(int soc, char *buf, size_t size)
{
    auto [ret, err] = SyscallSocketRecv(soc, buf, size);

    return ret;
}

int
send(int soc, char *buf, size_t size)
{
    auto [ret, err] = SyscallSocketSend(soc, buf, size);

    return ret;
}
