/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef LRTR_CONVERT_BYTE_ORDER_PRIVATE_H
#define LRTR_CONVERT_BYTE_ORDER_PRIVATE_H

#include <inttypes.h>

/**
 * @brief Target byte order for conversion.
 */
enum target_byte_order {
	TO_NETWORK_BYTE_ORDER,
	TO_HOST_HOST_BYTE_ORDER,
};

/**
 * Converts the passed short value to the given target byte order.
 * @param[in] tbo Target byte order.
 * @param[in] value Input (uint16_t) for conversion.
 * @result Converted uint16_t value.
 */
uint16_t lrtr_convert_short(const enum target_byte_order tbo, const uint16_t value);

/**
 * Converts the passed long value to the given target byte order.
 * @param[in] tbo Target byte order.
 * @param[in] value Input (uint32_t) for conversion.
 * @result Converted uint32_t value.
 */
uint32_t lrtr_convert_long(const enum target_byte_order tbo, const uint32_t value);

#endif /* LRTR_CONVERT_BYTE_ORDER_H */
