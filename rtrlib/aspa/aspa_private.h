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

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Swap root nodes of the argument tables
 * @param[in,out] a First table
 * @param[in,out] b second table
 */
void aspa_table_swap(struct aspa_table *a, struct aspa_table *b);

/**
 * @brief Copy content of @p src_table into @p dst_table
 * @details dst must be empty and initialized
 * @param[in] src_table Source table
 * @param[out] dst_table Destination table
 * @param[in] socket socket which prefixes should not be copied
 */
int aspa_table_copy_except_socket(struct aspa_table *src_table, struct aspa_table *dst_table,
				  const struct rtr_socket *socket);

/**
 * @brief Notify client about changes between to aspa tables regarding one specific socket
 * @details @c old_table will be modified it should be freed after calling this function
 * @param[in] new_table New table
 * @param[in] old_table Old table
 * @param[in] socket socket which prefixes should be diffed
 */
void aspa_table_notify_diff(struct aspa_table *new_table, struct aspa_table *old_table,
			    const struct rtr_socket *socket);

#endif
/** @} */
