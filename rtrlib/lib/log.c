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

#include <arpa/inet.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "rtrlib/lib/log.h"


void dbg(const char *frmt, ...)
{
#ifndef NDEBUG
    va_list argptr;
    va_start(argptr, frmt);
    struct timeval tv;
    struct timezone tz;

    bool fail = true;
    if(gettimeofday(&tv, &tz) == 0) {
        struct tm tm;
        if(localtime_r(&tv.tv_sec, &tm) != NULL) {
            printf("(%04d/%02d/%02d %02d:%02d:%02d:%06ld): ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec);
            fail = false;
        }
    }

    if(fail)
        printf("(%jd): ", (intmax_t) time(0));

    vprintf(frmt, argptr);
    printf("\n");
#endif
    return;
}
