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
 * INET group, Hamburg University of Applied Sciences,
 * CST group, Freie Universitaet Berlin
 * Website: http://rpki.realmv6.org/
 */

#ifndef RTR_IPV4_H
#define RTR_IPV4_H
#include <inttypes.h>
#include <stdbool.h>

/**
 * @brief Struct storing an IPv4 address in host byte order.
 * @param addr The IPv4 address.
 */
struct ipv4_addr {
    uint32_t addr;
};

/**
 * @brief Extracts number bits from the passed ipv4_addr, starting at bit number from. The bit with the highest
 * significance is bit 0. All bits that aren't in the specified range will be 0.
 * @param[in] val ipv4_addr
 * @param[in] from Position of the first bit that is extracted.
 * @param[in] number How many bits will be extracted.
 * @returns An ipv4_addr, where all bits that aren't in the specified range are set to 0.
*/
struct ipv4_addr ipv4_get_bits(const struct ipv4_addr *val, const uint8_t from, const uint8_t number);

/**
 * Converts the passed IPv4 address in string representation to an ipv4_addr struct.
 * @param[in] str Pointer to a string buffer.
 * @param[out] ip ipv4_addr
 * @result 0 on success
 * @result -1 on error
*/
int ipv4_str_to_addr(const char *str, struct ipv4_addr *ip);

/**
 * Converts the passed ipv4_addr to string representation.
 * @param[in] ip ipv4_addr
 * @param[out] str Pointer to a string buffer must be at least INET_ADDRSTRLEN bytes long.
 * @result 0 on success
 * @result -1 on error
*/
int ipv4_addr_to_str(const struct ipv4_addr *ip, char *str, const unsigned int len);

/**
 * Compares two ipv4_addr structs.
 * @param[in] a ipv4_addr
 * @param[in] b ipv4_addr
 * @return true if a == b
 * @return false if a != b
*/
bool ipv4_addr_equal(const struct ipv4_addr *a, const struct ipv4_addr *b);

#endif
