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
#include <time.h>
#include "rtrlib/lib/utils.h"


int get_monotonic_time(time_t* seconds){
    struct timespec time;
    if(clock_gettime(CLOCK_MONOTONIC, &time) == -1)
        return -1;
    *seconds = time.tv_sec;
    if((time.tv_nsec *  1000000000) >=5)
        *seconds +=1;
    return 0;
}

int ipaddr_to_string(const ip_addr* addr, char* result, const size_t len){
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
