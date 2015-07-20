/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
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
struct pfx_table {
    struct lpfst_node *ipv4;
    struct lpfst_node *ipv6;
    pfx_update_fp update_fp;
    pthread_rwlock_t lock;
};

#endif
/* @} */
