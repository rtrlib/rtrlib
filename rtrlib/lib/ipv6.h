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

#ifndef RTR_IPV6_H
#define RTR_IPV6_H
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>


/**
 * @brief Struct holding an IPv6 address in network byte order.
 * @param addr The IPv6 address.
 */
typedef struct {
    uint64_t addr[2];
} ipv6_addr;

extern bool ipv6_addr_equal(ipv6_addr a, ipv6_addr b);
extern ipv6_addr ipv6_get_bits(const ipv6_addr* val, const u_int8_t from, const u_int8_t to);
#endif 
