/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include <arpa/inet.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "rtrlib/lib/log_private.h"


void lrtr_dbg(const char *frmt, ...)
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
    va_end(argptr);
#endif
    return;
}
