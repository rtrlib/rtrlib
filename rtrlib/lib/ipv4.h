/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef RTR_IPV4_H
#define RTR_IPV4_H

#include <stdint.h>

/**
 * @brief Struct storing an IPv4 address in host byte order.
 * @param addr The IPv4 address.
 */
struct rtr_ipv4_addr {
	uint32_t addr;
};

#endif
