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

#ifndef RTR_IPV4_H
#define RTR_IPV4_H
#include <inttypes.h>
#include <stdbool.h>

/**
 * @brief Struct holding an IPv4 address in host byte order.
 * @param addr The IPv4 address.
 */
typedef struct {
    uint32_t addr;
} ipv4_addr;

ipv4_addr ipv4_get_bits(const ipv4_addr* val, const uint8_t from, const uint8_t number);
int str_to_ipv4_addr(const char* str, ipv4_addr* ip);
int ipv4_addr_to_str(const ipv4_addr* ip, char* str, unsigned int len);
bool ipv4_addr_eq(const ipv4_addr* a, const ipv4_addr* b);
#endif
