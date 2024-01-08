/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "../lib/alloc_utils_private.h"
#include "../rtrlib_export_private.h"
#include "aspa_array/aspa_array.h"
#include "aspa_private.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

static enum aspa_status aspa_table_notify_clients(struct aspa_table *aspa_table, struct aspa_record *record,
						  const struct rtr_socket *rtr_socket, const bool added)
{
	if (aspa_table->update_fp && record) {
		// Realloc in order not to expose internal record
		struct aspa_record rec = *record;
		size_t size = sizeof(uint32_t) * record->provider_count;
		rec.provider_asns = lrtr_malloc(size);
		memcpy(rec.provider_asns, record->provider_asns, size);
		aspa_table->update_fp(aspa_table, rec, rtr_socket, added);
	}

	return ASPA_SUCCESS;
}

static enum aspa_status aspa_store_insert_node(struct aspa_store_node **store, struct rtr_socket *rtr_socket,
					       struct aspa_array *aspa_array, struct aspa_store_node ***new_node)
{
	// Allocate new node
	struct aspa_store_node *new = lrtr_malloc(sizeof(struct aspa_store_node));

	if (new == NULL)
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
	if (node == NULL || *node == NULL || rtr_socket == NULL)
		return NULL;

	//	struct aspa_store_node *node = *head;

	while (*node != NULL) {
		if ((*node)->rtr_socket == rtr_socket) {
			return node;
		}
		node = &(*node)->next;
	}

	return NULL;
}

RTRLIB_EXPORT void aspa_table_init(struct aspa_table *aspa_table, aspa_update_fp update_fp)
{
	aspa_table->update_fp = update_fp;
	aspa_table->store = NULL;
	pthread_rwlock_init(&(aspa_table->lock), NULL);
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

	if (!array)
		// Doesn't exist anymore
		return ASPA_SUCCESS;

	// Notify clients about these records being removed
	if (notify) {
		for (size_t i = 0; i < array->size; i++) {
			aspa_table_notify_clients(aspa_table, aspa_array_get_record(array, i), socket, false);
		}
	}

	// Remove node for socket
	aspa_store_remove_node(node);

	// Release all records and their provider sets
	aspa_array_free(array, true);

	return ASPA_SUCCESS;
}

RTRLIB_EXPORT enum aspa_status aspa_table_src_remove(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket,
						     bool notify)
{
	pthread_rwlock_wrlock(&aspa_table->lock);

	struct aspa_store_node **node = aspa_store_get_node(&aspa_table->store, rtr_socket);

	if (!node || !*node) {
		// Already gone
		pthread_rwlock_unlock(&(aspa_table->lock));
		return ASPA_SUCCESS;
	}

	return aspa_table_remove_node(aspa_table, node, notify);
}

RTRLIB_EXPORT void aspa_table_free(struct aspa_table *aspa_table, bool notify)
{
	// To destroy the lock, first acquire the lock
	pthread_rwlock_wrlock(&aspa_table->lock);

	// Free store
	while (aspa_table->store != NULL) {
		aspa_table_remove_node(aspa_table, &aspa_table->store, notify);
	}

	aspa_table->store = NULL;

	pthread_rwlock_unlock(&aspa_table->lock);
	pthread_rwlock_destroy(&aspa_table->lock);
}

// MARK: - ASPA table update functions

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

#define UNUSED_PROVIDER_SETS_INCREASE_SIZE 10

static enum aspa_status append_to_set_array(uint32_t ***sets, size_t *size, size_t *capacity, uint32_t *new_set)
{
	if (!new_set)
		return ASPA_SUCCESS;

	if (*size >= *capacity) {
		uint32_t **tmp = lrtr_realloc(*sets, sizeof(struct uint32_t *) *
							     (*capacity + UNUSED_PROVIDER_SETS_INCREASE_SIZE));
		if (!tmp) {
			return ASPA_ERROR;
		}
		*capacity += UNUSED_PROVIDER_SETS_INCREASE_SIZE;
		*sets = tmp;
	}

	*sets[*size] = new_set;
	*size += 1;
	return ASPA_SUCCESS;
}

inline static enum aspa_operation_type invert(enum aspa_operation_type type)
{
	switch (type) {
	case ASPA_ADD:
		return ASPA_REMOVE;
	case ASPA_REMOVE:
		return ASPA_ADD;
	}
	return type;
}

static enum aspa_status aspa_table_update_internal(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket,
#ifdef ASPA_UPDATE_IN_PLACE
						   struct aspa_array *array, struct aspa_update_operation *operations,
						   size_t len, bool revert,
#else
						   struct aspa_array *array, struct aspa_array *new_array,
						   struct aspa_update_operation *operations, size_t len,
#endif
						   struct aspa_update_operation **failed_operation,
						   uint32_t ***unused_provider_sets, size_t *unused_size)
{
#ifdef ASPA_UPDATE_IN_PLACE
	if (!rtr_socket || !operations || len == 0 || !failed_operation)
		return ASPA_ERROR;

	if (!revert && (!unused_provider_sets || !unused_size))
		return ASPA_ERROR;
#else
	if (!rtr_socket || !operations || len == 0 || !failed_operation || !unused_provider_sets || !unused_size)
		return ASPA_ERROR;
#endif

	size_t unused_capacity = 0;
	size_t existing_i = 0;

	for (size_t i = 0; i < len; i++) {
		struct aspa_update_operation *current = &operations[i];

#ifdef ASPA_UPDATE_IN_PLACE
		if (revert && current->index == (*failed_operation)->index)
			break;
#endif

		struct aspa_update_operation *next = (i < len - 1) ? &operations[i + 1] : NULL;

#ifndef ASPA_UPDATE_IN_PLACE
		if (array) {
#endif
			while (existing_i < array->size &&
			       aspa_array_get_record(array, existing_i)->customer_asn < current->record.customer_asn) {
				// Skip over records untouched by these add/remove operations
				existing_i++;
			}
#ifndef ASPA_UPDATE_IN_PLACE
		}
#endif

		struct aspa_record *existing_record = aspa_array_get_record(array, existing_i);
#ifdef ASPA_UPDATE_IN_PLACE
		enum aspa_operation_type op_type = revert ? invert(current->type) : current->type;
#else
		enum aspa_operation_type op_type = current->type;
#endif

		// Sort providers
		if (current->record.provider_count > 0 && current->record.provider_asns)
			qsort(current->record.provider_asns, current->record.provider_count, sizeof(uint32_t),
			      compare_asns);

		// $CAS is not stored
		// Error: Duplicate Remove.
		if (op_type == ASPA_ADD && existing_record &&
		    current->record.customer_asn == existing_record->customer_asn) {
			*failed_operation = current;
			return ASPA_DUPLICATE_RECORD;
		}

		// $CAS has already been added
		// Error: Duplicate Add.
		if (op_type == ASPA_REMOVE &&
		    (!existing_record || current->record.customer_asn != existing_record->customer_asn)) {
			*failed_operation = current;
			return ASPA_RECORD_NOT_FOUND;
		}

		// Operations contains (ADD $CAS [..], REMOVE $CAS) and $CAS is not stored
		// No-op, skip next.
		if (next && current->type == ASPA_ADD && next->type == ASPA_REMOVE &&
		    next->record.customer_asn == current->record.customer_asn &&
		    (!existing_record || current->record.customer_asn != existing_record->customer_asn)) {
			// Remember this provider set is now unused

			if (
#ifdef ASPA_UPDATE_IN_PLACE
				!revert &&
#endif
				append_to_set_array(unused_provider_sets, unused_size, &unused_capacity,
						    current->record.provider_asns) != ASPA_SUCCESS) {
				*failed_operation = current;
				return ASPA_ERROR;
			}
			i += 1;
			continue;
		}

		// MARK: Adding a record
		if (op_type == ASPA_ADD) {
#ifdef ASPA_UPDATE_IN_PLACE
			if (aspa_array_insert(array, existing_i, &current->record) != ASPA_SUCCESS) {
#else
			if (aspa_array_append(new_array, &current->record) != ASPA_SUCCESS) {
#endif
				*failed_operation = current;
				return ASPA_ERROR;
			}
#ifdef ASPA_UPDATE_IN_PLACE
			existing_i += 1;
#endif
			aspa_table_notify_clients(aspa_table, &current->record, rtr_socket, true);
		}
		// MARK: Removing a record
		else if (op_type == ASPA_REMOVE) {
#ifdef ASPA_UPDATE_IN_PLACE
			if (!revert) {
#endif
				// Remember this provider set is now unused
				if (append_to_set_array(unused_provider_sets, unused_size, &unused_capacity,
							existing_record->provider_asns) != ASPA_SUCCESS) {
					*failed_operation = current;
					return ASPA_ERROR;
				}
#ifdef ASPA_UPDATE_IN_PLACE
			}
#endif

			aspa_table_notify_clients(aspa_table, existing_record, rtr_socket, false);

#ifdef ASPA_UPDATE_IN_PLACE
			if (aspa_array_remove(array, existing_i) != ASPA_SUCCESS) {
				*failed_operation = current;
				return ASPA_ERROR;
			}

			if (!revert) {
				// Replace record in operation so it could be undone later.
				current->record = *existing_record;
			}
#endif
		}
	}

	return ASPA_SUCCESS;
}

enum aspa_status aspa_table_update(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket,
				   struct aspa_update_operation *operations, size_t len,
#ifdef ASPA_UPDATE_IN_PLACE
				   bool revert,
#endif
				   struct aspa_update_operation **failed_operation,
				   struct aspa_update_finalization_args **finalization_args)
{
	if (!rtr_socket || !operations || len == 0 || !failed_operation)
		return ASPA_ERROR;

	if (!revert) {
		// stable sort operations, so operations dealing with the same customer ASN
		// are located right next to each other
		qsort(operations, len, sizeof(struct aspa_update_operation), compare_update_operations);
	}

	pthread_rwlock_wrlock(&aspa_table->lock);

	struct aspa_store_node **node = aspa_store_get_node(&aspa_table->store, rtr_socket);
	struct aspa_array *array = NULL;

	if (!node || !*node) {
		if (aspa_array_create(&array) != ASPA_SUCCESS || !array ||
		    aspa_store_insert_node(&aspa_table->store, rtr_socket, array, &node) != ASPA_SUCCESS) {
			pthread_rwlock_unlock(&aspa_table->lock);
			return ASPA_ERROR;
		}
	} else {
		array = (*node)->aspa_array;
		if (!array) {
			pthread_rwlock_unlock(&aspa_table->lock);
			return ASPA_ERROR;
		}
	}

	uint32_t **unused_sets = NULL;
	size_t unused_size = 0;

#ifdef ASPA_UPDATE_IN_PLACE
	enum aspa_status res = aspa_table_update_internal(aspa_table, rtr_socket, array, operations, len, revert,
							  failed_operation, &unused_sets, &unused_size);

	pthread_rwlock_unlock(&aspa_table->lock);
#else
	pthread_rwlock_unlock(&aspa_table->lock);

	struct aspa_array *new_array = NULL;
	if (aspa_array_create(&new_array) != ASPA_SUCCESS) {
		return ASPA_ERROR;
	}

	enum aspa_status res = aspa_table_update_internal(aspa_table, rtr_socket, array, new_array, operations, len,
							  failed_operation, &unused_sets, &unused_size);
#endif

	if (finalization_args && !*finalization_args) {
		*finalization_args = lrtr_malloc(sizeof(struct aspa_update_finalization_args));
	}

	if (finalization_args && *finalization_args) {
#ifndef ASPA_UPDATE_IN_PLACE
		if (res == ASPA_SUCCESS) {
			(*finalization_args)->old_array = &(*node)->aspa_array;
			(*finalization_args)->new_array = new_array;
		} else {
#else
		if (res != ASPA_SUCCESS) {
#endif
			// In case the update failed, clean up provider sets in indivudual operations
			if (unused_sets)
				lrtr_free(unused_sets);

			unused_sets = lrtr_malloc(sizeof(uint32_t *) * len);
			unused_size = 0;

			for (size_t i = 0; i < len; i++) {
				if (operations[i].type == ASPA_ADD) {
					unused_sets[unused_size] = operations[i].record.provider_asns;
					unused_size += 1;
				}
			}
		}

		if (unused_sets && unused_size > 0) {
			(*finalization_args)->unused_provider_arrays = unused_sets;
			(*finalization_args)->unused_provider_array_len = unused_size;
		} else {
			(*finalization_args)->unused_provider_arrays = NULL;
			(*finalization_args)->unused_provider_array_len = 0;
		}
	}

	return res;
}

enum aspa_status aspa_table_src_replace(struct aspa_table *dst, struct aspa_table *src, struct rtr_socket *rtr_socket,
					bool notify_dst, bool notify_src)
{
	if (dst == NULL || src == NULL || rtr_socket == NULL)
		return ASPA_ERROR;

	pthread_rwlock_wrlock(&dst->lock);
	pthread_rwlock_wrlock(&src->lock);

	struct aspa_store_node **src_node = aspa_store_get_node(&src->store, rtr_socket);

	if (!src_node || !*src_node || !(*src_node)->aspa_array) {
		pthread_rwlock_unlock(&src->lock);
		pthread_rwlock_unlock(&dst->lock);
		return ASPA_ERROR;
	}

	struct aspa_array *new_array = (*src_node)->aspa_array;

	struct aspa_store_node **old_node = aspa_store_get_node(&dst->store, rtr_socket);
	struct aspa_array *old_array = NULL;

	if (!old_node) {
		aspa_store_insert_node(&dst->store, rtr_socket, new_array, NULL);
	} else if (!*old_node) {
		pthread_rwlock_unlock(&src->lock);
		pthread_rwlock_unlock(&dst->lock);
		return ASPA_ERROR;
	} else {
		old_array = (*old_node)->aspa_array;
		(*old_node)->aspa_array = new_array;
	}

	// Remove socket from source table's store
	aspa_store_remove_node(src_node);

	pthread_rwlock_unlock(&src->lock);
	pthread_rwlock_unlock(&dst->lock);

	if (notify_src)
		// Notify src clients their records are being removed
		for (size_t i = 0; i < new_array->size; i++)
			aspa_table_notify_clients(src, aspa_array_get_record(new_array, i), rtr_socket, false);

	if (old_array) {
		if (notify_dst)
			// Notify dst clients of their existing records are being removed
			for (size_t i = 0; i < old_array->size; i++)
				aspa_table_notify_clients(dst, aspa_array_get_record(old_array, i), rtr_socket, false);

		// Free the old array and their provider sets
		aspa_array_free(old_array, true);
	}

	if (notify_dst)
		// Notify dst clients the records from src are added
		for (size_t i = 0; i < new_array->size; i++)
			aspa_table_notify_clients(src, aspa_array_get_record(new_array, i), rtr_socket, true);

	return ASPA_SUCCESS;
}

#ifdef ASPA_UPDATE_IN_PLACE
void aspa_update_finalize(struct aspa_update_finalization_args *finalization_args)
#else
void aspa_update_finalize(struct aspa_update_finalization_args *finalization_args, bool apply_update)
#endif
{
	if (!finalization_args)
		return;

#ifndef ASPA_UPDATE_IN_PLACE
	if (apply_update && finalization_args->old_array && finalization_args->new_array) {
		// Free the old array, but keep provider sets (released below)
		if (*finalization_args->old_array)
			aspa_array_free(*finalization_args->old_array, false);
		*finalization_args->old_array = finalization_args->new_array;
	}
#endif

	for (size_t i = 0; i < finalization_args->unused_provider_array_len; i++) {
		if (finalization_args->unused_provider_arrays[i])
			lrtr_free(finalization_args->unused_provider_arrays[i]);
	}

	if (finalization_args->unused_provider_arrays) {
		lrtr_free(finalization_args->unused_provider_arrays);
		finalization_args->unused_provider_arrays = NULL;
	}

	lrtr_free(finalization_args);
}
