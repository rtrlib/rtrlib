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

#ifndef RTR_UTILS_H
#define RTR_UTILS_H
#include <time.h>
#include "rtrlib/lib/ip.h"

int rtr_get_monotonic_time(time_t* seconds);
int rtr_ipaddr_to_str(const ip_addr* addr, char* result, const size_t len);
int rtr_str_to_ipaddr(const char* str, ip_addr* ip_addr);
bool rtr_cmp_ipv4(const ip_addr* addr1, const char* addr2);
bool rtr_cmp_ipv6(const ip_addr* addr1, const char* addr2);

#endif
