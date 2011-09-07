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

#include "rtrlib/lib/ip.h"
#include <stdbool.h>

bool ip_addr_is_zero(const ip_addr prefix){
    if(prefix.ver ==  IPV6){
        if(prefix.u.addr6.addr[0] == 0 && prefix.u.addr6.addr[1] == 0){
            return true;
        }
    }
    else if(prefix.u.addr4.addr == 0)
        return true;

    return false;
}

ip_addr ip_addr_get_bits(const ip_addr* val, const uint8_t from, const uint8_t to){
    ip_addr result;
    if(val->ver == IPV6){
        result.ver = IPV6;
        result.u.addr6 = ipv6_get_bits(&(val->u.addr6), from, to);
    }
    else{
        result.ver = IPV4;
        result.u.addr4 = ipv4_get_bits(&(val->u.addr4), from, to);
    }
    return result;
}

bool ip_addr_equal(const ip_addr a, const ip_addr b ){
    if(a.ver != b.ver)
        return false;
    if(a.ver == IPV6){
        if(a.u.addr6.addr[0] == b.u.addr6.addr[0] && a.u.addr6.addr[1] == b.u.addr6.addr[1])
            return true;
    }
    else{
        if(a.u.addr4.addr == b.u.addr4.addr)
            return true;
    }
    return false;
}
