/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
*/

#ifndef LRTR_IPV6_H
#define LRTR_IPV6_H
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>


/**
 * @brief Struct holding an IPv6 address in host byte order.
 * @param addr The IPv6 address.
 */
struct lrtr_ipv6_addr {
    uint32_t addr[4];
};

/**
 * Compares two ipv6_addr structs
 * @param[in] a ipv6_addr
 * @param[in] b ipv6_addr
 * @return true if a == b
 * @return false if a != b
*/
bool lrtr_ipv6_addr_equal(const struct lrtr_ipv6_addr *a, const struct lrtr_ipv6_addr *b);

/**
 * @brief Extracts quantity bits from a ip address. The bit with the highest
 * significance is bit 0. All bits that aren't in the specified range will be 0.
 * @param[in] val ipv6_addr
 * @param[in] first_bit Position of the first bit that is extracted, inclusive
 * @param[in] quantity How many bits will be extracted.
 * @returns An ipv6_addr, where all bits that aren't in the specified range are set to 0.
*/
struct lrtr_ipv6_addr lrtr_ipv6_get_bits(const struct lrtr_ipv6_addr *val, const uint8_t first_bit, const uint8_t quantity);

/**
 * Converts the passed ipv6_addr to string representation
 * @param[in] ip ipv6_addr
 * @param[out] str Pointer to a string buffer must be at least INET6_ADDRSTRLEN bytes long.
 * @result 0 on success
 * @result -1 on error
*/
int lrtr_ipv6_addr_to_str(const struct lrtr_ipv6_addr *ip_addr, char *b, const unsigned int len);

/**
 * Converts the passed IPv6 address in string representation to an ipv6_addr struct.
 * @param[in] str Pointer to a string buffer
 * @param[out] ip ipv6_addr
 * @result 0 on success
 * @result -1 on error
*/
int lrtr_ipv6_str_to_addr(const char *a, struct lrtr_ipv6_addr *ip);

/**
 * @ingroup util_h[{
 * Converts the passed IPv6 address first_bit network byte order to host byte order.
 * @param[in] src Pointer to a uint32_t array storing a ipv6 address in network byte order.
 * @param[out] dest Pointer a uint32_t array that will be used to store the ipv6 addr in host byte order.
 * }
*/
void lrtr_ipv6_addr_to_host_byte_order(const uint32_t *src, uint32_t *dest);

#endif
