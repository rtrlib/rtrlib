/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef RTR_IP_PRIVATE_H
#define RTR_IP_PRIVATE_H

#include "ip.h"

#include "rtrlib/lib/ipv4_private.h"
#include "rtrlib/lib/ipv6_private.h"

/**
 * @brief Detects if the rtr_ip_addr only contains 0 bits.
 * @param[in] rtr_ip_addr
 * @returns true If the saved rtr_ip_addr is 0.
 * @returns false If the saved rtr_ip_addr isn't 0.
 */
bool rtr_ip_addr_is_zero(const struct rtr_ip_addr);

/**
 * @brief Extracts number bits from the passed rtr_ip_addr, starting at bit number from. The bit with the highest
 * significance is bit 0. All bits that aren't in the specified range will be 0.
 * @param[in] val rtr_ip_addr
 * @param[in] from Position of the first bit that is extracted.
 * @param[in] number How many bits will be extracted.
 * @returns An rtr_ipv_addr, where all bits that aren't in the specified range are set to 0.
 */
struct rtr_ip_addr rtr_ip_addr_get_bits(const struct rtr_ip_addr *val, const uint8_t from, const uint8_t number);

/**
 * @defgroup util_h Utility functions
 * @{
 *
 * @brief Checks if two rtr_ip_addr structs are equal.
 * @param[in] a rtr_ip_addr
 * @param[in] b rtr_ip_addr
 * @return true If a == b.
 * @return false If a != b.
 */
bool rtr_ip_addr_equal(const struct rtr_ip_addr a, const struct rtr_ip_addr b);

/**
 * Compares addr1 in the rtr_ip_addr struct with addr2 in string representation.
 * @param[in] addr1 rtr_ip_addr
 * @param[in] addr2 IP-address as string
 * @return true If a == b
 * @return false If a != b
 */
bool rtr_ip_str_cmp(const struct rtr_ip_addr *addr1, const char *addr2);

#endif
/** @} */
