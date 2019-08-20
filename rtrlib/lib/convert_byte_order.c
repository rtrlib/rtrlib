/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "convert_byte_order_private.h"

#include <arpa/inet.h>
#include <assert.h>
#include <inttypes.h>

uint16_t lrtr_convert_short(const enum target_byte_order tbo, const uint16_t value)
{
	if (tbo == TO_NETWORK_BYTE_ORDER)
		return htons(value);
	else if (tbo == TO_HOST_HOST_BYTE_ORDER)
		return ntohs(value);

	assert(0);
}

uint32_t lrtr_convert_long(const enum target_byte_order tbo, const uint32_t value)
{
	if (tbo == TO_NETWORK_BYTE_ORDER)
		return htonl(value);
	else if (tbo == TO_HOST_HOST_BYTE_ORDER)
		return ntohl(value);

	assert(0);
}
