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

static enum aspa_status aspa_table_notify_clients(struct aspa_table *aspa_table, struct aspa_record *record,
						  const struct rtr_socket *rtr_socket,
						  const enum aspa_operation_type operation_type)
{
	if (!aspa_table || !rtr_socket)
		return ASPA_ERROR;

	if (aspa_table->update_fp && record) {
		// Realloc in order not to expose internal record
		struct aspa_record rec = *record;

		if (record->provider_asns && record->provider_count > 0) {
			size_t size = sizeof(uint32_t) * record->provider_count;

			rec.provider_asns = lrtr_malloc(size);
			memcpy(rec.provider_asns, record->provider_asns, size);
		} else {
			rec.provider_asns = NULL;
		}

		aspa_table->update_fp(aspa_table, rec, rtr_socket, operation_type);
	}

	return ASPA_SUCCESS;
}

static enum aspa_status aspa_store_create_node(struct aspa_store_node **store, struct rtr_socket *rtr_socket,
					       struct aspa_array *aspa_array, struct aspa_store_node ***new_node)
{
	if (!rtr_socket)
		return ASPA_ERROR;

	// Allocate new node
	struct aspa_store_node *new = lrtr_malloc(sizeof(struct aspa_store_node));

	if (!new)
		return ASPA_ERROR;

	// Store socket and ASPA array
	new->rtr_socket = rtr_socket;
	new->aspa_array = aspa_array;

	// prepend new node
	new->next = *store; // may be NULL
	*store = new;

	if (new_node)
		*new_node = store;

	return ASPA_SUCCESS;
}

static void aspa_store_remove_node(struct aspa_store_node **node)
{
	struct aspa_store_node *tmp = *node;
	*node = (*node)->next;
	lrtr_free(tmp);
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

RTRLIB_EXPORT void aspa_table_init(struct aspa_table *aspa_table, aspa_update_fp update_fp)
{
	aspa_table->update_fp = update_fp;
	aspa_table->store = NULL;
	pthread_rwlock_init(&(aspa_table->lock), NULL);
	pthread_rwlock_init(&(aspa_table->update_lock), NULL);
}

static enum aspa_status aspa_table_remove_node(struct aspa_table *aspa_table, struct aspa_store_node **node,
					       bool notify)
{
	if (!node)
		return ASPA_ERROR;

	if (!*node)
		// Doesn't exist anymore
		return ASPA_SUCCESS;

	struct aspa_array *array = (*node)->aspa_array;
	struct rtr_socket *socket = (*node)->rtr_socket;

	// Remove node for socket
	aspa_store_remove_node(node);

	if (!array)
		// Doesn't exist anymore
		return ASPA_SUCCESS;

	// Notify clients about these records being removed
	if (notify) {
		for (size_t i = 0; i < array->size; i++)
			aspa_table_notify_clients(aspa_table, aspa_array_get_record(array, i), socket, false);
	}

	// Release all records and their provider sets
	aspa_array_free(array, true);

	return ASPA_SUCCESS;
}

RTRLIB_EXPORT enum aspa_status aspa_table_src_remove(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket,
						     bool notify)
{
	pthread_rwlock_wrlock(&aspa_table->update_lock);
	pthread_rwlock_wrlock(&aspa_table->lock);

	struct aspa_store_node **node = aspa_store_get_node(&aspa_table->store, rtr_socket);

	if (!node || !*node) {
		// Already gone
		pthread_rwlock_unlock(&(aspa_table->lock));
		return ASPA_SUCCESS;
	}

	enum aspa_status res = aspa_table_remove_node(aspa_table, node, notify);

	pthread_rwlock_unlock(&(aspa_table->lock));
	pthread_rwlock_unlock(&aspa_table->update_lock);
	return res;
}

RTRLIB_EXPORT void aspa_table_free(struct aspa_table *aspa_table, bool notify)
{
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

enum aspa_status aspa_table_src_replace(struct aspa_table *dst, struct aspa_table *src, struct rtr_socket *rtr_socket,
					bool notify_dst, bool notify_src)
{
	if (!dst || !src || !rtr_socket || src == dst)
		return ASPA_ERROR;

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
		return ASPA_ERROR;
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
		if (aspa_store_create_node(&dst->store, rtr_socket, new_array, NULL) != ASPA_SUCCESS) {
			pthread_rwlock_unlock(&src->lock);
			pthread_rwlock_unlock(&dst->lock);
			pthread_rwlock_unlock(&src->update_lock);
			pthread_rwlock_unlock(&dst->update_lock);
			return ASPA_ERROR;
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
			aspa_table_notify_clients(src, aspa_array_get_record(new_array, i), rtr_socket, ASPA_REMOVE);

	if (old_array) {
		if (notify_dst)
			// Notify dst clients of their existing records are being removed
			for (size_t i = 0; i < old_array->size; i++)
				aspa_table_notify_clients(dst, aspa_array_get_record(old_array, i), rtr_socket,
							  ASPA_REMOVE);

		// Free the old array and their provider sets
		aspa_array_free(old_array, true);
	}

	if (notify_dst)
		// Notify dst clients the records from src are added
		for (size_t i = 0; i < new_array->size; i++)
			aspa_table_notify_clients(dst, aspa_array_get_record(new_array, i), rtr_socket, ASPA_ADD);

	return ASPA_SUCCESS;
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

static int compare_asns(const void *a, const void *b)
{
	return *(uint32_t *)a - *(uint32_t *)b;
}

// MARK: - Swap-In Update Mechanism

/**
 * @brief This function fills the given @p new_array with records based on a number of 'add' and 'remove' operations.
 *
 * @warning Do not call this function manually. This function fails if zero operations are supplied!
 */
static enum aspa_status aspa_table_update_compute_internal(struct rtr_socket *rtr_socket, struct aspa_array *array,
							   struct aspa_array *new_array,
							   struct aspa_update_operation *operations, size_t count,
							   struct aspa_update_operation **failed_operation)
{
	// Fail hard in debug builds.
	assert(rtr_socket);
	assert(array);
	assert(operations);
	assert(count > 0);
	assert(failed_operation);

	size_t existing_i = 0;

	for (size_t i = 0; i < count; i++) {
		struct aspa_update_operation *current = &operations[i];
		struct aspa_update_operation *next = (i < count - 1) ? &(operations[i + 1]) : NULL;

#ifndef NDEBUG
		// Sanity check record
		if (current->type == ASPA_REMOVE) {
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
			struct aspa_record *existing_record = aspa_array_get_record(array, existing_i);

			// Skip over records untouched by these add/remove operations
			if (existing_record->customer_asn < current->record.customer_asn) {
				existing_i += 1;

				// Append existing record (reuse existing provider array)
				if (aspa_array_append(new_array, existing_record, false) != ASPA_SUCCESS) {
					*failed_operation = current;
					return ASPA_ERROR;
				}
			} else {
				break;
			}
		}

		struct aspa_record *existing_record = aspa_array_get_record(array, existing_i);

		// existing record and current op have matching CAS
		bool existing_matches_current = existing_record &&
						existing_record->customer_asn == current->record.customer_asn;

		// next record and current op have matching CAS
		bool next_matches_current = next && next->record.customer_asn == current->record.customer_asn;

		// MARK: Handling 'add' operations
		if (current->type == ASPA_ADD) {
			// Attempt to add record with $CAS, but record with $CAS already exists
			// Error: Duplicate Add.
			if (existing_matches_current) {
				*failed_operation = current;
				return ASPA_DUPLICATE_RECORD;
			}

			// Attempt to add record with $CAS twice.
			// Error: Duplicate Add.
			if (next_matches_current && next->type == ASPA_ADD) {
				*failed_operation = next;
				current->record.provider_asns = NULL;
				return ASPA_DUPLICATE_RECORD;
			}

			// This operation adds a record with $CAS, the next op however removes this $CAS record again.
			// These form a no-op.
			if (next_matches_current && next->type == ASPA_REMOVE) {
#if ASPA_NOTIFY_NO_OPS
				// Complete record's providers for clients
				next->record = current->record;
#endif

				// Mark as no-op.
				current->is_no_op = true;
				next->is_no_op = true;

				// Skip next
				i += 1;
				continue;
			}

			// Add record by appending it to new array (copy providers)
			if (aspa_array_append(new_array, &current->record, true) != ASPA_SUCCESS) {
				*failed_operation = current;
				return ASPA_ERROR;
			}

			// If it's an add operation, we insert a reference to the newly created record's providers.
			current->record.provider_asns =
				aspa_array_get_record(new_array, new_array->size - 1)->provider_asns;
		}

		// MARK: Handling 'remove' operations
		else if (current->type == ASPA_REMOVE) {
			// Attempt to remove record with $CAS, but record with $CAS does not exist
			// Error: Removal of unknown record.
			if (!existing_matches_current) {
				*failed_operation = current;
				return ASPA_RECORD_NOT_FOUND;
			}

			// Attempt to remove record with $CAS twice.
			// Error: Removal of unknown record.
			if (next_matches_current && next->type == ASPA_REMOVE) {
				*failed_operation = next;
				return ASPA_RECORD_NOT_FOUND;
			}

			// "Remove" record by simply not appending it to the new array
			existing_i += 1;

			// If it's a remove operation, we insert a reference to the removed record's providers.
			current->record.provider_count = existing_record->provider_count;
			current->record.provider_asns = existing_record->provider_asns;
		}
	}

	// Append remaining records (reuse existing provider array)
	for (; existing_i < array->size; existing_i++)
		aspa_array_append(new_array, aspa_array_get_record(array, existing_i), false);

	return ASPA_SUCCESS;
}

enum aspa_status aspa_table_update_swap_in_compute(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket,
						   struct aspa_update_operation *operations, size_t count,
						   struct aspa_update **update)
{
	// Fail hard in debug builds.
	assert(aspa_table);
	assert(rtr_socket);
	assert(update);

	if (!aspa_table || !rtr_socket || !update || ((count > 0) && !operations))
		return ASPA_ERROR;

	if (count == 0)
		return ASPA_SUCCESS;

	if (!*update) {
		*update = lrtr_malloc(sizeof(struct aspa_update));

		if (!*update)
			return ASPA_ERROR;
	}

	// MARK: Update Lock
	// Prevent interim writes to the table: We need to make sure the table stays the same
	// until the cleanup since the computed update is based on the state the table is in right now
	pthread_rwlock_wrlock(&aspa_table->update_lock);

	(*update)->table = aspa_table;
	(*update)->operations = operations;
	(*update)->operation_count = count;
	(*update)->failed_operation = NULL;

	// stable sort operations, so operations dealing with the same customer ASN
	// are located right next to each other
	qsort(operations, count, sizeof(struct aspa_update_operation), compare_update_operations);

	pthread_rwlock_rdlock(&aspa_table->lock);
	struct aspa_store_node **node = aspa_store_get_node(&aspa_table->store, rtr_socket);

	pthread_rwlock_unlock(&aspa_table->lock);

	if (!node || !*node) {
		// The given table doesn't have a node for that socket, so create one
		struct aspa_array *a = NULL;

		if (aspa_array_create(&a) != ASPA_SUCCESS)
			return ASPA_ERROR;

		// Insert into table
		pthread_rwlock_wrlock(&aspa_table->lock);
		if (aspa_store_create_node(&aspa_table->store, rtr_socket, a, &node) != ASPA_SUCCESS || !node ||
		    !*node) {
			aspa_array_free(a, false);
			pthread_rwlock_unlock(&aspa_table->lock);
			return ASPA_ERROR;
		}
		pthread_rwlock_unlock(&aspa_table->lock);
	}

	assert(node);
	assert(*node);
	assert((*node)->aspa_array);

	// Create new array that will hold updated record data
	struct aspa_array *new_array = NULL;

	if (aspa_array_create(&new_array) != ASPA_SUCCESS)
		return ASPA_ERROR;

	// Populate new_array
	pthread_rwlock_rdlock(&aspa_table->lock);
	enum aspa_status res = aspa_table_update_compute_internal(rtr_socket, (*node)->aspa_array, new_array,
								  operations, count, &(*update)->failed_operation);
	pthread_rwlock_unlock(&aspa_table->lock);

	if (res == ASPA_SUCCESS) {
		(*update)->node = *node;
		(*update)->new_array = new_array;
	} else {
		(*update)->node = NULL;
		(*update)->new_array = NULL;

		// Update computation failed so release newly created array.
		// We must not release associated provider arrays here.
		aspa_array_free(new_array, false);
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
			if (!op->is_no_op && op->type == ASPA_REMOVE) {
				if (op->record.provider_asns)
					lrtr_free(op->record.provider_asns);

				op->record.provider_asns = NULL;
				op->record.provider_count = 0;
			}
		} else {
			// No cleanup necessary for operations that haven't even been performed in the first place
			if (update->failed_operation && (op == update->failed_operation))
				break;

			// If it's an add operation, we need to deallocate the newly created record's
			// provider array as it is not needed
			if (!op->is_no_op && op->type == ASPA_ADD) {
				if (op->record.provider_asns)
					lrtr_free(op->record.provider_asns);

				op->record.provider_asns = NULL;
			}
		}
	}

	// Free
	if (apply && old_array)
		aspa_array_free(old_array, false);

	if (!apply && update->new_array)
		aspa_array_free(update->new_array, false);

	if (update->operations)
		lrtr_free(update->operations);

	update->operations = NULL;
	update->operation_count = 0;
	update->failed_operation = NULL;
	update->new_array = NULL;
	update->node = NULL;
	update->table = NULL;
	lrtr_free(update);
}

void aspa_table_update_swap_in_apply(struct aspa_update **update_pointer)
{
	aspa_table_update_swap_in_consume(update_pointer, true);
}

void aspa_table_update_swap_in_discard(struct aspa_update **update_pointer)
{
	aspa_table_update_swap_in_consume(update_pointer, false);
}

// MARK: - In-Place Update Mechanism

enum aspa_status aspa_table_update_in_place(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket,
					    struct aspa_update_operation *operations, size_t count,
					    struct aspa_update_operation **failed_operation)
{
	// Fail hard in debug builds.
	assert(aspa_table);
	assert(rtr_socket);
	assert(failed_operation);

	if (!aspa_table || !rtr_socket || !failed_operation || ((count > 0) && !operations))
		return ASPA_ERROR;

	if (count == 0)
		return ASPA_SUCCESS;

	// stable sort operations, so operations dealing with the same customer ASN
	// are located right next to each other
	qsort(operations, count, sizeof(struct aspa_update_operation), compare_update_operations);

	pthread_rwlock_wrlock(&aspa_table->lock);
	struct aspa_store_node **node = aspa_store_get_node(&aspa_table->store, rtr_socket);

	if (!node || !*node) {
		// The given table doesn't have a node for that socket, so create one
		struct aspa_array *a = NULL;

		if (aspa_array_create(&a) != ASPA_SUCCESS)
			return ASPA_ERROR;

		// Insert into table
		if (aspa_store_create_node(&aspa_table->store, rtr_socket, a, &node) != ASPA_SUCCESS || !node ||
		    !*node) {
			aspa_array_free(a, false);
			pthread_rwlock_unlock(&aspa_table->lock);
			return ASPA_ERROR;
		}
	}

	assert(node);
	assert(*node);
	assert((*node)->aspa_array);

	struct aspa_array *array = (*node)->aspa_array;
	size_t existing_i = 0;

	for (size_t i = 0; i < count; i++) {
		struct aspa_update_operation *current = &operations[i];
		struct aspa_update_operation *next = (i < count - 1) ? &(operations[i + 1]) : NULL;

#ifndef NDEBUG
		// Sanity check record
		if (current->type == ASPA_REMOVE) {
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

		while (existing_i < array->size &&
		       aspa_array_get_record(array, existing_i)->customer_asn < current->record.customer_asn) {
			existing_i += 1;
		}

		struct aspa_record *existing_record = aspa_array_get_record(array, existing_i);

		// existing record and current op have matching CAS
		bool existing_matches_current = existing_record &&
						existing_record->customer_asn == current->record.customer_asn;

		// next record and current op have matching CAS
		bool next_matches_current = next && next->record.customer_asn == current->record.customer_asn;

		// MARK: Handling 'add' operations
		if (current->type == ASPA_ADD) {
			// Attempt to add record with $CAS, but record with $CAS already exists
			// Error: Duplicate Add.
			if (existing_matches_current) {
				*failed_operation = current;
				pthread_rwlock_unlock(&aspa_table->lock);
				return ASPA_DUPLICATE_RECORD;
			}

			// This operation adds a record with $CAS, the next op however removes this $CAS record again.
			if (next_matches_current && next->type == ASPA_REMOVE) {
#if ASPA_NOTIFY_NO_OPS
				// Complete record's providers for clients
				next->record = current->record;
				aspa_table_notify_clients(aspa_table, &current->record, rtr_socket, current->type);
				aspa_table_notify_clients(aspa_table, &next->record, rtr_socket, next->type);
#endif

				// Mark as no-op.
				current->is_no_op = true;
				next->is_no_op = true;

				// Skip next
				i += 1;
				continue;
			}

			// Add record by appending it to new array (copy providers)
			if (aspa_array_insert(array, existing_i, &current->record, true) != ASPA_SUCCESS) {
				*failed_operation = current;
				pthread_rwlock_unlock(&aspa_table->lock);
				return ASPA_ERROR;
			}
		}

		// MARK: Handling 'remove' operations
		else if (current->type == ASPA_REMOVE) {
			// Attempt to remove record with $CAS, but record with $CAS does not exist
			// Error: Removal of unknown record.
			if (!existing_matches_current) {
				*failed_operation = current;
				pthread_rwlock_unlock(&aspa_table->lock);
				return ASPA_RECORD_NOT_FOUND;
			}

			// If it's a remove operation, we insert a reference to the removed record's providers.
			current->record.provider_count = existing_record->provider_count;
			current->record.provider_asns = existing_record->provider_asns;

			// Remove record (don't release providers)
			if (aspa_array_remove(array, existing_i, false) != ASPA_SUCCESS) {
				*failed_operation = current;
				pthread_rwlock_unlock(&aspa_table->lock);
				return ASPA_RECORD_NOT_FOUND;
			}
		}

		// Notify clients
		aspa_table_notify_clients(aspa_table, &current->record, rtr_socket, current->type);
	}

	pthread_rwlock_unlock(&aspa_table->lock);
	return ASPA_SUCCESS;
}

static enum aspa_status aspa_table_update_in_place_undo_internal(struct aspa_table *aspa_table,
								 struct rtr_socket *rtr_socket,
								 struct aspa_update_operation *operations, size_t count,
								 struct aspa_update_operation *failed_operation)
{
	// Fail hard in debug builds.
	assert(aspa_table);
	assert(rtr_socket);
	assert(operations);
	assert(count > 0);

	pthread_rwlock_wrlock(&aspa_table->lock);
	struct aspa_store_node **node = aspa_store_get_node(&aspa_table->store, rtr_socket);

	if (!node || !*node || !(*node)->aspa_array) {
		// Node/array is gone -- nothing we can undo
		pthread_rwlock_unlock(&aspa_table->lock);
		return ASPA_ERROR;
	}

	assert(node);
	assert(*node);
	assert((*node)->aspa_array);

	struct aspa_array *array = (*node)->aspa_array;
	size_t existing_i = 0;

	for (size_t i = 0; i < array->size; i++) {
		struct aspa_update_operation *current = &operations[i];
		struct aspa_update_operation *next = (i < count - 1) ? &(operations[i + 1]) : NULL;

		// Check if this operation and the following were executed in the first place
		if (failed_operation && current == failed_operation)
			break;

		while (existing_i < array->size &&
		       aspa_array_get_record(array, existing_i)->customer_asn < current->record.customer_asn) {
			existing_i += 1;
		}

		struct aspa_record *existing_record = aspa_array_get_record(array, existing_i);

		// existing record and current op have matching CAS
		bool existing_matches_current = existing_record &&
						existing_record->customer_asn == current->record.customer_asn;

		// next record and current op have matching CAS
		bool next_matches_current = next && next->record.customer_asn == current->record.customer_asn;

		// MARK: Undo 'add' operation
		if (current->type == ASPA_ADD) {
			// Attempt to remove record with $CAS, but record with $CAS does not exist
			// Error: Removal of unknown record.
			if (!existing_matches_current) {
				pthread_rwlock_unlock(&aspa_table->lock);
				return ASPA_RECORD_NOT_FOUND;
			}

			// This operation adds a record with $CAS, the next op however removes this $CAS record again.
			if (next_matches_current && next->type == ASPA_REMOVE) {
#if ASPA_NOTIFY_NO_OPS
				// If it's a remove operation, we insert a reference to the removed record's providers.
				next->record = current->record;
				aspa_table_notify_clients(aspa_table, &next->record, rtr_socket, ASPA_ADD);
				aspa_table_notify_clients(aspa_table, &current->record, rtr_socket, ASPA_REMOVE);
#endif

				// Mark as no-op.
				current->is_no_op = true;
				next->is_no_op = true;

				// Skip next
				i += 1;
				continue;
			}

			// Remove record (release providers)
			if (aspa_array_remove(array, existing_i, true) != ASPA_SUCCESS) {
				pthread_rwlock_unlock(&aspa_table->lock);
				return ASPA_RECORD_NOT_FOUND;
			}

			aspa_table_notify_clients(aspa_table, &current->record, rtr_socket, ASPA_REMOVE);
		}

		// MARK: Undo 'remove' operation
		else if (current->type == ASPA_REMOVE) {
			// Next adds record with $CAS again.
			// Treat these two as a 'replace' op
			if (existing_matches_current & next_matches_current && next->type == ASPA_ADD) {
				if (existing_record->provider_asns)
					lrtr_free(existing_record->provider_asns);

				// If it's a remove operation, we inserted a reference to the existing
				// provider array. Put back reference.
				existing_record->provider_asns = current->record.provider_asns;
				existing_record->provider_count = current->record.provider_count;
				i += 1;

				aspa_table_notify_clients(aspa_table, &next->record, rtr_socket, ASPA_REMOVE);
			} else {
				// Attempt to add record with $CAS, but record with $CAS already exists
				// Error: Duplicate Add.
				if (existing_matches_current) {
					pthread_rwlock_unlock(&aspa_table->lock);
					return ASPA_DUPLICATE_RECORD;
				}

				// Insert record (don't copy providers)
				if (aspa_array_insert(array, existing_i, &current->record, false) != ASPA_SUCCESS) {
					pthread_rwlock_unlock(&aspa_table->lock);
					return ASPA_ERROR;
				}
			}

			aspa_table_notify_clients(aspa_table, &current->record, rtr_socket, ASPA_ADD);

			// If it's a remove operation, we inserted a reference to the existing
			// provider array. Restore that 'remove' operation back to its original state.
			current->record.provider_count = 0;
			current->record.provider_asns = NULL;
		}
	}

	pthread_rwlock_unlock(&aspa_table->lock);
	return ASPA_SUCCESS;
}

enum aspa_status aspa_table_update_in_place_undo(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket,
						 struct aspa_update_operation *operations, size_t count,
						 struct aspa_update_operation *failed_operation)
{
	// Fail hard in debug builds.
	assert(aspa_table);
	assert(rtr_socket);

	if (!aspa_table || !rtr_socket || ((count > 0) && !operations))
		return ASPA_ERROR;

	if (count == 0)
		return ASPA_SUCCESS;

	enum aspa_status res =
		aspa_table_update_in_place_undo_internal(aspa_table, rtr_socket, operations, count, failed_operation);

	for (size_t i = 0; i < count; i++) {
		struct aspa_update_operation *op = &operations[i];

		// If it's a remove operation, we inserted a reference to the removed record's providers.
		// Restore that 'remove' operation back to its original state.
		if (op->type == ASPA_REMOVE) {
			op->record.provider_asns = NULL;
			op->record.provider_count = 0;
		}
	}

	return res;
}

void aspa_table_update_in_place_cleanup(struct aspa_update_operation **operations, size_t count)
{
	if (!operations || !*operations)
		return;

	// If count == 0, this won't be executed
	for (size_t i = 0; i < count; i++) {
		struct aspa_update_operation *op = &(*operations)[i];

		// If it's a remove operation, we inserted a reference to the removed record's providers.
		// Release that provider array now and restore that 'remove'
		// operation back to its original state.
		if (!op->is_no_op && op->type == ASPA_REMOVE) {
			if (op->record.provider_asns)
				lrtr_free(op->record.provider_asns);

			op->record.provider_asns = NULL;
			op->record.provider_count = 0;
		}
	}

	lrtr_free(*operations);
	*operations = NULL;
}
