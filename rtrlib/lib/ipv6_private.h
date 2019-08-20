/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef LRTR_IPV6_PRIVATE_H
#define LRTR_IPV6_PRIVATE_H

#include "ipv6.h"

#include "rtrlib/lib/convert_byte_order_private.h"

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

/**
 * @brief Compares two lrtr_ipv6_addr structs
 *
 * @param[in] a		lrtr_ipv6_addr
 * @param[in] b		lrtr_ipv6_addr
 *
 * @return true if a == b
 * @return false if a != b
 */
bool lrtr_ipv6_addr_equal(const struct lrtr_ipv6_addr *a, const struct lrtr_ipv6_addr *b);

/**
 * @brief Extracts quantity bits from an IPv6 address.
 *
 * The bit with the highest significance is bit 0. All bits that aren't in the
 * specified range will be 0.
 *
 * @param[in] val	ipv6_addr
 * @param[in] first_bit Position of the first bit that is extracted, inclusive.
 * @param[in] quantity	How many bits will be extracted.
 *
 * @returns ipv6_addr, with all bits not in specified range set to 0.
 */
struct lrtr_ipv6_addr lrtr_ipv6_get_bits(const struct lrtr_ipv6_addr *val, const uint8_t first_bit,
					 const uint8_t quantity);

/**
 * @brief Converts the passed ipv6_addr to string representation
 *
 * @param[in] ip_addr	Pointer to an IPv6 address
 * @param[out] str	Pointer to string buf, at least INET6_ADDRSTRLEN bytes.
 * @param[in] len	Length of string buffer @p str
 *
 * @result 0 on success
 * @result -1 on error
 */
int lrtr_ipv6_addr_to_str(const struct lrtr_ipv6_addr *ip, char *str, const unsigned int len);

/**
 * @brief Converts the passed IPv6 address string in to lrtr_ipv6_addr struct.
 *
 * @param[in] str	Pointer to a string buffer
 * @param[out] ip	Pointer to lrtr_ipv6_addr
 *
 * @result 0 on success
 * @result -1 on error
 */
int lrtr_ipv6_str_to_addr(const char *str, struct lrtr_ipv6_addr *ip);

/**
 * @ingroup util_h
 * @{
 * @brief Converts the passed IPv6 address to given byte order.
 *
 * @param[in] src	IPv6 address (uint32_t array) in source byte order.
 * @param[out] dest	IPv6 address (uint32_t array) in target byte order.
 * @param[in] tbo	Target byte order for address conversion.
 */
void lrtr_ipv6_addr_convert_byte_order(const uint32_t *src, uint32_t *dest, const enum target_byte_order tbo);
/** @} */
#endif /* LRTR_IPV6_H */
