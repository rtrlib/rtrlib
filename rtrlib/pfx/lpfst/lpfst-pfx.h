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

/**
 * @defgroup mod_lpfst_pfx_h Longest prefix first search tree
 * @ingroup mod_pfx_h
 * @brief An implementation of a \ref mod_pfx_h "pfx_table" data structure
 * using longest prefix first trees (lpfst) for storing @ref pfx_record "pfx_records".
 * @details This implementation uses two separate lpfs-trees, one for IPv4
 * validation records and one for IPv6 records.\n
 * See \ref mod_pfx_h "pfx_table" for a list of supported operations of
 * this data structure.\n
 * For general details about the data structure see: <a
 * href="http://dx.doi.org/10.1016/j.comnet.2007.01.023">Wuu et al. "A
 * longest prefix first search tree for IP lookup", Computer Networks,
 * 51(12), pp. 3354-3367, 2007</a>.
 *
 * @{
 */

#ifndef RTR_LPFST_PFX
#define RTR_LPFST_PFX
#include "rtrlib/pfx/pfx.h"
#include "rtrlib/pfx/lpfst/lpfst.h"

/**
 * @brief pfx_table.
 * @param ipv4
 * @param ipv6
 * @param update_fp
 * @param lock
 */
typedef struct pfx_table{
    lpfst_node* ipv4;
    lpfst_node* ipv6;
    pfx_update_fp update_fp;
    pthread_rwlock_t lock;
} pfx_table;

#endif
/* @} */
