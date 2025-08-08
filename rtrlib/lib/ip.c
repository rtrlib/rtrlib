/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "ip_private.h"

#include "rtrlib/rtrlib_export_private.h"

#include <stdbool.h>
#include <string.h>

bool rtr_ip_addr_is_zero(const struct rtr_ip_addr prefix)
{
	if (prefix.ver == RTR_IPV6) {
		if (prefix.u.addr6.addr[0] == 0 && prefix.u.addr6.addr[1] == 0 && prefix.u.addr6.addr[2] == 0 &&
		    prefix.u.addr6.addr[3] == 0) {
			return true;
		}
	} else if (prefix.u.addr4.addr == 0) {
		return true;
	}

	return false;
}

struct rtr_ip_addr rtr_ip_addr_get_bits(const struct rtr_ip_addr *val, const uint8_t from, const uint8_t number)
{
	struct rtr_ip_addr result;

	if (val->ver == RTR_IPV6) {
		result.ver = RTR_IPV6;
		result.u.addr6 = rtr_ipv6_get_bits(&(val->u.addr6), from, number);
	} else {
		result.ver = RTR_IPV4;
		result.u.addr4 = rtr_ipv4_get_bits(&(val->u.addr4), from, number);
	}
	return result;
}

RTRLIB_EXPORT bool rtr_ip_addr_equal(const struct rtr_ip_addr a, const struct rtr_ip_addr b)
{
	if (a.ver != b.ver)
		return false;
	if (a.ver == RTR_IPV6)
		return rtr_ipv6_addr_equal(&(a.u.addr6), &(b.u.addr6));
	return rtr_ipv4_addr_equal(&(a.u.addr4), &(b.u.addr4));
}

RTRLIB_EXPORT int rtr_ip_addr_to_str(const struct rtr_ip_addr *ip, char *str, const unsigned int len)
{
	if (ip->ver == RTR_IPV6)
		return rtr_ipv6_addr_to_str(&(ip->u.addr6), str, len);
	return rtr_ipv4_addr_to_str(&(ip->u.addr4), str, len);
}

RTRLIB_EXPORT int rtr_ip_str_to_addr(const char *str, struct rtr_ip_addr *ip)
{
	if (!strchr(str, ':')) {
		ip->ver = RTR_IPV4;
		return rtr_ipv4_str_to_addr(str, &(ip->u.addr4));
	}
	ip->ver = RTR_IPV6;
	return rtr_ipv6_str_to_addr(str, &(ip->u.addr6));
}

// cppcheck-suppress unusedFunction
RTRLIB_EXPORT bool rtr_ip_str_cmp(const struct rtr_ip_addr *addr1, const char *addr2)
{
	struct rtr_ip_addr tmp;

	if (rtr_ip_str_to_addr(addr2, &tmp) == -1)
		return false;
	return rtr_ip_addr_equal(*addr1, tmp);
}
