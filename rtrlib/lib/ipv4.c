/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "ipv4_private.h"

#include "rtrlib/lib/convert_byte_order_private.h"
#include "rtrlib/lib/utils_private.h"

#include <assert.h>
#include <stdio.h>

struct lrtr_ipv4_addr lrtr_ipv4_get_bits(const struct lrtr_ipv4_addr *val, const uint8_t from, const uint8_t quantity)
{
	struct lrtr_ipv4_addr result;

	result.addr = lrtr_get_bits(val->addr, from, quantity);
	return result;
}

int lrtr_ipv4_addr_to_str(const struct lrtr_ipv4_addr *ip, char *str, unsigned int len)
{
	uint8_t buff[4];

	buff[0] = ip->addr >> 24 & 0xff;
	buff[1] = ip->addr >> 16 & 0xff;
	buff[2] = ip->addr >> 8 & 0xff;
	buff[3] = ip->addr & 0xff;

	if (snprintf(str, len, "%hhu.%hhu.%hhu.%hhu", buff[0], buff[1], buff[2], buff[3]) < 0)
		return -1;

	return 0;
}

int lrtr_ipv4_str_to_addr(const char *str, struct lrtr_ipv4_addr *ip)
{
	uint8_t buff[4];

	if (sscanf(str, "%3hhu.%3hhu.%3hhu.%3hhu", &buff[0], &buff[1], &buff[2], &buff[3]) != 4)
		return -1;

	ip->addr = buff[0] << 24 | buff[1] << 16 | buff[2] << 8 | buff[3];

	return 0;
}

bool lrtr_ipv4_addr_equal(const struct lrtr_ipv4_addr *a, const struct lrtr_ipv4_addr *b)
{
	if (a->addr == b->addr)
		return true;

	return false;
}

void lrtr_ipv4_addr_convert_byte_order(const uint32_t src, uint32_t *dest, const enum target_byte_order tbo)
{
	*dest = lrtr_convert_long(tbo, src);
}
