/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

/**
 * @defgroup mod_aspa_h ASPA validation table
 *
 * @brief The aspa_table is an abstract data structure to organize the validated Autonomous System Provider Authorization  data
 * received from an RPKI-RTR cache server.
 *
 * @{
 */

#ifndef RTR_ASPA_PRIVATE_H
#define RTR_ASPA_PRIVATE_H

#include "aspa.h"

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initializes a new ASPA table serving a cache for records associated with the given `rtr_socket`.
 * Note that records not associated with the given socket cannot be added or removed from the table.
 * @param aspa_table The ASPA table to initialize as a cache.
 * @param rtr_socket The socket whose records are to be cached.
 */
int aspa_table_cache_init(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket);

/**
 * @brief Writes all cached records from the cache table into the destination table.
 * @param cache_table The ASPA table initialized as a cache.
 * @param dest_table The ASPA table where the cached records are written into.
 */
int aspa_table_cache_writeback(struct aspa_table *cache_table, struct aspa_table *dest_table);

#endif
/** @} */
