/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef LRTR_IPV6_PUBLIC_H
#define LRTR_IPV6_PUBLIC_H

#include <stdint.h>

/**
 * @brief Struct holding an IPv6 address in host byte order.
 */
struct lrtr_ipv6_addr {
	uint32_t addr[4]; /**< The IPv6 address. */
};

#endif /* LRTR_IPV6_PUBLIC_H */
