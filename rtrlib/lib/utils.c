/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "utils_private.h"

#include <assert.h>
#include <stdint.h>
#include <time.h>

#ifdef __MACH__
#include <mach/mach_time.h>
static double timeconvert = 0.0;
#endif

int lrtr_get_monotonic_time(time_t *seconds)
{
#ifdef __MACH__
	if (timeconvert == 0.0) {
		mach_timebase_info_data_t timeBase;
		(void)mach_timebase_info(&timeBase);
		timeconvert = (double)timeBase.numer / (double)timeBase.denom / 1000000000.0;
	}
	*seconds = (time_t)mach_absolute_time() * timeconvert;
#else
	struct timespec time;
	if (clock_gettime(CLOCK_MONOTONIC, &time) == -1)
		return -1;
	*seconds = time.tv_sec;
	if ((time.tv_nsec * 1000000000) >= 5)
		*seconds += 1;
#endif
	return 0;
}

uint32_t lrtr_get_bits(const uint32_t val, const uint8_t from, const uint8_t number)
{
	assert(number < 33);
	assert(number > 0);

	uint32_t mask = ~0;
	if (number != 32)
		mask = ~(mask >> number);

	mask >>= from;
	return (mask & val);
}
