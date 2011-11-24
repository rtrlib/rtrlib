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

#include <stdbool.h>
#include <string.h>
#include "rtrlib/lib/ip.h"

bool ip_addr_is_zero(const ip_addr prefix){
    if(prefix.ver == IPV6){
        if(prefix.u.addr6.addr[0] == 0 && prefix.u.addr6.addr[1] == 0 && prefix.u.addr6.addr[2] == 0 && prefix.u.addr6.addr[3] == 0 ){
            return true;
        }
    }
    else if(prefix.u.addr4.addr == 0)
        return true;

    return false;
}

ip_addr ip_addr_get_bits(const ip_addr* val, const uint8_t from, const uint8_t number){
    ip_addr result;
    if(val->ver == IPV6){
        result.ver = IPV6;
        result.u.addr6 = ipv6_get_bits(&(val->u.addr6), from, number);
    }
    else{
        result.ver = IPV4;
        result.u.addr4 = ipv4_get_bits(&(val->u.addr4), from, number);
    }
    return result;
}

bool ip_addr_equal(const ip_addr a, const ip_addr b ){
    if(a.ver != b.ver)
        return false;
    if(a.ver == IPV6){
        return ipv6_addr_equal(&(a.u.addr6), &(b.u.addr6));
    }
    return ipv4_addr_equal(&(a.u.addr4), &(b.u.addr4));
}

int ip_addr_to_str(const ip_addr* ip, char* str, const unsigned int len){
    if(ip->ver == IPV6)
        return ipv6_addr_to_str(&(ip->u.addr6), str, len);
    return ipv4_addr_to_str(&(ip->u.addr4), str, len);
}

int ip_str_to_addr(const char* str, ip_addr* ip){
    if(strchr(str, ':') == NULL){
        ip->ver = IPV4;;
        return ipv4_str_to_addr(str, &(ip->u.addr4));
    }
    ip->ver = IPV6;
    return ipv6_str_to_addr(str, &(ip->u.addr6));
}

bool ip_str_cmp(const ip_addr* addr1, const char* addr2){
    ip_addr tmp;
    if(ip_str_to_addr(addr2, &tmp) == -1)
        return false;
    return(ip_addr_equal(*addr1, tmp));
}
