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
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <time.h>
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
    struct addrinfo hints;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_NUMERICHOST;
    struct addrinfo* result;
    if(getaddrinfo(str, NULL, &hints, &result) != 0)
        return -1;
    if(result->ai_family == AF_INET){
        ip_addr->ver = IPV4;
        ip_addr->u.addr4.addr = ((struct sockaddr_in*) (result->ai_addr))->sin_addr.s_addr;
    }
    else{
        ip_addr->ver = IPV6;
        const struct sockaddr_in6* sockaddr = ((struct sockaddr_in6*) (result->ai_addr));
        memcpy(ip_addr->u.addr6.addr, sockaddr->sin6_addr.s6_addr, sizeof(sockaddr->sin6_addr.s6_addr));
    }
    freeaddrinfo(result);
    return 0;
}


int rtr_ipaddr_to_str(const ip_addr* addr, char* result, const size_t len){
    if(addr->ver == IPV4){
        struct in_addr ina;
        ina.s_addr = addr->u.addr4.addr;
        if(inet_ntop(AF_INET, &ina, result, len) == NULL)
            return -1;
    }
    else if(addr->ver == IPV6){
        if(inet_ntop(AF_INET6, (struct in6_addr*) &(addr->u.addr6), result, len) == NULL)
            return -1;
    }
    return 0;
}

bool rtr_cmp_ipv4(const ip_addr* addr1, const char* addr2){
    char buf[INET_ADDRSTRLEN];
    rtr_ipaddr_to_str(addr1, buf, sizeof(buf));
    if(strncmp(buf, addr2, INET_ADDRSTRLEN) == 0)
        return true;
    return false;
}

bool rtr_cmp_ipv6(const ip_addr* addr1, const char* addr2){
    char buf[INET6_ADDRSTRLEN];
    rtr_ipaddr_to_str(addr1, buf, sizeof(buf));
    if(strncmp(buf, addr2, INET6_ADDRSTRLEN) == 0)
        return true;
    return false;
}
