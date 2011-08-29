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
 * Webpage: http://rpki.realmv6.org/
 */

#ifndef RTR_TEST_H
#define RTR_TEST_H
#include "rtrlib/lib/ip.h"
/*
 * @brief Utilities for testing the library
 */

void put_ipv4(ip_addr* res, char* addr);
void put_ipv6(ip_addr* res, char* addr);
void get_ipv4(ip_addr* res, char* addr);
void get_ipv6(ip_addr* res, char* addr);
bool cmp_ipv4(ip_addr* res, char* cmp);
bool cmp_ipv6(ip_addr* res, char* cmp);
void print_bytes(void* buf, size_t len);

#endif
