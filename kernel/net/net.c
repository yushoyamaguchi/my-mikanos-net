#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "util.h"
#include "net.h"

struct net_protocol {
    struct net_protocol *next;
    uint16_t type;
    mutex_t mutex; /* mutex for input queue */
    struct queue_head queue; /* input queue */
    void (*handler)(const uint8_t *data, size_t len, struct net_device *dev);
};

/* NOTE: the data follows immediately after the structure */
struct net_protocol_queue_entry {
    struct net_device *dev;
    size_t len;
};

struct net_timer {
    struct net_timer *next;
    struct timeval interval;
    struct timeval last;
    void (*handler)(void);
};

/* NOTE: if you want to add/delete the entries after net_run(), you need to protect these lists with a mutex. */
static struct net_device *devices;
static struct net_protocol *protocols;
static struct net_timer *timers;

struct net_device *
net_device_alloc(void)
{
    struct net_device *dev;

    dev = (struct net_device *)calloc(1, sizeof(*dev));
    if (!dev) {
        errorf("calloc() failure");
        return NULL;
    }
    return dev;
}

/* NOTE: must not be call after net_run() */
int
net_device_register(struct net_device *dev)
{
    static unsigned int index = 0;

    dev->index = index++;
    snprintf(dev->name, sizeof(dev->name), "net%d", dev->index);
    dev->next = devices;
    devices = dev;
    infof("registered, dev=%s, type=0x%04x", dev->name, dev->type);
    return 0;
}

int
net_device_open(struct net_device *dev)
{
    if (NET_DEVICE_IS_UP(dev)) {
        errorf("already opened, dev=%s", dev->name);
        return -1;
    }
    if (dev->ops->open) {
        if (dev->ops->open(dev) == -1) {
            errorf("failure, dev=%s", dev->name);
            return -1;
        }
    }
    dev->flags |= NET_DEVICE_FLAG_UP;
    infof("dev=%s, state=%s", dev->name, NET_DEVICE_STATE(dev));
    return 0;
}

int
net_device_close(struct net_device *dev)
{
    if (!NET_DEVICE_IS_UP(dev)) {
        errorf("not opened, dev=%s", dev->name);
        return -1;
    }
    if (dev->ops->close) {
        if (dev->ops->close(dev) == -1) {
            errorf("failure, dev=%s", dev->name);
            return -1;
        }
    }
    dev->flags &= ~NET_DEVICE_FLAG_UP;
    infof("dev=%s, state=%s", dev->name, NET_DEVICE_STATE(dev));
    return 0;
}

struct net_device *
net_device_by_index(unsigned int index)
{
    struct net_device *entry;

    for (entry = devices; entry; entry = entry->next) {
        if (entry->index == index) {
            break;
        }
    }
    return entry;
}
struct net_device *
net_device_by_name(const char *name)
{
    struct net_device *entry;

    for (entry = devices; entry; entry = entry->next) {
        if (strcmp(entry->name, name) == 0) {
            break;
        }
    }
    return entry;
}

/* NOTE: must not be call after net_run() */
int
net_device_add_iface(struct net_device *dev, struct net_iface *iface)
{
    struct net_iface *entry;

    for (entry = dev->ifaces; entry; entry = entry->next) {
        if (entry->family == iface->family) {
            errorf("already exists, dev=%s, family=%d", dev->name, entry->family);
            return -1;
        }
    }
    iface->next = dev->ifaces;
    iface->dev = dev;
    dev->ifaces = iface;
    return 0;
}

struct net_iface *
net_device_get_iface(struct net_device *dev, int family)
{
    struct net_iface *entry;

    for (entry = dev->ifaces; entry; entry = entry->next) {
        if (entry->family == family) {
            break;
        }
    }
    return entry;
}

int
net_device_output(struct net_device *dev, uint16_t type, const uint8_t *data, size_t len, const void *dst)
{
    if (!NET_DEVICE_IS_UP(dev)) {
        errorf("not opened, dev=%s", dev->name);
        return -1;
    }
    if (len > dev->mtu) {
        errorf("too long, dev=%s, mtu=%u, len=%lu", dev->name, dev->mtu, len);
        return -1;
    }
    debugf("dev=%s, type=0x%04x, len=%lu", dev->name, type, len);
    debugdump(data, len);
    if (dev->ops->transmit(dev, type, data, len, dst) == -1) {
        errorf("device transmit failure, dev=%s, len=%lu", dev->name, len);
        return -1;
    }
    return 0;
}

int
net_input_handler(uint16_t type, const uint8_t *data, size_t len, struct net_device *dev)
{
    struct net_protocol *proto;
    struct net_protocol_queue_entry *entry;

    for (proto = protocols; proto; proto = proto->next) {
        if (proto->type == type) {
            entry = (struct net_protocol_queue_entry *)calloc(1, sizeof(*entry) + len);
            if (!entry) {
                errorf("calloc() failure");
                return -1;
            }
            entry->dev = dev;
            entry->len = len;
            memcpy(entry+1, data, len);
            mutex_lock(&proto->mutex);
            if (!queue_push(&proto->queue, entry)) {
                mutex_unlock(&proto->mutex);
                errorf("queue_push() failure");
                free(entry);
                return -1;
            }
            debugf("queue pushed (num:%u), dev=%s, type=0x%04x, len=%ld", proto->queue.num, dev->name, type, len);
            debugdump(data, len);
            mutex_unlock(&proto->mutex);
            softirq();
            return 0;
        }
    }
    /* unsupported protocol */
    return 0;
}

/* NOTE: must not be call after net_run() */
int
net_protocol_register(uint16_t type, void (*handler)(const uint8_t *data, size_t len, struct net_device *dev))
{
    struct net_protocol *proto;

    for (proto = protocols; proto; proto = proto->next) {
        if (type == proto->type) {
            errorf("already registered, type=0x%04x", type);
            return -1;
        }
    }
    proto = (struct net_protocol *)calloc(1, sizeof(*proto));
    if (!proto) {
        errorf("calloc() failure");
        return -1;
    }
    proto->type = type;
    mutex_init(&proto->mutex, NULL);
    proto->handler = handler;
    proto->next = protocols;
    protocols = proto;
    infof("registered, type=0x%04x", type);
    return 0;
}

/* NOTE: must not be call after net_run() */
int
net_timer_register(struct timeval interval, void (*handler)(void))
{
    struct net_timer *timer;

    timer = (struct net_timer *)calloc(1, sizeof(*timer));
    if (!timer) {
        errorf("calloc() failure");
        return -1;
    }
    timer->interval = interval;
    gettimeofday(&timer->last, NULL);
    timer->handler = handler;
    timer->next = timers;
    timers = timer;
    infof("registered: interval={%d, %d}", interval.tv_sec, interval.tv_usec);
    return 0;
}

void
net_softirq_handler()
{
    struct net_protocol *proto;
    struct net_protocol_queue_entry *entry;

    for (proto = protocols; proto; proto = proto->next) {
        mutex_lock(&proto->mutex);
        entry = (struct net_protocol_queue_entry *)queue_pop(&proto->queue);
        if (!entry) {
            mutex_unlock(&proto->mutex);
            continue;
        }
        debugf("queue popped (num:%u), dev=%s, type=0x%04x, len=%ld", proto->queue.num, entry->dev->name, proto->type, entry->len);
        debugdump((uint8_t *)(entry+1), entry->len);
        mutex_unlock(&proto->mutex);
        proto->handler((uint8_t *)(entry+1), entry->len, entry->dev);
        free(entry);
    }
}

void
net_timer_handler()
{
    struct net_timer *timer;
    struct timeval now, diff;

    for (timer = timers; timer; timer = timer->next) {
        gettimeofday(&now, NULL);
        timersub(&now, &timer->last, &diff);
        if (timercmp(&timer->interval, &diff, <) != 0) { /* true (!0) or false (0) */
            timer->handler();
            timer->last = now;
        }
    }
}

int
net_run(void)
{
    struct net_device *dev;

    debugf("open all devices...");
    for (dev = devices; dev; dev = dev->next) {
        net_device_open(dev);
    }
    debugf("running...");
    return 0;
}

void
net_shutdown(void)
{
    struct net_device *dev;

    debugf("close all devices...");
    for (dev = devices; dev; dev = dev->next) {
        net_device_close(dev);
    }
    debugf("shutdown");
}

void
net_interrupt(void)
{
    // ignore
}

#include "arp.h"
#include "ip.h"
#include "icmp.h"
#include "udp.h"
#include "tcp.h"

int
net_init(void)
{
    if (arp_init() == -1) {
        errorf("arp_init() failure");
        return -1;
    }
    if (ip_init() == -1) {
        errorf("ip_init() failure");
        return -1;
    }
    if (icmp_init() == -1) {
        errorf("icmp_init() failure");
        return -1;
    }
    if (udp_init() == -1) {
        errorf("udp_init() failure");
        return -1;
    }
    if (tcp_init() == -1) {
        errorf("tcp_init() failure");
        return -1;
    }
    infof("initialized");
    return 0;
}
