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

#define ASPA_UPDATE_IN_PLACE 1

/**
 * @brief A linked list storing the bond between a socket and an @c aspa_array .
 */
struct aspa_store_node {
	struct aspa_array *aspa_array;
	struct rtr_socket *rtr_socket;
	struct aspa_store_node *next;
};

/**
 * @brief Replaces all ASPA records associated with the given socket with the records in the src table.
 * @param[in,out] dst The destination table. Existing records associated with the socket are replaced.
 * @param[in,out] src The source table.
 * @param[in,out] rtr_socket The socket the records are associated with.
 * @param notify_dst A boolean value determining whether to notify the destination tables' clients.
 * @param notify_src A boolean value determining whether to notify the source tables' clients.
 */
enum aspa_status aspa_table_src_replace(struct aspa_table *dst, struct aspa_table *src, struct rtr_socket *rtr_socket,
					bool notify_dst, bool notify_src);

// MARK: - Updating

/**
 * @brief An enum describing the type of operation the ASPA table should perform using any given ASPA record.
 */
enum aspa_operation_type {
	/** The existing record, identified by its customer ASN, shall be withdrawn from the ASPA table. */
	ASPA_REMOVE = 0,

	/** The new record, identified by its customer ASN, shall be added to the ASPA table. */
	ASPA_ADD = 1
};

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
 * @brief ASPA update finalization arguments
 * @param unused_provider_arrays Array of pointers to now unused provider sets.
 * @param unused_provider_array_len Number of unused provider sets.
 */
struct aspa_update_finalization_args {
	uint32_t **unused_provider_arrays;
	size_t unused_provider_array_len;
#ifndef ASPA_UPDATE_IN_PLACE
	struct aspa_array **old_array;
	struct aspa_array *new_array;
#endif
};

/**
 * @brief Updates the given ASPA table by adding and removing records.
 *
 * @param[in] aspa_table ASPA table to store new ASPA data in.
 * @param[in] rtr_socket The socket the updates originate from.
 * @param[in] operations  Add and remove operations to perform.
 * @param[in] len  Number of operations
 * @param[in] revert A boolean value indicating whether to apply the inverse the effect of the given operations (add <-> remove).
 * @param[in,out] failed_operation The operation responsible for causing the table update failure. @c NULL , if the update succeeded. If this function is called with @c revert set to @c false , treat this as an output parameter. If @c revert is @c true , this argument must not be @ NULL.
 * @param[out] cleanup_args Arguments for cleanup.
 * @return @c ASPA_SUCCESS On success.
 * @return @c ASPA_RECORD_NOT_FOUND If a records is supposed to be removed but cannot be found.
 * @return @c ASPA_DUPLICATE_RECORD If a records is supposed to be added but its corresponding customer ASN already exists.
 * @return @c ASPA_ERROR On on failure.
 *
 * @note You should only release the unused provider sets if you're sure you are not going to reverse this update (by calling with argument @c reverse set to @c true ).
 */
enum aspa_status aspa_table_update(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket,
				   struct aspa_update_operation *operations, size_t len, bool revert,
				   struct aspa_update_operation **failed_operation,
				   struct aspa_update_finalization_args **finalization_args);

/**
 * @brief Finalizes update
 * @param finalization_args Finalization arguments from aspa_table_update
 */
#ifdef ASPA_UPDATE_IN_PLACE
void aspa_update_finalize(struct aspa_update_finalization_args *finalization_args);
#else
void aspa_update_finalize(struct aspa_update_finalization_args *finalization_args, bool apply_update);
#endif

// MARK: - Verification

enum aspa_hop_result { ASPA_NO_ATTESTATION, ASPA_NOT_PROVIDER_PLUS, ASPA_PROVIDER_PLUS };

/**
 * @brief Checks a hop in the given @c AS_PATH .
 * @return @c aspa_hop_result .
 */
enum aspa_hop_result aspa_check_hop(struct aspa_table *aspa_table, uint32_t customer_asn, uint32_t provider_asn);

#endif
/** @} */
