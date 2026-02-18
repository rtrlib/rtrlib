/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "rtrlib/aspa/aspa_array/aspa_array.h"
#include "rtrlib/aspa/aspa_private.h"
#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/rtrlib_export_private.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

static enum rtr_aspa_status aspa_table_notify_clients(struct rtr_aspa_table *aspa_table, struct rtr_aspa_record *record,
						      const struct rtr_socket *rtr_socket,
						      const enum rtr_aspa_operation_type operation_type)
{
	if (!aspa_table || !rtr_socket)
		return RTR_ASPA_ERROR;

	if (aspa_table->update_fp && record) {
		// Realloc in order not to expose internal record
		struct rtr_aspa_record rec = *record;

		if (record->provider_asns && record->provider_count > 0) {
			size_t size = sizeof(uint32_t) * record->provider_count;

			rec.provider_asns = rtr_malloc(size);

			if (rec.provider_asns == NULL) {
				return RTR_ASPA_ERROR;
			}

			memcpy(rec.provider_asns, record->provider_asns, size);
		} else {
			rec.provider_asns = NULL;
		}

		aspa_table->update_fp(aspa_table, rec, rtr_socket, operation_type);
	}

	return RTR_ASPA_SUCCESS;
}

static enum rtr_aspa_status aspa_store_create_node(struct aspa_store_node **store, struct rtr_socket *rtr_socket,
						   struct aspa_array *aspa_array, struct aspa_store_node ***new_node)
{
	if (!rtr_socket)
		return RTR_ASPA_ERROR;

	// Allocate new node
	struct aspa_store_node *new = rtr_malloc(sizeof(struct aspa_store_node));

	if (!new)
		return RTR_ASPA_ERROR;

	// Store socket and ASPA array
	new->rtr_socket = rtr_socket;
	new->aspa_array = aspa_array;

	// prepend new node
	new->next = *store; // may be NULL
	*store = new;

	if (new_node)
		*new_node = store;

	return RTR_ASPA_SUCCESS;
}

static void aspa_store_remove_node(struct aspa_store_node **node)
{
	struct aspa_store_node *tmp = *node;
	*node = (*node)->next;
	rtr_free(tmp);
}

static struct aspa_store_node **aspa_store_get_node(struct aspa_store_node **node, const struct rtr_socket *rtr_socket)
{
	if (!node || !*node || !rtr_socket)
		return NULL;

	while (*node) {
		if ((*node)->rtr_socket == rtr_socket)
			return node;
		node = &(*node)->next;
	}

	return NULL;
}

RTRLIB_EXPORT void rtr_aspa_table_init(struct rtr_aspa_table *aspa_table, rtr_aspa_update_fp update_fp)
{
	aspa_table->update_fp = update_fp;
	aspa_table->store = NULL;
	pthread_rwlock_init(&(aspa_table->lock), NULL);
	pthread_rwlock_init(&(aspa_table->update_lock), NULL);
}

static enum rtr_aspa_status aspa_table_remove_node(struct rtr_aspa_table *aspa_table, struct aspa_store_node **node,
						   bool notify)
{
	if (!node)
		return RTR_ASPA_ERROR;

	if (!*node)
		// Doesn't exist anymore
		return RTR_ASPA_SUCCESS;

	struct aspa_array *array = (*node)->aspa_array;
	struct rtr_socket *socket = (*node)->rtr_socket;

	// Remove node for socket
	aspa_store_remove_node(node);

	if (!array)
		// Doesn't exist anymore
		return RTR_ASPA_SUCCESS;

	// Notify clients about these records being removed
	if (notify) {
		for (size_t i = 0; i < array->size; i++)
			aspa_table_notify_clients(aspa_table, aspa_array_get_record(array, i), socket, RTR_ASPA_REMOVE);
	}

	// Release all records and their provider sets
	aspa_array_free(array, true);

	return RTR_ASPA_SUCCESS;
}

RTRLIB_EXPORT enum rtr_aspa_status rtr_aspa_table_src_remove(struct rtr_aspa_table *aspa_table,
							     struct rtr_socket *rtr_socket, bool notify)
{
	if (aspa_table == NULL) {
		ASPA_DBG1("ASPA table is not initialized. Nothing to remove.");
		return RTR_ASPA_SUCCESS;
	}

	pthread_rwlock_wrlock(&aspa_table->update_lock);
	pthread_rwlock_wrlock(&aspa_table->lock);

	struct aspa_store_node **node = aspa_store_get_node(&aspa_table->store, rtr_socket);

	if (!node || !*node) {
		// Already gone
		pthread_rwlock_unlock(&(aspa_table->lock));
		return RTR_ASPA_SUCCESS;
	}

	enum rtr_aspa_status res = aspa_table_remove_node(aspa_table, node, notify);

	pthread_rwlock_unlock(&(aspa_table->lock));
	pthread_rwlock_unlock(&aspa_table->update_lock);
	return res;
}

RTRLIB_EXPORT void rtr_aspa_table_free(struct rtr_aspa_table *aspa_table, bool notify)
{
	if (aspa_table == NULL) {
		ASPA_DBG1("ASPA table is not initialized. Nothing to free.");
		return;
	}

	pthread_rwlock_wrlock(&aspa_table->update_lock);
	pthread_rwlock_wrlock(&aspa_table->lock);

	// Free store
	while (aspa_table->store)
		aspa_table_remove_node(aspa_table, &aspa_table->store, notify);

	aspa_table->store = NULL;

	pthread_rwlock_unlock(&aspa_table->lock);
	pthread_rwlock_destroy(&aspa_table->lock);
	pthread_rwlock_unlock(&aspa_table->update_lock);
	pthread_rwlock_destroy(&aspa_table->update_lock);
}

enum rtr_aspa_status aspa_table_src_replace(struct rtr_aspa_table *dst, struct rtr_aspa_table *src,
					    struct rtr_socket *rtr_socket, bool notify_dst, bool notify_src)
{
	if (!dst || !src || !rtr_socket || src == dst)
		return RTR_ASPA_ERROR;

	pthread_rwlock_wrlock(&dst->update_lock);
	pthread_rwlock_wrlock(&src->update_lock);
	pthread_rwlock_wrlock(&dst->lock);
	pthread_rwlock_wrlock(&src->lock);

	struct aspa_store_node **src_node = aspa_store_get_node(&src->store, rtr_socket);

	if (!src_node || !*src_node || !(*src_node)->aspa_array) {
		pthread_rwlock_unlock(&src->lock);
		pthread_rwlock_unlock(&dst->lock);
		pthread_rwlock_unlock(&src->update_lock);
		pthread_rwlock_unlock(&dst->update_lock);
		return RTR_ASPA_ERROR;
	}

	struct aspa_array *new_array = (*src_node)->aspa_array;
	struct aspa_array *old_array = NULL;

	// Try to get an existing node in the source table's store
	struct aspa_store_node **existing_dst_node = aspa_store_get_node(&dst->store, rtr_socket);

	if (existing_dst_node && *existing_dst_node) {
		// Swap array
		old_array = (*existing_dst_node)->aspa_array;
		(*existing_dst_node)->aspa_array = new_array;
	} else {
		// There's no old_array.
		// Destination table hasn't got an existing store node for the socket, so create a new one
		if (aspa_store_create_node(&dst->store, rtr_socket, new_array, NULL) != RTR_ASPA_SUCCESS) {
			pthread_rwlock_unlock(&src->lock);
			pthread_rwlock_unlock(&dst->lock);
			pthread_rwlock_unlock(&src->update_lock);
			pthread_rwlock_unlock(&dst->update_lock);
			return RTR_ASPA_ERROR;
		}
	}

	// Remove socket from source table's store
	aspa_store_remove_node(src_node);
	pthread_rwlock_unlock(&src->lock);
	pthread_rwlock_unlock(&dst->lock);
	pthread_rwlock_unlock(&src->update_lock);
	pthread_rwlock_unlock(&dst->update_lock);

	if (notify_src)
		// Notify src clients their records are being removed
		for (size_t i = 0; i < new_array->size; i++)
			aspa_table_notify_clients(src, aspa_array_get_record(new_array, i), rtr_socket,
						  RTR_ASPA_REMOVE);

	if (old_array) {
		if (notify_dst)
			// Notify dst clients of their existing records are being removed
			for (size_t i = 0; i < old_array->size; i++)
				aspa_table_notify_clients(dst, aspa_array_get_record(old_array, i), rtr_socket,
							  RTR_ASPA_REMOVE);

		// Free the old array and their provider sets
		aspa_array_free(old_array, true);
	}

	if (notify_dst)
		// Notify dst clients the records from src are added
		for (size_t i = 0; i < new_array->size; i++)
			aspa_table_notify_clients(dst, aspa_array_get_record(new_array, i), rtr_socket, RTR_ASPA_ADD);

	return RTR_ASPA_SUCCESS;
}

// MARK: - Updating an ASPA table

static int compare_update_operations(const void *a, const void *b)
{
	const struct aspa_update_operation *op1 = a;
	const struct aspa_update_operation *op2 = b;

	// compare index in case customer ASNs match, so result is stable
	if (op1->record.customer_asn < op2->record.customer_asn)
		return -1;
	else if (op1->record.customer_asn > op2->record.customer_asn)
		return 1;
	else if (op1->index > op2->index)
		return 1;
	else if (op1->index < op2->index)
		return -1;
	else
		return 0;
}

static int compare_asns(const void *aptr, const void *bptr)
{
	uint32_t a = *((uint32_t *)aptr);
	uint32_t b = *((uint32_t *)bptr);

	if (a < b)
		return -1;
	if (a > b)
		return 1;
	return 0;
}

// MARK: - Swap-In Update Mechanism

/**
 * @brief This function fills the given @p new_array with records based on a number of 'add' and 'remove' operations.
 *
 * @warning Do not call this function manually. This function fails if zero operations are supplied!
 */
static enum rtr_aspa_status aspa_table_update_compute_internal(struct rtr_socket *rtr_socket, struct aspa_array *array,
							   struct aspa_array *new_array,
							   struct aspa_update_operation *operations, size_t count,
							   struct aspa_update_operation **failed_operation,
							   struct aspa_array *replaced_records)
{
	// Fail hard in debug builds.
	assert(rtr_socket);
	assert(array);
	assert(operations);
	assert(count > 0);
	assert(failed_operation);

	size_t existing_i = 0;

	// preemptively allocate space for the arrays
	aspa_array_reserve(new_array, array->size + count);

	for (size_t i = 0; i < count; i++) {
		struct aspa_update_operation *current = &operations[i];
		struct aspa_update_operation *next = (i < count - 1) ? &(operations[i + 1]) : NULL;

#ifndef NDEBUG
		// Sanity check record
		if (current->type == RTR_ASPA_REMOVE) {
			assert(current->record.provider_count == 0);
			assert(!current->record.provider_asns);
		}
#endif

		// Sort providers.
		// We consider this an implementation detail, callers must not make any assumptions on the
		// ordering of provider ASNs.
		if (current->record.provider_count > 0 && current->record.provider_asns)
			qsort(current->record.provider_asns, current->record.provider_count, sizeof(uint32_t),
			      compare_asns);

		while (existing_i < array->size) {
			struct rtr_aspa_record *existing_record = aspa_array_get_record(array, existing_i);

			// Skip over records untouched by these add/remove operations
			if (existing_record->customer_asn < current->record.customer_asn) {
				existing_i += 1;

				// Append existing record (reuse existing provider array)
				if (aspa_array_append(new_array, existing_record, false) != RTR_ASPA_SUCCESS) {
					*failed_operation = current;
					return RTR_ASPA_ERROR;
				}
			} else {
				break;
			}
		}

		struct rtr_aspa_record *existing_record = aspa_array_get_record(array, existing_i);

		// existing record and current op have matching CAS
		bool existing_matches_current = existing_record &&
						existing_record->customer_asn == current->record.customer_asn;

		// next record and current op have matching CAS
		bool next_matches_current = next && next->record.customer_asn == current->record.customer_asn;

		// MARK: Handling 'add' operations
		if (current->type == RTR_ASPA_ADD) {
			// Attempt to add record with $CAS twice.
			// Error: Duplicate Add.
			if (next_matches_current && next->type == RTR_ASPA_ADD) {
				*failed_operation = next;
				current->record.provider_asns = NULL;
				return RTR_ASPA_DUPLICATE_RECORD;
			}

			// This operation adds a new ASPA record with some $CAS; the next operation, however,
			// removes the ASPA record for the same $CAS again.
			//
			// Independently, if a record for this customer number already exists or not, both
			// instructions cancel each other out and will remove an already present ASPA record.
			if (next_matches_current && next->type == RTR_ASPA_REMOVE) {
				// Scenario 1
				if (existing_matches_current) {
					// "Remove" record by simply not appending it to the new array
					existing_i += 1;

					// Since the combined outcome of the current and the next operation is the
					// removal of the existing record, we mark the current replace/add-operation
					// as no-op and adjust the remove operation so that it will be notified as
					// removal of the previously existing record.
					current->is_no_op = true;
					next->record.provider_count = existing_record->provider_count;
					next->record.provider_asns = existing_record->provider_asns;
				} else { // Scenario 2
#if ASPA_NOTIFY_NO_OPS
					// Complete record's providers for clients
					next->record = current->record;
#endif

					// Mark as no-op.
					next->is_no_op = true;
					current->is_no_op = true;
				}

				// Skip next
				i += 1;
				continue;
			}

			// Add record by appending it to new array (copy providers)
			if (aspa_array_append(new_array, &current->record, true) != RTR_ASPA_SUCCESS) {
				*failed_operation = current;
				return RTR_ASPA_ERROR;
			}

			if (existing_matches_current) {
				// Store the replaced ASPA record so that its provider ASN list can be freed when all
				// operations were successful to avoid memory leaks. Copying the provider ASNs must not
				// be done, otherwise we free the copied list but not the original one and thus have
				// memory leaks again.
				if (aspa_array_append(replaced_records, existing_record, false) != RTR_ASPA_SUCCESS) {
					*failed_operation = current;
					return RTR_ASPA_ERROR;
				}
			}

			// If it's an add operation, we insert a reference to the newly created record's providers.
			current->record.provider_asns =
				aspa_array_get_record(new_array, new_array->size - 1)->provider_asns;
		}

		// MARK: Handling 'remove' operations
		else if (current->type == RTR_ASPA_REMOVE) {
			// Attempt to remove record with $CAS, but record with $CAS does not exist
			// Error: Removal of unknown record.
			if (!existing_matches_current) {
				*failed_operation = current;
				return RTR_ASPA_RECORD_NOT_FOUND;
			}

			// Attempt to remove record with $CAS twice.
			// Error: Removal of unknown record.
			if (next_matches_current && next->type == RTR_ASPA_REMOVE) {
				*failed_operation = next;
				return RTR_ASPA_RECORD_NOT_FOUND;
			}

			// "Remove" record by simply not appending it to the new array
			existing_i += 1;

			// If it's a remove operation, we insert a reference to the removed record's providers.
			current->record.provider_count = existing_record->provider_count;
			current->record.provider_asns = existing_record->provider_asns;
		}
	}

	struct rtr_aspa_record *last_record_from_new_array = aspa_array_get_record(new_array, new_array->size - 1);
	struct rtr_aspa_record *next_record_from_old_array = aspa_array_get_record(array, existing_i);

	// If the customer AS number of the next element in the old array is already in the new array, skip that
	// element and don't copy it to the new array below.
	if (last_record_from_new_array && next_record_from_old_array &&
	    last_record_from_new_array->customer_asn == next_record_from_old_array->customer_asn) {
		existing_i += 1;
	}

	// Append remaining records (reuse existing provider array)
	for (; existing_i < array->size; existing_i++)
		aspa_array_append(new_array, aspa_array_get_record(array, existing_i), false);

	return RTR_ASPA_SUCCESS;
}

enum rtr_aspa_status aspa_table_update_swap_in_compute(struct rtr_aspa_table *aspa_table, struct rtr_socket *rtr_socket,
						       struct aspa_update_operation *operations, size_t count,
						       struct aspa_update **update)
{
	// Fail hard in debug builds.
	assert(aspa_table);
	assert(rtr_socket);
	assert(update);

	if (!aspa_table || !rtr_socket || !update || ((count > 0) && !operations))
		return RTR_ASPA_ERROR;

	if (count == 0)
		return RTR_ASPA_SUCCESS;

	if (!*update) {
		*update = rtr_malloc(sizeof(struct aspa_update));

		if (!*update)
			return RTR_ASPA_ERROR;
	}

	// MARK: Update Lock
	// Prevent interim writes to the table: We need to make sure the table stays the same
	// until the cleanup since the computed update is based on the state the table is in right now
	pthread_rwlock_wrlock(&aspa_table->update_lock);

	(*update)->table = aspa_table;
	(*update)->operations = operations;
	(*update)->operation_count = count;
	(*update)->failed_operation = NULL;
	(*update)->replaced_records = NULL;

	// stable sort operations, so operations dealing with the same customer ASN
	// are located right next to each other
	qsort(operations, count, sizeof(struct aspa_update_operation), compare_update_operations);

	pthread_rwlock_rdlock(&aspa_table->lock);
	struct aspa_store_node **node = aspa_store_get_node(&aspa_table->store, rtr_socket);

	pthread_rwlock_unlock(&aspa_table->lock);

	if (!node || !*node) {
		// The given table doesn't have a node for that socket, so create one
		struct aspa_array *a = NULL;

		if (aspa_array_create(&a) != RTR_ASPA_SUCCESS)
			return RTR_ASPA_ERROR;

		// Insert into table
		pthread_rwlock_wrlock(&aspa_table->lock);
		if (aspa_store_create_node(&aspa_table->store, rtr_socket, a, &node) != RTR_ASPA_SUCCESS || !node ||
		    !*node) {
			aspa_array_free(a, false);
			pthread_rwlock_unlock(&aspa_table->lock);
			return RTR_ASPA_ERROR;
		}
		pthread_rwlock_unlock(&aspa_table->lock);
	}

	assert(node);
	assert(*node);
	assert((*node)->aspa_array);

	// Create new array that will hold updated record data
	struct aspa_array *new_array = NULL;

	if (aspa_array_create(&new_array) != RTR_ASPA_SUCCESS) {
		return RTR_ASPA_ERROR;
	}

	struct aspa_array *replaced_records = NULL;

	if (aspa_array_create(&replaced_records) != RTR_ASPA_SUCCESS) {
		aspa_array_free(new_array, false);
		return RTR_ASPA_ERROR;
	}

	// Populate new_array
	pthread_rwlock_rdlock(&aspa_table->lock);
	enum rtr_aspa_status res = aspa_table_update_compute_internal(rtr_socket, (*node)->aspa_array, new_array,
								  operations, count, &(*update)->failed_operation,
								  replaced_records);
	pthread_rwlock_unlock(&aspa_table->lock);

	if (res == RTR_ASPA_SUCCESS) {
		(*update)->node = *node;
		(*update)->new_array = new_array;
		(*update)->replaced_records = replaced_records;
	} else {
		(*update)->node = NULL;
		(*update)->new_array = NULL;
		(*update)->replaced_records = NULL;

		// Update computation failed so release newly created array and
		// the replaced ASPA records that have been stored. We must not
		// release associated provider arrays here.
		aspa_array_free(new_array, false);
		aspa_array_free(replaced_records, false);
	}
	return res;
}

static void aspa_table_update_swap_in_consume(struct aspa_update **update_pointer, const bool apply)
{
	// Not going to re-apply update or apply if computation failed
	if (!update_pointer)
		return;

	struct aspa_update *update = *update_pointer;

	if (!update)
		return;

	struct aspa_array *old_array = NULL;

	if (apply) {
		if (!update->table || !update->operations || !update->node || !update->new_array ||
		    !!update->failed_operation)
			return;

		old_array = update->node->aspa_array;
		pthread_rwlock_wrlock(&update->table->lock);
		update->node->aspa_array = update->new_array;
		pthread_rwlock_unlock(&update->table->lock);
	}

	// Prevent access
	*update_pointer = NULL;

	// MARK: Change Lock
	// We're done with the update, allow changes again.
	pthread_rwlock_unlock(&update->table->update_lock);

	// Handle notifications and cleanup...
	for (size_t i = 0; i < update->operation_count; i++) {
		struct aspa_update_operation *op = &update->operations[i];

		// TODO: @carl check if this can get optimized away
		if (apply) {
			// Notify clients
			// We can directly use the operation array as the source for the diff
			// so we don't need to rely on first notifying clients about all records
			// in the old_array being removed and then every record in the new_array
			// being added again.
#if ASPA_NOTIFY_NO_OPS
			aspa_table_notify_clients(update->table, &op->record, update->node->rtr_socket, op->type);
#else
			if (!op->is_no_op)
				aspa_table_notify_clients(update->table, &op->record, update->node->rtr_socket,
							  op->type);
#endif

			// If it's a remove operation, we need to deallocate the existing record's
			// provider array as it is no longer present in the new ASPA array.
			if (!op->is_no_op && op->type == RTR_ASPA_REMOVE) {
				if (op->record.provider_asns)
					rtr_free(op->record.provider_asns);

				op->record.provider_asns = NULL;
				op->record.provider_count = 0;
			}
		} else {
			// No cleanup necessary for operations that haven't even been performed in the first place
			if (update->failed_operation && (op == update->failed_operation))
				break;

			// If it's an add operation, we need to deallocate the newly created record's
			// provider array as it is not needed
			if (!op->is_no_op && op->type == RTR_ASPA_ADD) {
				if (op->record.provider_asns)
					rtr_free(op->record.provider_asns);

				op->record.provider_asns = NULL;
			}
		}
	}

	// Free
	if (apply && old_array)
		aspa_array_free(old_array, false);

	if (!apply && update->new_array)
		aspa_array_free(update->new_array, false);

	if (apply && update->replaced_records) {
		// The new table is now active and thus the replaced ASPA records of the old array can be freed.
		aspa_array_free(update->replaced_records, true);
	}

	if (update->operations)
		rtr_free(update->operations);

	update->operations = NULL;
	update->operation_count = 0;
	update->failed_operation = NULL;
	update->new_array = NULL;
	update->node = NULL;
	update->table = NULL;
	rtr_free(update);
}

void aspa_table_update_swap_in_apply(struct aspa_update **update_pointer)
{
	aspa_table_update_swap_in_consume(update_pointer, true);
}

void aspa_table_update_swap_in_discard(struct aspa_update **update_pointer)
{
	aspa_table_update_swap_in_consume(update_pointer, false);
}
