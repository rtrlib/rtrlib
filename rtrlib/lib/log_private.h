/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef LRTR_LOG_PRIVATE_H
#define LRTR_LOG_PRIVATE_H

/**
 * @brief Writes a message to stdout if NDEBUG isn't defined.
 * @param[in] frmt log message in printf format style.
 */
void lrtr_dbg(const char *frmt, ...) __attribute__ ((format (printf, 1, 2)));

#endif
