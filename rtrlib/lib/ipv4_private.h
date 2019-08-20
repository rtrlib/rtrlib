/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef LRTR_IPV4_PRIVATE_H
#define LRTR_IPV4_PRIVATE_H

#include "ipv4.h"

#include "rtrlib/lib/convert_byte_order_private.h"

#include <inttypes.h>
#include <stdbool.h>

/**
 * @brief Extracts number bits from the passed ipv4_addr,
 *
 * Starting at bit number from. The bit with the highest significance is bit 0.
 * All bits that aren't in the specified range will be 0.
 *
 * @param[in] val ipv4_addr
 * @param[in] from Position of the first bit that is extracted.
 * @param[in] number How many bits will be extracted.
 *
 * @returns An ipv4_addr, with all bits not in the specified range set to 0.
 */
struct lrtr_ipv4_addr lrtr_ipv4_get_bits(const struct lrtr_ipv4_addr *val, const uint8_t from, const uint8_t number);

/**
 * @brief Converts ab IPv4 address from string to ipv4_addr struct.
 *
 * @param[in] str Pointer to a string buffer.
 * @param[out] ip ipv4_addr
 *
 * @result 0 on success
 * @result -1 on error
 */
int lrtr_ipv4_str_to_addr(const char *str, struct lrtr_ipv4_addr *ip);

/**
 * @brief Converts an ipv4_addr struct to its string representation.
 *
 * @param[in] ip ipv4_addr
 * @param[out] str Pointer to a string buffer, of at least INET_ADDRSTRLEN.
 * @param[in] length of *str
 *
 * @result 0 on success
 * @result -1 on error
 */
int lrtr_ipv4_addr_to_str(const struct lrtr_ipv4_addr *ip, char *str, const unsigned int len);

/**
 * @brief Compares two ipv4_addr structs.
 *
 * @param[in] a ipv4_addr
 * @param[in] b ipv4_addr
 *
 * @return true if a == b
 * @return false if a != b
 */
bool lrtr_ipv4_addr_equal(const struct lrtr_ipv4_addr *a, const struct lrtr_ipv4_addr *b);

/**
 * @ingroup util_h[{
 * @brief Converts the passed IPv4 address to given byte order.
 *
 * @param[in] src       IPv4 address in source byte order.
 * @param[out] dest     IPv4 address in target byte order.
 * @param[in] tbo       Target byte order for address conversion.
 * }
 */
void lrtr_ipv4_addr_convert_byte_order(const uint32_t src, uint32_t *dest, const enum target_byte_order tbo);

#endif
