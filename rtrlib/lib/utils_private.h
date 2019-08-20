/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef LRTR_UTILS_PRIVATE_H
#define LRTR_UTILS_PRIVATE_H

#include <stdint.h>
#include <time.h>

/**
 * @brief Returns the current time of the CLOCK_MONOTONIC clock.
 * @param[in] seconds Time in seconds since some unspecified starting point.
 * @return 0 on successs
 * @return -1 on error
 */
int lrtr_get_monotonic_time(time_t *seconds);

/**
 * @brief Extracts number bits from the passed uint32_t, starting at bit number from. The bit with the highest
 * significance is bit 0. All bits that aren't in the specified range will be 0.
 * @param[in] val uint32_t
 * @param[in] from Position of the first bit that is extracted.
 * @param[in] number How many bits will be extracted.
 * @returns a uint32_t, where all bits that aren't in the specified range are set to 0.
 */
uint32_t lrtr_get_bits(const uint32_t val, const uint8_t from, const uint8_t number);

#endif
