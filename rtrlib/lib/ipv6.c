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

#include "rtrlib/lib/ipv6.h"
#include "rtrlib/lib/ipv4.h"
#include "rtrlib/lib/utils.h"
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

inline bool ipv6_addr_equal(const ipv6_addr* a, const ipv6_addr* b){
    if(a->addr[0] == b->addr[0] && a->addr[1] == b->addr[1] && a->addr[2] == b->addr[2] && a->addr[3] == b->addr[3])
        return true;
    return false;
}

ipv6_addr ipv6_get_bits(const ipv6_addr* val, const uint8_t from, const uint8_t number){
    ipv6_addr result;

    //0-31
    if(from < 32){
        const uint8_t end = (from + number) > 32 ? (32 - from) : number;
        result.addr[0] = rtr_get_bits(val->addr[0], from, end);
    }
    else
        result.addr[0] = 0;

    //32-63
    if(from < 64 && from + number > 33){
        const uint8_t start = from < 32 ? 0 : (from - 63);
        const uint8_t end = (from + number) > 63 ? (32 - start) : (64 - number);
        result.addr[1] = rtr_get_bits(val->addr[1], start, end);
    }
    else
        result.addr[1] = 0;

    //64-95
    if(from < 96 && from + number > 65){
        const uint8_t start = from < 65 ? 0 : (from - 95);
        const uint8_t end = (from + number) > 95 ? (32 - start) : number;
        result.addr[2] = rtr_get_bits(val->addr[2], start, end);
    }
    else
        result.addr[2] = 0;

    //96-127
    if(from + number > 97){
        const uint8_t start = from < 96 ? 0 : (from - 127);
        const uint8_t end = (from + number) > 126 ? (32 - start) : number; //from63 - 64 klappt nicht
        result.addr[3] = rtr_get_bits(val->addr[3], start, end);
    }
    else
        result.addr[3] = 0;
    return result;
}

/*
 * This function was copied from the bird routing daemon's ip_pton(..) function.
 */
int ipv6_str_to_addr(const char *a, ipv6_addr* ip){
    uint32_t* o = ip->addr;
    uint16_t words[8];
    int i, j, k, l, hfil;
    const char *start;

    if (a[0] == ':'){           /* Leading :: */
        if (a[1] != ':')
            return -1;
        a++;
    }
    hfil = -1;
    i = 0;
    while (*a){
        if (*a == ':'){         /* :: */
            if (hfil >= 0)
                return -1;
          hfil = i;
          a++;
          continue;
        }
        j = 0;
        l = 0;
        start = a;
        for(;;){
            if (*a >= '0' && *a <= '9')
                k = *a++ - '0';
            else if (*a >= 'A' && *a <= 'F')
                k = *a++ - 'A' + 10;
            else if (*a >= 'a' && *a <= 'f')
                k = *a++ - 'a' + 10;
            else
                break;
            j = (j << 4) + k;
            if (j >= 0x10000 || ++l > 4)
                return -1;
        }
        if (*a == ':' && a[1])
            a++;
        else if (*a == '.' && (i == 6 || (i < 6 && hfil >= 0))) {             /* Embedded IPv4 address */
            ipv4_addr addr4;
            if (ipv4_str_to_addr(start, &addr4) == -1)
                return -1;
            words[i++] = addr4.addr >> 16;
            words[i++] = addr4.addr;
            break;
        }
        else if (*a)
            return -1;
        if (i >= 8)
            return -1;
        words[i++] = j;
    }

  /* Replace :: with an appropriate number of zeros */
    if (hfil >= 0){
        j = 8 - i;
        for(i=7; i-j >= hfil; i--)
            words[i] = words[i-j];
        for(; i>=hfil; i--)
            words[i] = 0;
    }

  /* Convert the address to ip_addr format */
    for(i=0; i<4; i++)
        o[i] = (words[2*i] << 16) | words[2*i+1];
    return 0;
}

/*
 * This function was copied from the bird routing daemon's ip_ntop(..) function.
*/
int ipv6_addr_to_str(const ipv6_addr* ip_addr, char *b, const unsigned int len){
    if(len < INET6_ADDRSTRLEN)
        return -1;
    const uint32_t* a = ip_addr->addr;
    uint16_t words[8];
    int bestpos, bestlen, curpos, curlen, i;

    /* First of all, preprocess the address and find the longest run of zeros */
    bestlen = bestpos = curpos = curlen = 0;
    for(i = 0; i < 8; i++){
        uint32_t x = a[i/2];
        words[i] = ((i % 2) ? x : (x >> 16)) & 0xffff;
        if (words[i])
            curlen = 0;
        else{
            if (!curlen)
                curpos = i;
            curlen++;
            if (curlen > bestlen) {
                bestpos = curpos;
                bestlen = curlen;
            }
        }
    }
    if (bestlen < 2)
        bestpos = -1;

  /* Is it an encapsulated IPv4 address? */
    if (!bestpos && ((bestlen == 5 && a[2] == 0xffff) || bestlen == 6))
    //if (!bestpos && ((bestlen == 5 && (a[2] == 0xffff)) || bestlen == 6))
    {
        uint32_t x = a[3];
        b += sprintf(b, "::%s%d.%d.%d.%d", a[2] ? "ffff:" : "", ((x >> 24) & 0xff), ((x >> 16) & 0xff), ((x >> 8) & 0xff), (x & 0xff));
      return 0;
    }

  /* Normal IPv6 formatting, compress the largest sequence of zeros */
    for(i = 0; i < 8; i++) {
        if (i == bestpos) {
            i += bestlen - 1;
            *b++ = ':';
            if (i == 7)
            *b++ = ':';
        }
        else {
            if (i)
            *b++ = ':';
            b += sprintf(b, "%x", words[i]);
        }
    }
    *b = '\0';
    return 0;
}

void ipv6_addr_to_host_byte_order(const uint32_t* src, uint32_t* dest){
  for(int i = 0; i < 4; i++)
    dest[i] = ntohl(src[i]);
}
