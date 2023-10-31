/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "aspa_private.h"
#include "ordered_dyn_array/ordered_dyn_array.h"
#include "rtrlib/rtrlib_export_private.h"
#include "rtrlib/lib/alloc_utils_private.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

static void aspa_table_notify_clients(struct aspa_table *aspa_table, const struct aspa_record *record, const struct rtr_socket *rtr_socket, const bool added)
{
	if (aspa_table->update_fp) {
		// Realloc in order not to expose internal record
		struct aspa_record *rec = lrtr_realloc((void *)record, sizeof(record) + record->provider_count * sizeof(record->provider_asns[0]));
		aspa_table->update_fp(aspa_table, *rec, rtr_socket, added);
	}
}

RTRLIB_EXPORT void aspa_table_init(struct aspa_table *aspa_table, aspa_update_fp update_fp)
{
	aspa_table->update_fp = update_fp;
	pthread_rwlock_init(&(aspa_table->lock), NULL);
}

int aspa_table_cache_init(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket)
{
	// ASPA table with cache does not need update_fp
	aspa_table->update_fp = NULL;
	
	// Create an aspa_cache
	// Records associated to the given rtr_socket won't be added or removed from the
	// socket's ordered_dyn_array, but from the cache instead.
	struct aspa_cache *cache = lrtr_malloc(sizeof(struct aspa_cache));
	
	// Save a reference to the socket, since the aspa cache table won't allow
	// adding or removing records associated to other sockets!
	cache->rtr_socket = rtr_socket;
	
	// copy the socket's existing ordered_dyn_array into the cache
	if (ordered_dyn_array_copy(&(cache->cached_array), rtr_socket->aspa_array) < 0)
		return ASPA_ERROR;
	
	aspa_table->cache = cache;
	
	// Init the lock
	pthread_rwlock_init(&(aspa_table->lock), NULL);
	
	return ASPA_SUCCESS;
}

int aspa_table_cache_writeback(struct aspa_table *cache_table, struct aspa_table *dest_table)
{
	pthread_rwlock_wrlock(&(cache_table->lock));
	pthread_rwlock_wrlock(&(dest_table->lock));
	
	if (!cache_table->cache) {
		pthread_rwlock_unlock(&(cache_table->lock));
		pthread_rwlock_unlock(&(dest_table->lock));
		return ASPA_ERROR;
	}
	
	// The src table is expected to be a cache table, that is, it has a cache associated with it.
	// If the src table is not a cache table, fail immediately.
	// Free the sockets aspa_array and replace it with the cached array.
	struct rtr_socket *rtr_socket = cache_table->cache->rtr_socket;
	struct ordered_dyn_array *old_array = rtr_socket->aspa_array;
	struct ordered_dyn_array *new_array = cache_table->cache->cached_array;
	
	if (old_array && old_array->size > 0)
		// The old ordered_dyn_array has at leat one record, so notify clients these records
		// will be removed
		for (size_t i = 0; i < old_array->size; i++)
			aspa_table_notify_clients(dest_table, &(old_array->data[i]), rtr_socket, true);
	
	// The old ordered_dyn_array is not needed anymore, release it.
	if (old_array)
		if (ordered_dyn_array_free(old_array) < 0) {
			pthread_rwlock_unlock(&(cache_table->lock));
			pthread_rwlock_unlock(&(dest_table->lock));
			return ASPA_ERROR;
		}
		
	// replace the socket's ordered_dyn_array with the new one from the cache
	rtr_socket->aspa_array = new_array;
		
	// Free the cache
	lrtr_free(cache_table->cache);
	
	if (new_array->size > 0)
		// Since the socket's ordered_dyn_array has been to be replaced with the new cached array,
		// notify clients these records have been added
		for (size_t i = 0; i < new_array->size; i++)
			aspa_table_notify_clients(dest_table, &(new_array->data[i]), rtr_socket, true);
	
	pthread_rwlock_unlock(&(cache_table->lock));
	pthread_rwlock_unlock(&(dest_table->lock));
	
	return ASPA_SUCCESS;
}


RTRLIB_EXPORT void aspa_table_free(struct aspa_table *aspa_table)
{
	// To destroy the lock, first acquire the lock
	pthread_rwlock_wrlock(&(aspa_table->lock));
	
	// Free cache only if it has been used, that is, non-NULL
	if (aspa_table->cache) {
		if (aspa_table->cache->cached_array)
			ordered_dyn_array_free(aspa_table->cache->cached_array);
		
		lrtr_free(aspa_table->cache);
	}
	pthread_rwlock_unlock(&(aspa_table->lock));
	pthread_rwlock_destroy(&(aspa_table->lock));
}

RTRLIB_EXPORT int aspa_table_add(struct aspa_table *aspa_table, struct aspa_record *record, struct rtr_socket *rtr_socket)
{
	pthread_rwlock_wrlock(&(aspa_table->lock));
	
	struct ordered_dyn_array *array;
	bool is_cache = false;
	
	if (aspa_table->cache) {
		is_cache = true;
		if (aspa_table->cache->rtr_socket != rtr_socket) {
			pthread_rwlock_unlock(&aspa_table->lock);
			return ASPA_ERROR;
		}
		
		// Use cached array
		array = aspa_table->cache->cached_array;
	} else {
		if (!rtr_socket->aspa_array)
			if (ordered_dyn_array_create(&(rtr_socket->aspa_array)) < 0) {
				pthread_rwlock_unlock(&aspa_table->lock);
				return ASPA_ERROR;
			}
		
		// Use the socket's array
		array = rtr_socket->aspa_array;
	}
		
	// Insert record into socket's ordered_dyn_array
	if (ordered_dyn_array_insert(array, *record) < 0) {
		pthread_rwlock_unlock(&aspa_table->lock);
		return ASPA_ERROR;
	}
	
	pthread_rwlock_unlock(&aspa_table->lock);
	
	// table may have changed, so capture if table is cache table
	if (!is_cache)
		// Notify clients that the record has been added
		aspa_table_notify_clients(aspa_table, record, rtr_socket, true);
	
	return ASPA_SUCCESS;
}

RTRLIB_EXPORT int aspa_table_remove(struct aspa_table *aspa_table, struct aspa_record *record, struct rtr_socket *rtr_socket)
{
	pthread_rwlock_wrlock(&(aspa_table->lock));
	
	struct ordered_dyn_array *array;
	bool is_cache = false;
	
	if (aspa_table->cache) {
		is_cache = true;
		// Other sockets are disallowed
		if (aspa_table->cache->rtr_socket != rtr_socket) {
			pthread_rwlock_unlock(&aspa_table->lock);
			return ASPA_ERROR;
		}
		
		// Use cached array
		array = aspa_table->cache->cached_array;
	} else {
		if (!rtr_socket->aspa_array) {
			pthread_rwlock_unlock(&aspa_table->lock);
			return ASPA_ERROR;
		}
		
		// Use the socket's array
		array = rtr_socket->aspa_array;
	}
		
	// TODO: Could deleting be optimited any further? like not searching by CAS ASN (?)
	size_t i = ordered_dyn_array_search(array, record->customer_asn);
	
	// Remove record from socket's ordered_dyn_array
	if (i < 0 || ordered_dyn_array_free_at(array, i) < 0) {
		return ASPA_ERROR;
	}
	
	pthread_rwlock_unlock(&aspa_table->lock);
	
	// table may have changed, so capture if table is cache table
	if (!is_cache)
		// Notify clients that the record has been removed
		aspa_table_notify_clients(aspa_table, record, rtr_socket, false);
	
	return ASPA_SUCCESS;
}

RTRLIB_EXPORT int aspa_table_src_remove(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket)
{
	pthread_rwlock_rdlock(&(aspa_table->lock));
	
	struct ordered_dyn_array *array;
	
	if (aspa_table->cache) {
		// Other sockets are disallowed
		if (aspa_table->cache->rtr_socket != rtr_socket) {
			pthread_rwlock_unlock(&aspa_table->lock);
			return ASPA_ERROR;
		}
		
		// Use cached array
		array = aspa_table->cache->cached_array;
	} else {
		if (!rtr_socket->aspa_array) {
			// no ordered_dynamic array associated
			pthread_rwlock_unlock(&aspa_table->lock);
			return ASPA_SUCCESS;
		}
		
		// Use the socket's array
		array = rtr_socket->aspa_array;
	}
	
	if (!aspa_table->cache)
		// Notify clients about these records being removed
		for (size_t i = 0; i < array->size; i++)
			aspa_table_notify_clients(aspa_table, &(array->data[i]), rtr_socket, false);
	
	// Release all records associated with the socket
	if (ordered_dyn_array_free(array) < 0) {
		pthread_rwlock_unlock(&(aspa_table->lock));
		return ASPA_ERROR;
	}
	
	if (!aspa_table->cache)
		rtr_socket->aspa_array = NULL;
		
	pthread_rwlock_unlock(&(aspa_table->lock));
	return ASPA_SUCCESS;
}

void aspa_table_for_each_record(struct aspa_table *aspa_table, aspa_for_each_fp fp, const struct rtr_socket **sockets, size_t sockets_len, void *data)
{
	pthread_rwlock_rdlock(&(aspa_table->lock));

	// TODO: iterate over records (@M: consider order of iterating -> sockets in parallel or sequential)
	
	pthread_rwlock_unlock(&(aspa_table->lock));
}
