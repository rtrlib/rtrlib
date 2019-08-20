/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef LRTR_IP_PRIVATE_H
#define LRTR_IP_PRIVATE_H

#include "ip.h"

#include "rtrlib/lib/ipv4_private.h"
#include "rtrlib/lib/ipv6_private.h"

/**
 * @brief Detects if the lrtr_ip_addr only contains 0 bits.
 * @param[in] lrtr_ip_addr
 * @returns true If the saved lrtr_ip_addr is 0.
 * @returns false If the saved lrtr_ip_addr isn't 0.
 */
bool lrtr_ip_addr_is_zero(const struct lrtr_ip_addr);

/**
 * @brief Extracts number bits from the passed lrtr_ip_addr, starting at bit number from. The bit with the highest
 * significance is bit 0. All bits that aren't in the specified range will be 0.
 * @param[in] val lrtr_ip_addr
 * @param[in] from Position of the first bit that is extracted.
 * @param[in] number How many bits will be extracted.
 * @returns An lrtr_ipv_addr, where all bits that aren't in the specified range are set to 0.
 */
struct lrtr_ip_addr lrtr_ip_addr_get_bits(const struct lrtr_ip_addr *val, const uint8_t from, const uint8_t number);

/**
 * @defgroup util_h Utility functions
 * @{
 *
 * @brief Checks if two lrtr_ip_addr structs are equal.
 * @param[in] a lrtr_ip_addr
 * @param[in] b lrtr_ip_addr
 * @return true If a == b.
 * @return false If a != b.
 */
bool lrtr_ip_addr_equal(const struct lrtr_ip_addr a, const struct lrtr_ip_addr b);

/**
 * Compares addr1 in the lrtr_ip_addr struct with addr2 in string representation.
 * @param[in] addr1 lrtr_ip_addr
 * @param[in] addr2 IP-address as string
 * @return true If a == b
 * @return false If a != b
 */
bool lrtr_ip_str_cmp(const struct lrtr_ip_addr *addr1, const char *addr2);

#endif
/** @} */
