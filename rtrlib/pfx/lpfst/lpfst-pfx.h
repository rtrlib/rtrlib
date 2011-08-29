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

/**
 * @defgroup mod_lpfst_pfx_h Longest Prefix Tree
 * @ingroup mod_pfx_h
 * @brief An implementation of a pfx_table datastructure using longest prefix first trees (lpfst) for storing 
 * pfx_records. This implementation uses 2 seperate lpfs-trees, one for IPv4 validation records and one IPv6 records.\n
 * See \ref mod_pfx_h "pfx_table" for a list of supported operations of this datastructure.
 *
 * @{
 */

#ifndef RTR_LPFST_PFX
#define RTR_LPFST_PFX
#include "rtrlib/pfx/pfx.h"
#include "rtrlib/pfx/lpfst/lpfst.h"

typedef struct pfx_table{
    lpfst_node* ipv4;
    lpfst_node* ipv6;
    rtr_update_fp* update_fp;
    unsigned int update_fp_len;
} pfx_table;

#endif
/* @} */
