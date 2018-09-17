/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

/**
 * @defgroup mod_trie_pfx_h Trie
 * @ingroup mod_pfx_h
 * @brief An implementation of a \ref mod_pfx_h "pfx_table" data structure
 * using a shortest prefix first tree (trie) for storing @ref pfx_record "pfx_records".
 * @details This implementation uses two separate lpfs-trees, one for IPv4
 * validation records and one for IPv6 records.\n
 * See \ref mod_pfx_h "pfx_table" for a list of supported operations of
 * this data structure.\n
 *
 * @{
 */

#ifndef RTR_TRIE_PFX_PRIVATE
#define RTR_TRIE_PFX_PRIVATE
#include <pthread.h>

#include "rtrlib/pfx/pfx_private.h"

/**
 * @brief pfx_table.
 * @param ipv4
 * @param ipv6
 * @param update_fp
 * @param lock
 */
struct pfx_table {
    struct trie_node *ipv4;
    struct trie_node *ipv6;
    pfx_update_fp update_fp;
    pthread_rwlock_t lock;
};

#endif
/* @} */
