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

#include "rtrlib/lib/ipv4.h"
#include "rtrlib/lib/utils.h"
#include <assert.h>
#include <stdio.h>

ipv4_addr ipv4_get_bits(const ipv4_addr* val, const uint8_t from, const uint8_t number){
    ipv4_addr result;
    result.addr = rtr_get_bits(val->addr, from, number);
    return result;
}

int ipv4_addr_to_str(const ipv4_addr* ip, char* str, unsigned int len){
    uint8_t* t = (uint8_t*) &(ip->addr);

    if (snprintf(str, len, "%hhu.%hhu.%hhu.%hhu", t[3], t[2], t[1], t[0]) < 0){
        return -1;
    }
    return 0;
}

int str_to_ipv4_addr(const char* str, ipv4_addr* ip){
    unsigned char* t =  (unsigned char*) &(ip->addr);
    if(sscanf(str, "%hhu.%hhu.%hhu.%hhu", &(t[3]), &(t[2]), &(t[1]), &(t[0])) != 4)
        return -1;
    return 0;
}

bool ipv4_addr_eq(const ipv4_addr* a, const ipv4_addr* b){
    if(a->addr == b->addr)
        return true;
    return false;
}
