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

#include "rtrlib/lib/ipv6.h"
#include <assert.h>
static inline int64_t get_bits64(const u_int64_t val, const u_int8_t from, const u_int8_t to);

inline bool ipv6_addr_equal(ipv6_addr a, ipv6_addr b){
    if(a.addr[0] == b.addr[0] && a.addr[1] == b.addr[1])
        return true;
    return false;
}

ipv6_addr ipv6_get_bits(const ipv6_addr* val, const u_int8_t from, const u_int8_t to){
    ipv6_addr result;

    if(from < 64){
        const u_int8_t end = to > 63 ? 63 : to;
        result.addr[0] = get_bits64(val->addr[0], from, end);
    }
    else
        result.addr[0] = 0;

    if(to > 63){
        const u_int8_t start = from < 64 ? 0 : (from - 64);
        result.addr[1] = get_bits64(val->addr[1], start, (to - 64));
    }
    else
        result.addr[1] = 0;
    return result;
}

static inline int64_t get_bits64(const u_int64_t val, const u_int8_t from, const u_int8_t to){
    assert(to <= 63);

    /*
    u_int64_t mask = ~0;
    mask = (mask << (63 - (to - from))) >> from;
    return (mask & val);
    */

    u_int8_t n = to - from + 1;
    u_int64_t mask = ~0;
    if(n != 64)
        mask = ~(mask << n);
    if(from != 0)
        mask <<= from;
    return mask & val;
}
