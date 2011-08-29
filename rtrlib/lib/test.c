/*
 * This file is part of RTRlib.
 *
 * RTRlib is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * RTRlib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RTRlib; see the file COPYING.LESSER.
 *
 * written by Fabian Holler, in cooperation with:
 * INET group, Hamburg University of Applied Sciences,
 * CST group, Freie Universitaet Berlin
 * Webpage: http://rpki.realmv6.org/
 */

#include "rtrlib/lib/test.h"
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void put_ipv4(ip_addr* res, char* addr){
    res->ver = IPV4;
    struct in_addr iaddr; 
    assert(inet_pton(AF_INET, addr, &iaddr) != -1);
    res->u.addr4.addr = iaddr.s_addr;
}

void put_ipv6(ip_addr* res, char* addr){
    res->ver = IPV6;

    struct in6_addr iaddr; 
    assert(inet_pton(AF_INET6, addr, &iaddr) != -1);

    memcpy(res->u.addr6.addr, iaddr.s6_addr, sizeof(iaddr.s6_addr));
}

void get_ipv4(ip_addr* res, char* addr){
    struct in_addr iaddr; 
    iaddr.s_addr = res->u.addr4.addr;
    assert(inet_ntop(AF_INET, &iaddr, addr, INET_ADDRSTRLEN) != NULL);
}

void get_ipv6(ip_addr* res, char* addr){
    struct in6_addr iaddr; 
    memcpy(iaddr.s6_addr, res->u.addr6.addr, sizeof(res->u.addr6.addr));
    assert(inet_ntop(AF_INET6, &iaddr, addr, INET6_ADDRSTRLEN) != NULL);
}

bool cmp_ipv4(ip_addr* res, char* cmp){
    char buf[INET_ADDRSTRLEN];
    get_ipv4(res, buf);
    if(strncmp(buf, cmp, INET_ADDRSTRLEN) == 0)
        return true;
    return false;
}

bool cmp_ipv6(ip_addr* res, char* cmp){
    char buf[INET6_ADDRSTRLEN];
    get_ipv6(res, buf);
    if(strncmp(buf, cmp, INET6_ADDRSTRLEN) == 0)
        return true;
    return false;
}
void print_bytes(void* buf, size_t len){
    for(int i = 0;i < len; i++){
        if(len != 0)
            printf(":");
        printf("%02x", *((uint8_t*) ((char*) buf + i)));
    }
    printf("\n");
}
