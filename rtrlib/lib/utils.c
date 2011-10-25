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
 * Website: http://rpki.realmv6.org/
 */

#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "rtrlib/lib/utils.h"


int rtr_get_monotonic_time(time_t* seconds){
    struct timespec time;
    if(clock_gettime(CLOCK_MONOTONIC, &time) == -1)
        return -1;
    *seconds = time.tv_sec;
    if((time.tv_nsec *  1000000000) >=5)
        *seconds +=1;
    return 0;
}




int rtr_str_to_ipaddr(const char* str, ip_addr* ip_addr)
{
    if(strchr(str, ':') == NULL){ //IPv4 Addr
        ip_addr->ver = IPV4;
        return str_to_ipv4_addr(str, &(ip_addr->u.addr4));
    }
    //IPv6 Addr
    ip_addr->ver = IPV6;
    return str_to_ipv6_addr(str, &(ip_addr->u.addr6));
}


int rtr_ipaddr_to_str(const ip_addr* addr, char* result, const size_t len){
    if(addr->ver == IPV4){
        return ipv4_addr_to_str(&(addr->u.addr4), result, len);
    }
    else if(addr->ver == IPV6){
        return ipv6_addr_to_str(&(addr->u.addr6), result, len);
    }
    return 0;
}

bool rtr_cmp_ipv4(const ip_addr* addr1, const char* addr2){
    ip_addr tmp;
    if(rtr_str_to_ipaddr(addr2, &tmp) == -1)
        return false;
    return(ipv4_addr_eq(&(addr1->u.addr4), &(tmp.u.addr4)));
}

bool rtr_cmp_ipv6(const ip_addr* addr1, const char* addr2){
    ip_addr tmp;
    if(rtr_str_to_ipaddr(addr2, &tmp) == -1)
        return false;
    return(ipv6_addr_eq(&(addr1->u.addr6), &(tmp.u.addr6)));
}

uint32_t rtr_get_bits(const uint32_t val, const uint8_t from, const uint8_t number){
    assert(number < 33);
    assert(number > 0);

    uint32_t mask = ~0;
    if(number != 32)
        mask = ~(mask >> number);

    mask >>= from;
    return (mask & val);
}
