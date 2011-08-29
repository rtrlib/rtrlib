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
 * Webpage: https://rpki.realmv6.org/
 */

#ifndef RTR_IPV4_H
#define RTR_IPV4_H
#include <sys/types.h>

/**
 * @brief Struct holding an IPv4 address in network byte order.
 * @param addr The IPv4 address.
 */
typedef struct {
    u_int32_t addr;
} ipv4_addr;

ipv4_addr ipv4_get_bits(const ipv4_addr* val, const u_int8_t from, const u_int8_t to);
#endif
