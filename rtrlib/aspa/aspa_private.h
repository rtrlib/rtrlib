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
 * @brief The @p aspa_table is an abstract data structure to organize the validated Autonomous System Provider
 * Authorization  data received from an RPKI-RTR cache server.
 *
 * # Updating an ASPA table
 * ASPA tables implement aggregated updating using an array of 'add record' and 'remove record' operations --
 * reducing iterations and memory allocations.  E.g., these operations can be derived from a RTR cache response.
 * Currently, two distinct update mechanisms are supported: **Swap-In** and **In-Place** updates. Use the macro
 * `ASPA_UPDATE_MECHANISM` in rtr/packets.c to configure which implementation is used during syncing.
 * The array of operations is effectively a diff to the table's previous state. This diff can be conveniently used to
 * notify callers about changes once the update is applied.
 *
 * ## Swap-In Update Mechanism
 * The ASPA table's **Swap-In** update mechanism avoids blocking callers who want to
 * verify an `AS_PATH` (and therefore need read access to the table) while an update is in progress and removes the
 * need for an *undo mechanism* in case the update to the ASPA table itself or some other action performed
 * inbetween fails.
 *
 * Performing an update using this mechanism involves these steps:
 * - **Compute Update**:
 *   Every time you want to update a given ASPA table, call `aspa_table_update_swap_in_compute`.
 *   This will create a new ASPA array, appending both existing records and new records. Everything needed to update
 *   the table is stored in an update structure.
 * - **Apply Update** or **Discard Update**:
 *   You either have to apply the update using `aspa_table_update_swap_in_apply` or discard it by calling
 *   `aspa_table_update_swap_in_discard`. This will either swap in the newly created ASPA array and notify
 *   clients about changes or discard and release data that's now unused.
 *
 * The implementation guarantess no changes are made to the ASPA table between
 * calling `aspa_table_update_swap_in_compute` and `aspa_table_update_swap_in_apply`
 * or `aspa_table_update_swap_in_discard`.
 *
 * ## In-Place Update Mechanism
 * The ASPA table's **In-Place** update mechanism involves in-place modifications to the array of records
 * and an undo function that undoes changes made previously.
 *
 * Performing an update using this mechanism involves these steps:
 * - **Update**:
 *   Every time you want to update a given ASPA table, call `aspa_table_update_in_place`. This will modify the ASPA
 *   array. If the update fails, `failed_operation` will be set to the operation where the error occurring.
 * - **Undo Update**:
 *   You may, but do not need to, undo the update using `aspa_table_update_in_place_undo`. This will undo all
 *   operations up to `failed_operation` or all operations.
 * - **Clean Up**:
 *   After computing the update you should go through a cleanup step using `aspa_table_update_in_place_cleanup`.
 *   This will deallocate provider arrays and other data created during the update that's now unused.
 *
 * ## Special Cases
 * There're various cases that need to be handled appropriately by both implementations.
 *   1. **Add existing record**:
 *     The caller attempts to add a record that's already present in the table (`ASPA_DUPLICATE_RECORD`).
 *   2. **Duplicate adds**:
 *     The caller attempts to add two or more records with the same customer ASN (`ASPA_DUPLICATE_RECORD`).
 *   3. **Removal of unknown record**:
 *     The caller attempts to remove a record from the table that doesn't exist (`ASPA_RECORD_NOT_FOUND`).
 *   4. **Duplicate removal**:
 *     The caller attempts to remove a record twice or more (`ASPA_RECORD_NOT_FOUND`).
 *   5. **Complementary add/remove**:
 *     The caller attempts to first add a record and then wants to remove the same record. This is equivalent to a
 *     no-op. `ASPA_NOTIFY_NO_OPS` (`1` or `0`) determines if clients are notified about these no-ops.
 *
 * ## Implementation Details
 * Both update mechanism implementations tackle the beforementioned cases by first sorting the array of 'add' and
 * 'remove' operations by their customer ASN stably. That is, 'add' and 'remove' operations dealing with matching
 * customer ASNs will remain in the same order as they arrived. This makes checking for cases 2 - *Duplicate
 * Announcement* and 4 - *Duplicate Removal* easy as possible duplicates are neighbors in the operations array.
 * Ordering the operations also enables skipping annihilating operations as described in case 5 - *Complementary
 * Announcement/Withdrawal*.
 * Both implementations are comprised of a loop iterating over operations and a nested loop that handles
 * records from the existing ASPA array with an ASN smaller than the current operation's ASN.
 * - If the record in the existing array and the current 'add' operation have a matching customer ASN,
 *   that's case 1 - *Announcement of Existing Record*.
 * - If the record in the existing array and the current 'remove' operation do not have a matching customer ASN,
 *   that's case 3 - *Removal of Unknown Record*.
 *
 * @{
 */

#ifndef RTR_ASPA_PRIVATE_H
#define RTR_ASPA_PRIVATE_H

#include "aspa.h"

#include "rtrlib/rtr/rtr.h"

#include <stdbool.h>
#include <stdint.h>

#define ASPA_NOTIFY_NO_OPS 0

// MARK: - Verification

enum aspa_hop_result { ASPA_NO_ATTESTATION, ASPA_NOT_PROVIDER_PLUS, ASPA_PROVIDER_PLUS };

/**
 * @brief Checks a hop in the given `AS_PATH`.
 * @return @c aspa_hop_result .
 */
enum aspa_hop_result aspa_check_hop(struct aspa_table *aspa_table, uint32_t customer_asn, uint32_t provider_asn);

// MARK: - Storage
/**
 * @brief A linked list storing the bond between a @p rtr_socket and an @p aspa_array .
 *
 * @param aspa_array The node's array of ASPA records.
 * @param rtr_socket The socket the records originate from.
 * @param next Next node in the linked list.
 */
struct aspa_store_node {
	struct aspa_array *aspa_array;
	struct rtr_socket *rtr_socket;
	struct aspa_store_node *next;
};

/**
 * @brief Replaces all ASPA records associated with the given socket with the records in the src table.
 *
 * @param[in,out] dst The destination table. Existing records associated with the socket are replaced.
 * @param[in,out] src The source table.
 * @param[in] rtr_socket The socket the records are associated with.
 * @param notify_dst A boolean value determining whether to notify the destination table's clients.
 * @param notify_src A boolean value determining whether to notify the source table's clients.
 * @return @c ASPA_SUCCESS if the operation succeeds, @c ASPA_ERROR if it fails.
 */
enum aspa_status aspa_table_src_replace(struct aspa_table *dst, struct aspa_table *src, struct rtr_socket *rtr_socket,
					bool notify_dst, bool notify_src);

// MARK: - Updating
/**
 * @brief A struct describing a specific type of operation that should be performed using the attached ASPA record.
 *
 * @param index A value uniquely identifying this operation's position within the array of operations.
 * @param type The operation's type.
 * @param record The record that should be added or removed.
 * @param is_no_op A boolean value indicating whether this operation is part of a pair
 * of 'add `<CAS>`' and 'remove `<CAS>`' operations that form a no-op.
 */
struct aspa_update_operation {
	size_t index;
	enum aspa_operation_type type;
	struct aspa_record record;
	bool is_no_op;
};

// MARK: - Swap-In Update Mechanism
/**
 * @brief Computed ASPA update.
 *
 * @param table The ASPA table the update was computed for.
 * @param operations The array of update operations used to compute this update.
 * @param operation_count The number of update operations in the operations array.
 * @param failed_operation An optional pointer to the operation that failed.
 * @param node The node in the given ASPA table's store whose ASPA array is going to be replaced
 * by @p new_array if this update is applied.
 * @param new_array The new ASPA array replacing the node's existing array.
 */
struct aspa_update {
	struct aspa_table *table;
	struct aspa_update_operation *operations;
	size_t operation_count;
	struct aspa_update_operation *failed_operation;
	struct aspa_store_node *node;
	struct aspa_array *new_array;
};

/**
 * @brief Computes an update structure that can later be applied to the given ASPA table.
 *
 * @note Each record in an 'add' operation may have a provider array associated with it. Any record in a 'remove'
 * operation must have its `provider_count` set to 0 and `provider_array` set to `NULL`.
 * @note This function acquires an update lock on the given ASPA table ensuring no mutations occur while
 * computing the update or before the update is applied.
 * You must call `aspa_table_update_swap_in_apply` or `aspa_table_update_swap_in_discard`
 * to either apply or discard the update.
 *
 * @param[in] aspa_table ASPA table to store new ASPA data in.
 * @param[in] rtr_socket The socket the updates originate from.
 * @param[in] operations  Add and remove operations to perform.
 * @param[in] count  Number of operations.
 * @param update The computed update. The update pointer must be non-NULL, but may point to a @c NULL
 * value initially. Points to an update struct after  this function returns.
 * @return @c ASPA_SUCCESS On success.
 * @return @c ASPA_RECORD_NOT_FOUND If a records is supposed to be removed but cannot be found.
 * @return @c ASPA_DUPLICATE_RECORD If a records is supposed to be added but its corresponding
 * customer ASN already exists.
 * @return @c ASPA_ERROR On other failures.
 */
enum aspa_status aspa_table_update_swap_in_compute(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket,
						   struct aspa_update_operation *operations, size_t count,
						   struct aspa_update **update);

/**
 * @brief Applys the given update, as previously computed by `aspa_table_update_swap_in_compute`,
 * releases memory allocated while computing the update and unlocks update lock. The update is consumed.
 *
 * @param update The update that will be applied.
 */
void aspa_table_update_swap_in_apply(struct aspa_update **update);

/**
 * @brief Discards the given update, releases memory allocated while computing the update and unlocks update lock.
 * The update is consumed.
 *
 * @param update The update to discard.
 */
void aspa_table_update_swap_in_discard(struct aspa_update **update);

// MARK: - In-Place Update Mechanism

/**
 * @brief Updates the given ASPA table.
 *
 * @note Each record in an 'add' operation may have a provider array associated with it. Any record in a 'remove'
 * operation must have its `provider_count` set to `0` and `provider_array` set to `NULL`.
 *
 * @param[in] aspa_table ASPA table to store new ASPA data in.
 * @param[in] rtr_socket The socket the updates originate from.
 * @param[in] operations  Add and remove operations to perform.
 * @param[in] count  Number of operations.
 * @param[out] failed_operation Failed operation, filled in if update fails.
 * @return @c ASPA_SUCCESS On success.
 * @return @c ASPA_RECORD_NOT_FOUND If a records is supposed to be removed but cannot be found.
 * @return @c ASPA_DUPLICATE_RECORD If a records is supposed to be added but its corresponding customer ASN already
 * exists.
 * @return @c ASPA_ERROR On other failures.
 */
enum aspa_status aspa_table_update_in_place(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket,
					    struct aspa_update_operation *operations, size_t count,
					    struct aspa_update_operation **failed_operation);

/**
 * @brief Tries to undo @c operations up to @p failed_operation and then releases all operations.
 *
 * @param[in] aspa_table ASPA table to store new ASPA data in.
 * @param[in] rtr_socket The socket the updates originate from.
 * @param[in] operations  Add and remove operations to perform.
 * @param[in] count  Number of operations.
 * @param[in] failed_operation Failed operation.
 * @return @c ASPA_SUCCESS On success.
 * @return @c ASPA_RECORD_NOT_FOUND If a records is supposed to be removed but cannot be found.
 * @return @c ASPA_DUPLICATE_RECORD If a records is supposed to be added but its corresponding customer ASN already
 * exists.
 * @return @c ASPA_ERROR On other failures.
 */
enum aspa_status aspa_table_update_in_place_undo(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket,
						 struct aspa_update_operation *operations, size_t count,
						 struct aspa_update_operation *failed_operation);

/**
 * @brief Releases operations and unused provider arrays.
 * @param[in] operations  Add and remove operations.
 * @param[in] count  Number of operations.
 */
void aspa_table_update_in_place_cleanup(struct aspa_update_operation **operations, size_t count);

#endif /* RTR_ASPA_PRIVATE_H */
/** @} */
