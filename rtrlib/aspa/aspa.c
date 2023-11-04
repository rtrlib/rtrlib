/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "aspa_array/aspa_array.h"
#include "aspa_private.h"
#include "aspa_store/aspa_store.h"

#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/rtrlib_export_private.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

RTRLIB_EXPORT size_t aspa_size_of_aspa_record(const struct aspa_record *record)
{
	return sizeof(struct aspa_record) + sizeof(*record->provider_asns) * record->provider_count;
}

static void aspa_table_notify_clients(struct aspa_table *aspa_table, const struct aspa_record *record,
				      const struct rtr_socket *rtr_socket, const bool added)
{
	if (aspa_table->update_fp) {
		// Realloc in order not to expose internal record
		size_t record_size = aspa_size_of_aspa_record(record);
		struct aspa_record *copy = lrtr_malloc(record_size);
		memcpy(copy, record, record_size);
		aspa_table->update_fp(aspa_table, *copy, rtr_socket, added);
	}
}

RTRLIB_EXPORT void aspa_table_init(struct aspa_table *aspa_table, aspa_update_fp update_fp)
{
	aspa_table->update_fp = update_fp;
	aspa_table->store = NULL;
	pthread_rwlock_init(&(aspa_table->lock), NULL);
}

RTRLIB_EXPORT void aspa_table_free(struct aspa_table *aspa_table, bool notify)
{
	// To destroy the lock, first acquire the lock
	pthread_rwlock_wrlock(&aspa_table->lock);

	struct aspa_store_node *node;

	// Free store
	while (aspa_table->store != NULL) {
		node = aspa_table->store;

		if (notify) {
			// Notify clients about these records being removed
			for (size_t i = 0; i < node->aspa_array->size; i++)
				aspa_table_notify_clients(aspa_table, &(node->aspa_array->data[i]), node->rtr_socket,
							  false);
		}

		aspa_table->store = aspa_table->store->next;
		lrtr_free(node);
	}

	aspa_table->store = NULL;

	pthread_rwlock_unlock(&aspa_table->lock);
	pthread_rwlock_destroy(&aspa_table->lock);
}

RTRLIB_EXPORT int aspa_table_add(struct aspa_table *aspa_table, struct aspa_record *record,
				 struct rtr_socket *rtr_socket, bool replace)
{
	if (!aspa_table)
		return ASPA_ERROR;

	pthread_rwlock_wrlock(&aspa_table->lock);

	struct aspa_array *array;

	// Find the socket's corresponding aspa_array.
	// If fast lookup suceeds (rtr_socket->aspa_table == aspa_table),
	// access rtr_socket->aspa_array directly,
	// perform lookup on aspa_table->store insted.

	// Use fast lookup
	if (rtr_socket->aspa_table == aspa_table) {
		// Check if an ASPA array exists for this socket
		if (rtr_socket->aspa_array == NULL) {
			// Create a new ASPA array, store that array algonside with the socket in the table
			if (aspa_array_create(&rtr_socket->aspa_array) < 0 ||
			    aspa_store_insert(&aspa_table->store, rtr_socket, rtr_socket->aspa_array) != ASPA_SUCCESS) {
				pthread_rwlock_unlock(&aspa_table->lock);
				return ASPA_ERROR;
			}
		}

		array = rtr_socket->aspa_array;
	} else {
		// This isn't the primary table (not the table the socket holds a reference to)
		// Find node matching the given socket
		array = *aspa_store_search(&aspa_table->store, rtr_socket);

		if (array == NULL) {
			// Create a new ASPA array, store that array algonside with the socket in the table
			if (aspa_array_create(&array) < 0 ||
			    aspa_store_insert(&aspa_table->store, rtr_socket, array) != ASPA_SUCCESS) {
				pthread_rwlock_unlock(&aspa_table->lock);
				return ASPA_ERROR;
			}
		}
	}

	// Insert record aspa_array
	// TODO: This function does not handle duplicates/replacing the record
	if (aspa_array_insert(array, *record) < 0) {
		pthread_rwlock_unlock(&aspa_table->lock);
		return ASPA_ERROR;
	}

	pthread_rwlock_unlock(&aspa_table->lock);

	// Notify clients that the record has been added
	aspa_table_notify_clients(aspa_table, record, rtr_socket, true);

	return ASPA_SUCCESS;
}

RTRLIB_EXPORT int aspa_table_remove(struct aspa_table *aspa_table, struct aspa_record *record,
				    struct rtr_socket *rtr_socket)
{
	if (!aspa_table)
		return ASPA_ERROR;

	pthread_rwlock_wrlock(&aspa_table->lock);

	struct aspa_array *array;

	// Find the socket's corresponding aspa_array.
	// If fast lookup suceeds (rtr_socket->aspa_table == aspa_table),
	// access rtr_socket->aspa_array directly,
	// perform lookup on aspa_table->store insted.

	// Use fast lookup
	if (rtr_socket->aspa_table == aspa_table) {
		// Check if an ASPA array exists for this socket
		if (rtr_socket->aspa_array == NULL) {
			pthread_rwlock_unlock(&aspa_table->lock);
			return ASPA_ERROR;
		}

		array = rtr_socket->aspa_array;
	} else {
		// This isn't the primary table (not the table the socket holds a reference to)
		// Find node matching the given socket
		array = *aspa_store_search(&aspa_table->store, rtr_socket);

		if (array == NULL) {
			pthread_rwlock_unlock(&aspa_table->lock);
			return ASPA_ERROR;
		}
	}

	size_t i = aspa_array_search(array, record->customer_asn);

	if (i < 0) {
		pthread_rwlock_unlock(&aspa_table->lock);
		return ASPA_RECORD_NOT_FOUND;
	}

	// Remove record aspa_array
	if (aspa_array_free_at(array, i) < 0) {
		pthread_rwlock_unlock(&aspa_table->lock);
		return ASPA_ERROR;
	}

	pthread_rwlock_unlock(&aspa_table->lock);

	// Notify clients that the record has been removed
	aspa_table_notify_clients(aspa_table, record, rtr_socket, false);

	return ASPA_SUCCESS;
}

RTRLIB_EXPORT int aspa_table_src_remove(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket)
{
	pthread_rwlock_rdlock(&aspa_table->lock);

	struct aspa_array *array = *aspa_store_search(&aspa_table->store, rtr_socket);

	// Try to find array with fast lookup
	if (array == NULL && rtr_socket->aspa_table == aspa_table) {
		array = rtr_socket->aspa_array;
		rtr_socket->aspa_array = NULL;
	}

	if (array == NULL) {
		// Doesn't exist anymore
		pthread_rwlock_unlock(&(aspa_table->lock));
		return ASPA_SUCCESS;
	}

	// Notify clients about these records being removed
	for (size_t i = 0; i < array->size; i++)
		aspa_table_notify_clients(aspa_table, &(array->data[i]), rtr_socket, false);

	// Remove node for socket
	aspa_store_remove(&aspa_table->store, rtr_socket);

	// Release all records associated with the socket
	if (aspa_array_free(array) < 0) {
		pthread_rwlock_unlock(&(aspa_table->lock));
		return ASPA_ERROR;
	}

	pthread_rwlock_unlock(&(aspa_table->lock));
	return ASPA_SUCCESS;
}

int aspa_table_src_move(struct aspa_table *dst, struct aspa_table *src, struct rtr_socket *rtr_socket, bool notify_dst,
			bool notify_src)
{
	if (dst == NULL || src == NULL || rtr_socket == NULL)
		return ASPA_ERROR;

	pthread_rwlock_wrlock(&dst->lock);
	pthread_rwlock_wrlock(&src->lock);

	struct aspa_array *new_array = *aspa_store_search(&src->store, rtr_socket);
	struct aspa_array **old_array_ptr = aspa_store_search(&dst->store, rtr_socket);
	struct aspa_array *old_array = *old_array_ptr;

	if (new_array == NULL) {
		pthread_rwlock_unlock(&src->lock);
		pthread_rwlock_unlock(&dst->lock);
		return ASPA_ERROR;
	}

	int res = ASPA_SUCCESS;

	if (old_array == NULL) {
		// If destination table has no aspa_array associated to the given socket
		res = aspa_store_insert(&dst->store, rtr_socket, new_array);
	} else {
		// Destination table has an aspa_array associated to the given socket
		// Replace ref with new array
		*old_array_ptr = new_array;
	}

	// Remove socket from source table's store
	aspa_store_remove(&src->store, rtr_socket);

	// We may need to replace the aspa_array reference in rtr_socket too
	if (rtr_socket->aspa_table == src) {
		// Socket is associated with source table, remove reference since
		// the array is being moved to the destination table
		rtr_socket->aspa_array = NULL;
	} else if (rtr_socket->aspa_table == dst) {
		// Socket is associated with the destination table, replace
		// with new one
		rtr_socket->aspa_array = new_array;
	}

	pthread_rwlock_unlock(&src->lock);
	pthread_rwlock_unlock(&dst->lock);

	if (notify_src)
		// Notify src clients their records are being removed
		for (size_t i = 0; i < new_array->size; i++)
			aspa_table_notify_clients(src, &(new_array->data[i]), rtr_socket, false);

	if (old_array != NULL) {
		if (notify_dst)
			// Notify dst clients of their existing records are being removed
			for (size_t i = 0; i < old_array->size; i++)
				aspa_table_notify_clients(dst, &(old_array->data[i]), rtr_socket, false);

		// Free the old array
		aspa_array_free(old_array);
	}

	if (notify_dst)
		// Notify dst clients the records from src are added
		for (size_t i = 0; i < new_array->size; i++)
			aspa_table_notify_clients(src, &(new_array->data[i]), rtr_socket, true);

	return res;
}

static int as_path_hop(struct aspa_table *aspa_table, uint32_t customer_asn, uint32_t provider_asn)
{
	pthread_rwlock_rdlock(&aspa_table->lock);

	struct aspa_store_node *node = aspa_table->store;

	bool customer_found = 0;

	while (node != NULL) {
		struct aspa_array *aspa_array = node->aspa_array;

		int pos = aspa_array_search(aspa_array, customer_asn);

		if (pos == -1)
			goto cont;

		customer_found = 1;

		for (size_t i = 0; i < aspa_array->data[pos].provider_count; i++) {
			if (aspa_array->data[pos].provider_asns[i] == provider_asn) {
				pthread_rwlock_unlock(&aspa_table->lock);
				return AS_PROVIDER;
			}
		}

		cont:
			node = node->next;
	}

	pthread_rwlock_unlock(&aspa_table->lock);
	return customer_found ? AS_NOT_PROVIDER : AS_NO_ATTESTATION;
}

RTRLIB_EXPORT int as_path_verify_upstream(struct aspa_table *aspa_table, uint32_t *as_path, size_t as_path_length)
{
	if (as_path_length < 1)
		return AS_PATH_INVALID;
	if (as_path_length == 1)
		return AS_PATH_VALID;

	for (size_t i = 1; i < as_path_length; i++) {
		if (as_path_hop(aspa_table, as_path[i-1], as_path[i]) == AS_NOT_PROVIDER)
			return AS_PATH_INVALID;
	}

	for (size_t i = 1; i < as_path_length; i++) {
		if (as_path_hop(aspa_table, as_path[i-1], as_path[i]) == AS_NO_ATTESTATION)
			return AS_PATH_UNKNOWN;
	}

	return AS_PATH_VALID;
}

RTRLIB_EXPORT int as_path_verify_downstream(struct aspa_table *aspa_table, uint32_t *as_path, size_t as_path_length)
{
	if (as_path_length < 1)
		return AS_PATH_INVALID;
	if (as_path_length <= 2)
		return AS_PATH_VALID;

	size_t u_min = as_path_length+1;
	for (size_t u = 2; u <= as_path_length; u++) {
		if (as_path_hop(aspa_table, as_path[(u-1)-1], as_path[(u-1)]) == AS_NOT_PROVIDER) {
			u_min = u;
			break;
		}
	}

	size_t v_max = 0;
	for (size_t v = as_path_length-1; v >= 1; v--) {
		if (as_path_hop(aspa_table, as_path[(v-1)+1], as_path[(v-1)]) == AS_NOT_PROVIDER) {
			v_max = v;
			break;
		}
	}

	if (u_min < v_max)
		return AS_PATH_INVALID;


	size_t K = 0;
	for (size_t i = 1; i < as_path_length; i++) {
		if (as_path_hop(aspa_table, as_path[i-1], as_path[i]) == AS_PROVIDER)
			K++;
		else
			break;
	}

	size_t L = as_path_length-1;
	for (size_t j = as_path_length-2; j >= 0; j--) {
		if (as_path_hop(aspa_table, as_path[j+1], as_path[j]) == AS_PROVIDER)
			L--;
		else
			break;
	}

	if (L-K <= 1)
		return AS_PATH_VALID;
	return AS_PATH_UNKNOWN;
}
