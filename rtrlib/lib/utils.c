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
 * written by Fabian Holler, in cooperation with:
 * INET group, Hamburg University of Applied Sciences,
 * CST group, Freie Universitaet Berlin
 * Website: http://rpki.realmv6.org/
 */

#include "rtrlib/lib/utils.h"
#include <time.h>

int get_monotonic_time(time_t* seconds){
    struct timespec time;
    if(clock_gettime(CLOCK_MONOTONIC, &time) == -1)
        return -1;
    *seconds = time.tv_sec;
    if((time.tv_nsec *  1000000000) >=5)
        *seconds +=1;
    return 0;
}
