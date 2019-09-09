/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

/**
 * @defgroup mod_pfx_h Prefix validation table
 * @brief The pfx_table is an abstract data structure to organize the validated prefix origin data
 * received from an RPKI-RTR cache server.
 *
 * @{
 */

#ifndef RTR_PFX_PRIVATE_H
#define RTR_PFX_PRIVATE_H

#include "pfx.h"

#include "rtrlib/lib/ip_private.h"

#include <stdint.h>

/**
 * @brief Frees all memory associated with the pfx_table without calling the update callback.
 * @param[in] pfx_table pfx_table that will be freed.
 */
void pfx_table_free_without_notify(struct pfx_table *pfx_table);

/**
 * @brief Swap root nodes of the argument tables
 * @param[in,out] a First table
 * @param[in,out] b second table
 */
void pfx_table_swap(struct pfx_table *a, struct pfx_table *b);

/**
 * @brief Copy content of @p src_table into @p dst_table
 * @details dst must be empty and initialized
 * @param[in] src_table Source table
 * @param[out] dst_table Destination table
 * @param[in] socket socket which prefixes should not be copied
 */
int pfx_table_copy_except_socket(struct pfx_table *src_table, struct pfx_table *dst_table,
				 const struct rtr_socket *socket);

/**
 * @brief Notify client about changes between to pfx tables regarding one specific socket
 * @details old_table will be modified it should be freed after calling this function
 * @param[in] new_table
 * @param[in] old_table
 * @param[in] socket socket which prefixes should be diffed
 */
void pfx_table_notify_diff(struct pfx_table *new_table, struct pfx_table *old_table, const struct rtr_socket *socket);

#endif
/** @} */
