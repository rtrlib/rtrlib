/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef LRTR_IP_PUBLIC_H
#define LRTR_IP_PUBLIC_H

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
enum lrtr_ip_version {
	/** IPV4 */
	LRTR_IPV4,

	/** LRTR_IPV6 */
	LRTR_IPV6
};

/**
 * @brief The lrtr_ip_addr struct stores a IPv4 or IPv6 address in host byte order.
 * @param ver Specifies the type of the stored address.
 * @param u Union holding a lrtr_ipv4_addr or lrtr_ipv6_addr.
 */
struct lrtr_ip_addr {
	enum lrtr_ip_version ver;
	union {
		struct lrtr_ipv4_addr addr4;
		struct lrtr_ipv6_addr addr6;
	} u;
};

/**
 * Converts the passed lrtr_ip_addr struct to string representation.
 * @param[in] ip lrtr_ip_addr
 * @param[out] str Pointer to a char array.
 * The array must be at least INET_ADDRSTRLEN bytes long if the passed lrtr_ip_addr stores an IPv4 address.
 * If lrtr_ip_addr stores an IPv6 address, str must be at least INET6_ADDRSTRLEN bytes long.
 * @param[in] len Length of the str array.
 * @result 0 On success.
 * @result -1 On error.
 */
int lrtr_ip_addr_to_str(const struct lrtr_ip_addr *ip, char *str, const unsigned int len);

/**
 * Converts the passed IP address in string representation to an lrtr_ip_addr.
 * @param[in] str Pointer to a Null terminated char array.
 * @param[out] ip Pointer to a lrtr_ip_addr struct.
 * @result 0 On success.
 * @result -1 On error.
 */
int lrtr_ip_str_to_addr(const char *str, struct lrtr_ip_addr *ip);

/**
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
