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

#include "../rtr/rtr.h"
#include "aspa.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief An enum describing the type of operation the ASPA table should perform using any given ASPA record.
 */
enum aspa_operation_type {
	/** The existing record, identified by its customer ASN, shall be withdrawn from the ASPA table. */
	ASPA_REMOVE = 0,

	/** The new record, identified by its customer ASN, shall be added to the ASPA table. */
	ASPA_ADD = 1
};

enum aspa_hop_result { ASPA_NO_ATTESTATION, ASPA_NOT_PROVIDER_PLUS, ASPA_PROVIDER_PLUS };

/**
 * @brief A struct describing a specific type of operation that should be performed using the attached ASPA record.
 * @param index A value uniquely identifying this operation's position within the array of operations.
 * @param type The operation's type.
 * @param record The record that should be added or removed.
 */
struct aspa_update_operation {
	size_t index;
	enum aspa_operation_type type;
	struct aspa_record record;
};

/**
 * @brief A type describing an update to an ASPA table than can be later be applied.
 */
struct aspa_update {
	struct aspa_table *aspa_table;
	struct rtr_socket *rtr_socket;
	struct aspa_array *aspa_array;
	size_t stale_provider_arrays_count;
	uint32_t **stale_provider_arrays;
};

/**
 * @brief Computes an update to an ASPA table that can be applied or discard later.
 *
 * @param[in] aspa_table ASPA table to store new ASPA data in.
 * @param[in] operations  Add or remove operations to perform.
 * @param[in] count  Number of operations
 * @param[in] rtr_socket The socket the updates originate from.
 * @param[out] update The computed update.
 * @param[out] failed_operation The operation responsible for causing the table update failure. @c NULL , if the update succeeds.
 * @return @c ASPA_SUCCESS On success.
 * @return @c ASPA_RECORD_NOT_FOUND If a records is supposed to be removed but cannot be found.
 * @return @c ASPA_DUPLICATE_RECORD If a records is supposed to be added but its corresponding customer ASN already exists.
 * @return @c ASPA_ERROR On on failure.
 */
enum aspa_rtvals aspa_table_compute_update(struct aspa_table *aspa_table, struct aspa_update_operation *operations,
					   size_t count, struct rtr_socket *rtr_socket, struct aspa_update *update,
					   struct aspa_update_operation **failed_operation);

/**
 * @brief Applies a previously computed ASPA update.
 * @param[in] update  ASPA update that should be applied.
 */
enum aspa_rtvals aspa_table_apply_update(struct aspa_update *update);

/**
 * @brief Discards a previously computed ASPA update.
 * @param[in] update  ASPA update
 */
void aspa_table_free_update(struct aspa_update *update);

/**
 * @brief Checks a hop in the given @c AS_PATH .
 * @return @c aspa_hop_result .
 */
enum aspa_hop_result aspa_check_hop(struct aspa_table *aspa_table, uint32_t customer_asn, uint32_t provider_asn);

#endif
/** @} */
