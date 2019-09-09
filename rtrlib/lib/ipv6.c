/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "ipv6_private.h"

#include "rtrlib/lib/convert_byte_order_private.h"
#include "rtrlib/lib/ipv4_private.h"
#include "rtrlib/lib/utils_private.h"

#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

inline bool lrtr_ipv6_addr_equal(const struct lrtr_ipv6_addr *a, const struct lrtr_ipv6_addr *b)
{
	if (a->addr[0] == b->addr[0] && a->addr[1] == b->addr[1] && a->addr[2] == b->addr[2] &&
	    a->addr[3] == b->addr[3])
		return true;
	return false;
}

struct lrtr_ipv6_addr lrtr_ipv6_get_bits(const struct lrtr_ipv6_addr *val, const uint8_t first_bit,
					 const uint8_t quantity)
{
	assert(first_bit <= 127);
	assert(quantity <= 128);
	assert(first_bit + quantity <= 128);

	// if no bytes get extracted the result has to be 0
	struct lrtr_ipv6_addr result;

	memset(&result, 0, sizeof(result));

	uint8_t bits_left = quantity;

	if (first_bit <= 31) {
		const uint8_t q = quantity > 32 ? 32 : quantity;

		assert(bits_left >= q);
		bits_left -= q;
		result.addr[0] = lrtr_get_bits(val->addr[0], first_bit, q);
	}

	if ((first_bit <= 63) && ((first_bit + quantity) > 32)) {
		const uint8_t fr = first_bit < 32 ? 0 : first_bit - 32;
		const uint8_t q = bits_left > 32 ? 32 : bits_left;

		assert(bits_left >= q);
		bits_left -= q;
		result.addr[1] = lrtr_get_bits(val->addr[1], fr, q);
	}

	if ((first_bit <= 95) && ((first_bit + quantity) > 64)) {
		const uint8_t fr = first_bit < 64 ? 0 : first_bit - 64;
		const uint8_t q = bits_left > 32 ? 32 : bits_left;

		assert(bits_left >= q);
		bits_left -= q;
		result.addr[2] = lrtr_get_bits(val->addr[2], fr, q);
	}

	if ((first_bit <= 127) && ((first_bit + quantity) > 96)) {
		const uint8_t fr = first_bit < 96 ? 0 : first_bit - 127;
		const uint8_t q = bits_left > 32 ? 32 : bits_left;

		assert(bits_left >= q);
		result.addr[3] = lrtr_get_bits(val->addr[3], fr, q);
	}
	return result;
}

/*
 * This function was copied from the bird routing daemon's ip_pton(..) function.
 */
int lrtr_ipv6_str_to_addr(const char *a, struct lrtr_ipv6_addr *ip)
{
	uint32_t *o = ip->addr;
	uint16_t words[8];
	int i, j, k, l, hfil;
	const char *start;

	if (a[0] == ':') { /* Leading :: */
		if (a[1] != ':')
			return -1;
		a++;
	}
	hfil = -1;
	i = 0;
	while (*a) {
		if (*a == ':') { /* :: */
			if (hfil >= 0)
				return -1;
			hfil = i;
			a++;
			continue;
		}
		j = 0;
		l = 0;
		start = a;
		for (;;) {
			if (*a >= '0' && *a <= '9')
				k = *a++ - '0';
			else if (*a >= 'A' && *a <= 'F')
				k = *a++ - 'A' + 10;
			else if (*a >= 'a' && *a <= 'f')
				k = *a++ - 'a' + 10;
			else
				break;
			j = (j << 4) + k;
			if (j >= 0x10000 || ++l > 4)
				return -1;
		}
		if (*a == ':' && a[1]) {
			a++;
		} else if (*a == '.' && (i == 6 || (i < 6 && hfil >= 0))) { /* Embedded IPv4 address */
			struct lrtr_ipv4_addr addr4;

			if (lrtr_ipv4_str_to_addr(start, &addr4) == -1)
				return -1;
			words[i++] = addr4.addr >> 16;
			words[i++] = addr4.addr;
			break;
		} else if (*a) {
			return -1;
		}
		if (i >= 8)
			return -1;
		words[i++] = j;
	}

	/* Replace :: with an appropriate quantity of zeros */
	if (hfil >= 0) {
		j = 8 - i;
		for (i = 7; i - j >= hfil; i--)
			words[i] = words[i - j];
		for (; i >= hfil; i--)
			words[i] = 0;
	}

	/* Convert the address to lrtr_ip_addr format */
	for (i = 0; i < 4; i++)
		o[i] = (words[2 * i] << 16) | words[2 * i + 1];
	return 0;
}

/*
 * This function was copied from the bird routing daemon's ip_ntop(..) function.
 */
int lrtr_ipv6_addr_to_str(const struct lrtr_ipv6_addr *ip_addr, char *b, const unsigned int len)
{
	if (len < INET6_ADDRSTRLEN)
		return -1;
	const uint32_t *a = ip_addr->addr;
	uint16_t words[8];
	int bestpos = 0;
	int bestlen = 0;
	int curpos = 0;
	int curlen = 0;
	int i;

	/* First of all, preprocess the address and find the longest run of zeros */
	for (i = 0; i < 8; i++) {
		uint32_t x = a[i / 2];

		words[i] = ((i % 2) ? x : (x >> 16)) & 0xffff;
		if (words[i]) {
			curlen = 0;
		} else {
			if (!curlen)
				curpos = i;
			curlen++;
			if (curlen > bestlen) {
				bestpos = curpos;
				bestlen = curlen;
			}
		}
	}
	if (bestlen < 2)
		bestpos = -1;

	/* Is it an encapsulated IPv4 address? */
	if (!bestpos && ((bestlen == 5 && a[2] == 0xffff) || bestlen == 6))
	// if (!bestpos && ((bestlen == 5 && (a[2] == 0xffff)) || bestlen == 6))
	{
		uint32_t x = a[3];

		b += sprintf(b, "::%s%d.%d.%d.%d", a[2] ? "ffff:" : "", ((x >> 24) & 0xff), ((x >> 16) & 0xff),
			     ((x >> 8) & 0xff), (x & 0xff));
		return 0;
	}

	/* Normal IPv6 formatting, compress the largest sequence of zeros */
	for (i = 0; i < 8; i++) {
		if (i == bestpos) {
			i += bestlen - 1;
			*b++ = ':';
			if (i == 7)
				*b++ = ':';
		} else {
			if (i)
				*b++ = ':';
			b += sprintf(b, "%x", words[i]);
		}
	}
	*b = '\0';
	return 0;
}

void lrtr_ipv6_addr_convert_byte_order(const uint32_t *src, uint32_t *dest, const enum target_byte_order tbo)
{
	for (int i = 0; i < 4; i++)
		dest[i] = lrtr_convert_long(tbo, src[i]);
}
