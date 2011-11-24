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

#ifndef RTR_IP_H
#define RTR_IP_H
#include "rtrlib/lib/ipv4.h"
#include "rtrlib/lib/ipv6.h"

/**
 * @brief Version of the IP protocol.
 */
typedef enum {
    /** IPV4 */
    IPV4,

    /** IPV6 */
    IPV6
} ip_version;

/**
 * @brief The ip_addr struct stores a IPv4 or IPv6 address in host byte order.
 * @param ver Specifies the address type that is stored.
 * @param u Union holding a ipv4_addr or ipv6_addr.
 */
typedef struct {
    ip_version ver;
    union{
        ipv4_addr addr4;
        ipv6_addr addr6;
    } u;
} ip_addr;

/**
 * @brief Detects if the ip_addr only contains 0 bits.
 * @param[in] ip_addr
 * @returns true if the saved ip_addr is 0.
 * @returns false if the saved ip_addr isn't 0.
 */
bool ip_addr_is_zero(const ip_addr);

/**
 * @brief Extracts number bits from the passed ip_addr, starting at bit number from. The bit with the highest
 * significance is bit 0. All bits that aren't in the specified range will be 0.
 * @param[in] val ip_addr
 * @param[in] from Position of the first bit that is extracted.
 * @param[in] number How many bits will be extracted.
 * @returns An ipv4_addr, where all bits that aren't in the specified range are set to 0.
*/
ip_addr ip_addr_get_bits(const ip_addr* val, const uint8_t from, const uint8_t number);

/**
 * @brief Checks if two ip_addr structs are equal.
 * @param[in] a ip_addr
 * @param[in] b ip_addr
 * @return true if a == b
 * @return false if a != b
 */
bool ip_addr_equal(const ip_addr a, const ip_addr b);

/**
 * Converts the passed ip_addr struct to string representation.
 * @param[in] ip ip_addr
 * @param[out] str Pointer to a char array. The array must be at least INET_ADDRSTRLEN bytes long if the passed ip_addr stores
 * an IPv4 address. If ip_addr stores an IPv6 address, str must be at least INET6_ADDRSTRLEN bytes long.
 * @result 0 on success
 * @result -1 on error
*/
int ip_addr_to_str(const ip_addr* ip, char* str, const unsigned int len);

/**
 * Converts the passed IP address in string representation to an ip_addr.
 * @param[in] str Pointer to a char array
 * @param[out] ip Pointer to a ip_addr struct
 * @result 0 on success
 * @result -1 on error
*/
int ip_str_to_addr(const char* str, ip_addr* ip);

/**
 * Compares addr1 in the ip_addr struct with addr2 in string representation.
 * @param[in] addr1 ip_addr
 * @param[in] addr2 IP-address as string
 * @return true if a == b
 * @return false if a != b
*/
bool ip_str_cmp(const ip_addr* addr1, const char* addr2);

#endif
/* @} */
