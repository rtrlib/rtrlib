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

#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "rtrlib/lib/utils.h"

#ifdef __MACH__
#include <mach/mach_time.h>
static double timeconvert = 0.0;
#endif

int rtr_get_monotonic_time(time_t *seconds)
{
#ifdef __MACH__
  if (timeconvert == 0.0) {
    mach_timebase_info_data_t timeBase;
    (void)mach_timebase_info( &timeBase );
    timeconvert = (double)timeBase.numer /
                  (double)timeBase.denom /
                  1000000000.0;
  }
  *seconds = (time_t) mach_absolute_time() * timeconvert;
#else
    struct timespec time;
    if(clock_gettime(CLOCK_MONOTONIC, &time) == -1)
        return -1;
    *seconds = time.tv_sec;
    if((time.tv_nsec *  1000000000) >=5)
        *seconds +=1;
#endif
    return 0;
}

uint32_t rtr_get_bits(const uint32_t val, const uint8_t from, const uint8_t number)
{
    assert(number < 33);
    assert(number > 0);

    uint32_t mask = ~0;
    if(number != 32)
        mask = ~(mask >> number);

    mask >>= from;
    return (mask & val);
}
