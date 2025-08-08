/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef RTR_IP_PUBLIC_H
#define RTR_IP_PUBLIC_H

#include "rtrlib/lib/ipv4.h"
#include "rtrlib/lib/ipv6.h"

#include <stdbool.h>
/**
 * @defgroup util_h Utility functions
 * @{
 */

/**
 * @brief Version of the IP protocol.
 */
enum rtr_ip_version {
	/** IPV4 */
	RTR_IPV4,

	/** RTR_IPV6 */
	RTR_IPV6
};

/**
 * @brief The rtr_ip_addr struct stores a IPv4 or IPv6 address in host byte order.
 * @param ver Specifies the type of the stored address.
 * @param u Union holding a rtr_ipv4_addr or rtr_ipv6_addr.
 */
struct rtr_ip_addr {
	enum rtr_ip_version ver;
	union {
		struct rtr_ipv4_addr addr4;
		struct rtr_ipv6_addr addr6;
	} u;
};

/**
 * Converts the passed rtr_ip_addr struct to string representation.
 * @param[in] ip rtr_ip_addr
 * @param[out] str Pointer to a char array.
 * The array must be at least INET_ADDRSTRLEN bytes long if the passed rtr_ip_addr stores an IPv4 address.
 * If rtr_ip_addr stores an IPv6 address, str must be at least INET6_ADDRSTRLEN bytes long.
 * @param[in] len Length of the str array.
 * @result 0 On success.
 * @result -1 On error.
 */
int rtr_ip_addr_to_str(const struct rtr_ip_addr *ip, char *str, const unsigned int len);

/**
 * Converts the passed IP address in string representation to an rtr_ip_addr.
 * @param[in] str Pointer to a Null terminated char array.
 * @param[out] ip Pointer to a rtr_ip_addr struct.
 * @result 0 On success.
 * @result -1 On error.
 */
int rtr_ip_str_to_addr(const char *str, struct rtr_ip_addr *ip);

/**
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
