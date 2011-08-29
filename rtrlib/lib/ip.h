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

#ifndef RTR_IP_H
#define RTR_IP_H
#include "rtrlib/lib/ipv4.h"
#include "rtrlib/lib/ipv6.h"

/**
 * @brief Version of the IP protocol.
 */
typedef enum ip_version {
    /** IPV4 */
    IPV4,
    /** IPV6 */
    IPV6
} ip_version;

/**
 * @brief The ip_addr struct, can hold a IPv4 or IPv6 address.
 * @param ver Specifies the type of address that is stored in this struct.
 * @param u Union holding a ipv4_addr or ipv6_addr.
 */
typedef struct {
    ip_version ver;
    union{
        ipv4_addr addr4;
        ipv6_addr addr6;
    } u;
} ip_addr;


bool ip_addr_is_zero(const ip_addr);
ip_addr ip_addr_get_bits(const ip_addr* val, const u_int8_t from, const u_int8_t to);

/**
 * @brief Check if two ip_addr structs are equal
 * @param a
 * @param b
 * @return true if a == b
 * @return false if a != b
 */
bool ip_addr_equal(const ip_addr a, const ip_addr b);

#endif
/* @} */
