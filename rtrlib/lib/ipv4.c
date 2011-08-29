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

#include "rtrlib/lib/ipv4.h"
#include <assert.h>

ipv4_addr ipv4_get_bits(const ipv4_addr* val, const u_int8_t from, const u_int8_t to){
    assert(to <= 31);

    u_int8_t n = to - from + 1;
    ipv4_addr result;
    u_int32_t mask = ~0;
    if(n != 32)
        mask = ~(mask << n);

    if(from != 0)
        mask <<= from;
    result.addr = mask & val->addr;

    return result;
}
