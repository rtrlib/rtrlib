/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
*/

#ifndef RTR_IP_H
#define RTR_IP_H
#include "rtrlib/lib/ipv4.h"
#include "rtrlib/lib/ipv6.h"


/**
 * @brief Version of the IP protocol.
 */
enum ip_version {
    /** IPV4 */
    IPV4,

    /** IPV6 */
    IPV6
};

/**
 * @brief The ip_addr struct stores a IPv4 or IPv6 address in host byte order.
 * @param ver Specifies the type of the stored address.
 * @param u Union holding a ipv4_addr or ipv6_addr.
 */
struct ip_addr {
    enum ip_version ver;
    union {
        struct ipv4_addr addr4;
        struct ipv6_addr addr6;
    } u;
};

/**
 * @brief Detects if the ip_addr only contains 0 bits.
 * @param[in] ip_addr
 * @returns true If the saved ip_addr is 0.
 * @returns false If the saved ip_addr isn't 0.
 */
bool ip_addr_is_zero(const struct ip_addr);

/**
 * @brief Extracts number bits from the passed ip_addr, starting at bit number from. The bit with the highest
 * significance is bit 0. All bits that aren't in the specified range will be 0.
 * @param[in] val ip_addr
 * @param[in] from Position of the first bit that is extracted.
 * @param[in] number How many bits will be extracted.
 * @returns An ipv4_addr, where all bits that aren't in the specified range are set to 0.
*/
struct ip_addr ip_addr_get_bits(const struct ip_addr *val, const uint8_t from, const uint8_t number);

/**
 * @defgroup util_h Utility functions
 * @{
 *
 * @brief Checks if two ip_addr structs are equal.
 * @param[in] a ip_addr
 * @param[in] b ip_addr
 * @return true If a == b.
 * @return false If a != b.
 */
bool ip_addr_equal(const struct ip_addr a, const struct ip_addr b);

/**
 * Converts the passed ip_addr struct to string representation.
 * @param[in] ip ip_addr
 * @param[out] str Pointer to a char array. The array must be at least INET_ADDRSTRLEN bytes long if the passed ip_addr stores
 * an IPv4 address. If ip_addr stores an IPv6 address, str must be at least INET6_ADDRSTRLEN bytes long.
 * @param[in] len Length of the str array.
 * @result 0 On success.
 * @result -1 On error.
*/
int ip_addr_to_str(const struct ip_addr *ip, char *str, const unsigned int len);

/**
 * Converts the passed IP address in string representation to an ip_addr.
 * @param[in] str Pointer to a Null terminated char array.
 * @param[out] ip Pointer to a ip_addr struct.
 * @result 0 On success.
 * @result -1 On error.
*/
int ip_str_to_addr(const char *str, struct ip_addr *ip);

/**
 * Compares addr1 in the ip_addr struct with addr2 in string representation.
 * @param[in] addr1 ip_addr
 * @param[in] addr2 IP-address as string
 * @return true If a == b
 * @return false If a != b
*/
bool ip_str_cmp(const struct ip_addr *addr1, const char *addr2);

#endif
/* @} */
