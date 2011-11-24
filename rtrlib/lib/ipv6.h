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

#ifndef RTR_IPV6_H
#define RTR_IPV6_H
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>


/**
 * @brief Struct holding an IPv6 address in host byte order.
 * @param addr The IPv6 address.
 */
typedef struct {
    uint32_t addr[4];
} ipv6_addr;

/**
 * Compares two ipv6_addr structs
 * @param[in] a ipv6_addr
 * @param[in] b ipv6_addr
 * @return true if a == b
 * @return false if a != b
*/
bool ipv6_addr_equal(const ipv6_addr* a, const ipv6_addr* b);

/**
 * @brief Extracts number bits from the passed ip_addr, starting at bit number from. The bit with the highest
 * significance is bit 0. All bits that aren't in the specified range will be 0.
 * @param[in] val ipv6_addr
 * @param[in] from Position of the first bit that is extracted.
 * @param[in] number How many bits will be extracted.
 * @returns An ipv6_addr, where all bits that aren't in the specified range are set to 0.
*/
ipv6_addr ipv6_get_bits(const ipv6_addr* val, const uint8_t from, const uint8_t to);

/**
 * Converts the passed ipv6_addr to string representation
 * @param[in] ip ipv6_addr
 * @param[out] str Pointer to a string buffer must be at least INET6_ADDRSTRLEN bytes long.
 * @result 0 on success
 * @result -1 on error
*/
int ipv6_addr_to_str(const ipv6_addr* ip_addr, char *b, const unsigned int len);

/**
 * Converts the passed IPv6 address in string representation to an ipv6_addr struct.
 * @param[in] str Pointer to a string buffer
 * @param[out] ip ipv6_addr
 * @result 0 on success
 * @result -1 on error
*/
int ipv6_str_to_addr(const char *a, ipv6_addr* ip);

/**
 * Converts the passed IPv6 address from network byte order to host byte order.
 * @param[in] src Pointer to a uint32_t array storing a ipv6 address in network byte order.
 * @param[out] dest Pointer a uint32_t array that will be used to store the ipv6 addr in host byte order.
*/
void ipv6_addr_to_host_byte_order(const uint32_t* src, uint32_t* dest);

#endif
