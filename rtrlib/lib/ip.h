/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
*/

#ifndef LRTR_IP_H
#define LRTR_IP_H
#include "rtrlib/lib/ipv4.h"
#include "rtrlib/lib/ipv6.h"


/**
 * @brief Version of the IP protocol.
 */
enum lrtr_ip_version {
    /** IPV4 */
            RTRLIB_IPV4,

    /** IPV6 */
            RTRLIB_IPV6
};

/**
 * @brief The rtr_ip_addr struct stores a IPv4 or IPv6 address in host byte order.
 * @param ver Specifies the type of the stored address.
 * @param u Union holding a ipv4_addr or ipv6_addr.
 */
struct lrtr_ip_addr {
    enum lrtr_ip_version ver;
    union {
        struct lrtr_ipv4_addr addr4;
        struct lrtr_ipv6_addr addr6;
    } u;
};

/**
 * @brief Detects if the rtr_ip_addr only contains 0 bits.
 * @param[in] rtr_ip_addr
 * @returns true If the saved rtr_ip_addr is 0.
 * @returns false If the saved rtr_ip_addr isn't 0.
 */
bool lrtr_ip_addr_is_zero(const struct lrtr_ip_addr);

/**
 * @brief Extracts number bits from the passed rtr_ip_addr, starting at bit number from. The bit with the highest
 * significance is bit 0. All bits that aren't in the specified range will be 0.
 * @param[in] val rtr_ip_addr
 * @param[in] from Position of the first bit that is extracted.
 * @param[in] number How many bits will be extracted.
 * @returns An ipv4_addr, where all bits that aren't in the specified range are set to 0.
*/
struct lrtr_ip_addr lrtr_ip_addr_get_bits(const struct lrtr_ip_addr *val, const uint8_t from, const uint8_t number);

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
bool lrtr_ip_addr_equal(const struct lrtr_ip_addr a, const struct lrtr_ip_addr b);

/**
 * Converts the passed rtr_ip_addr struct to string representation.
 * @param[in] ip rtr_ip_addr
 * @param[out] str Pointer to a char array. The array must be at least INET_ADDRSTRLEN bytes long if the passed rtr_ip_addr stores
 * an IPv4 address. If rtr_ip_addr stores an IPv6 address, str must be at least INET6_ADDRSTRLEN bytes long.
 * @param[in] len Length of the str array.
 * @result 0 On success.
 * @result -1 On error.
*/
int lrtr_ip_addr_to_str(const struct lrtr_ip_addr *ip, char *str, const unsigned int len);

/**
 * Converts the passed IP address in string representation to an rtr_ip_addr.
 * @param[in] str Pointer to a Null terminated char array.
 * @param[out] ip Pointer to a rtr_ip_addr struct.
 * @result 0 On success.
 * @result -1 On error.
*/
int lrtr_ip_str_to_addr(const char *str, struct lrtr_ip_addr *ip);

/**
 * Compares addr1 in the rtr_ip_addr struct with addr2 in string representation.
 * @param[in] addr1 rtr_ip_addr
 * @param[in] addr2 IP-address as string
 * @return true If a == b
 * @return false If a != b
*/
bool lrtr_ip_str_cmp(const struct lrtr_ip_addr *addr1, const char *addr2);

#endif
/* @} */
