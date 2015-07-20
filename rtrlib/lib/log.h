/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
*/

#ifndef RTR_LOG_H
#define RTR_LOG_H
#include <time.h>
#include <stdlib.h>
#include "rtrlib/lib/ip.h"

/**
 * @brief Writes a message to stdout if NDEBUG isn't defined.
 * @param[in] frmt log message in printf format style.
 */
void dbg(const char *frmt, ...) __attribute__ ((format (printf, 1, 2)));

#endif
