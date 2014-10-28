/*
 * This file is part of RTRlib.
 *
 * RTRlib is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * RTRlib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RTRlib; see the file COPYING.LESSER.
 *
 * INET group, Hamburg University of Applied Sciences,
 * CST group, Freie Universitaet Berlin
 * Website: http://rpki.realmv6.org/
 */

#ifndef RTR_UTILS_H
#define RTR_UTILS_H
#include <time.h>
#include "rtrlib/lib/ip.h"

/**
 * @brief Returns the current time of the CLOCK_MONOTONIC clock.
 * @param[in] seconds Time in seconds since some unspecified starting point.
 * @return 0 on successs
 * @return -1 on error
 */
int rtr_get_monotonic_time(time_t *seconds);

/**
 * @brief Extracts number bits from the passed uint32_t, starting at bit number from. The bit with the highest
 * significance is bit 0. All bits that aren't in the specified range will be 0.
 * @param[in] val uint32_t
 * @param[in] from Position of the first bit that is extracted.
 * @param[in] number How many bits will be extracted.
 * @returns a uint32_t, where all bits that aren't in the specified range are set to 0.
*/
uint32_t rtr_get_bits(const uint32_t val, const uint8_t from, const uint8_t number);

#endif
