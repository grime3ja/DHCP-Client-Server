#ifndef __server_h__
#define __server_h__

#include <stdbool.h>
#include <stdint.h>
#include <netdb.h>
#include <stdbool.h>

#include "dhcp.h"


extern bool debug;
extern struct in_addr THIS_SERVER;


ssize_t serve_web (char *);

typedef struct {
    uint32_t xid;
    uint8_t chaddr[16];
    struct in_addr ciaddr;
    bool tombstone;
} client_t;

typedef struct {
    int ciaddr_digit;
    bool active;
} ip_t;

#endif
