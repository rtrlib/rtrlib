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

#ifndef RTR_TRIE_PFX
#define RTR_TRIE_PFX

#include "rtrlib/lib/ip.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

struct pfx_table;

/**
 * @brief pfx_record.
 * @param asn Origin AS number.
 * @param prefix IP prefix.
 * @param min_len Minimum prefix length.
 * @param max_len Maximum prefix length.
 * @param socket The rtr_socket that received this record.
 */
struct pfx_record {
	uint32_t asn;
	struct lrtr_ip_addr prefix;
	uint8_t min_len;
	uint8_t max_len;
	const struct rtr_socket *socket;
};

/**
 * @brief A function pointer that is called if an record was added to the pfx_table or was removed from the pfx_table.
 * @param pfx_table which was updated.
 * @param record pfx_record that was modified.
 * @param added True if the record was added, false if the record was removed.
 */
typedef void (*pfx_update_fp)(struct pfx_table *pfx_table, const struct pfx_record record, const bool added);

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
/** @} */
