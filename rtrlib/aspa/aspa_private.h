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
 * @brief Moves all ASPA records associated with the given socket from the source table into the destination table.
 * @param[in,out] dst The destination table. Existing records associated with the socket are replaced.
 * @param[in,out] src The source table.
 * @param[in,out] rtr_socket The socket the records are associated with.
 * @param notify_dst A boolean value determining whether to notify the destination tables' clients.
 * @param notify_src A boolean value determining whether to notify the source tables' clients.
 */
int aspa_table_src_move(struct aspa_table *dst, struct aspa_table *src, struct rtr_socket *rtr_socket, bool notify_dst,
			bool notify_src);

#endif
/** @} */
