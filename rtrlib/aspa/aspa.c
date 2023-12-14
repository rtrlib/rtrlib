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

#include "../lib/alloc_utils_private.h"
#include "../rtrlib_export_private.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

static enum aspa_rtvals aspa_table_notify_clients(struct aspa_table *aspa_table, const struct aspa_record *record,
				      const struct rtr_socket *rtr_socket, const bool added)
{
	if (aspa_table->update_fp) {
		// Realloc in order not to expose internal record
		struct aspa_record *copy = lrtr_malloc(sizeof(struct aspa_record));
		if (!copy)
			return ASPA_ERROR;
		
		copy->customer_asn = record->customer_asn;
		copy->provider_count = record->provider_count;
		
		size_t provider_size = sizeof(uint32_t) * record->provider_count;
		copy->provider_asns = lrtr_malloc(provider_size);
		memcpy(copy->provider_asns, record->provider_asns, provider_size);
		
		aspa_table->update_fp(aspa_table, *copy, rtr_socket, added);
	}
	
	return ASPA_SUCCESS;
}

RTRLIB_EXPORT void aspa_table_init(struct aspa_table *aspa_table, aspa_update_fp update_fp)
{
	aspa_table->update_fp = update_fp;
	aspa_table->store = NULL;
	pthread_rwlock_init(&(aspa_table->lock), NULL);
}

static enum aspa_rtvals aspa_table_remove_node(struct aspa_table *aspa_table, struct aspa_store_node **node, bool notify)
{
	if (!node || *node) {
		pthread_rwlock_unlock(&(aspa_table->lock));
		return ASPA_ERROR;
	}
	
	struct aspa_array *array = (*node)->aspa_array;
	struct rtr_socket *socket = (*node)->rtr_socket;

	// Try to find array with fast lookup
	if (socket->aspa_table == aspa_table) {
		socket->aspa_array = NULL;
	}

	if (array == NULL) {
		// Doesn't exist anymore
		return ASPA_SUCCESS;
	}

	// Notify clients about these records being removed
	for (size_t i = 0; i < array->size; i++) {
		if (notify)
			aspa_table_notify_clients(aspa_table, &(array->data[i]), socket, false);
		
		// Release provider ASN array
		lrtr_free(array->data[i].provider_asns);
	}

	// Remove node for socket
	// aspa_store_remove(&aspa_table->store, rtr_socket);
	*node = (*node)->next;

	// Release all records associated with the socket
	aspa_array_free(array);

	pthread_rwlock_unlock(&(aspa_table->lock));
	return ASPA_SUCCESS;
}

RTRLIB_EXPORT enum aspa_rtvals aspa_table_src_remove(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket, bool notify)
{
	pthread_rwlock_wrlock(&aspa_table->lock);

	struct aspa_store_node **node = aspa_store_search_node(&aspa_table->store, rtr_socket);
	
	if (!node || *node) {
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
/*
RTRLIB_EXPORT int aspa_table_add(struct aspa_table *aspa_table,
				struct aspa_record *record, struct rtr_socket *rtr_socket,
				bool overwrite)
{
	if (!aspa_table)
		return ASPA_ERROR;

	pthread_rwlock_wrlock(&aspa_table->lock);

	struct aspa_array *array;

	// sort to-be-added providers in ascending order
	qsort(record->provider_asns, record->provider_count,
			sizeof(uint32_t), cmp_uint32_t);

	// Find the socket's corresponding aspa_array.
	// If fast lookup suceeds (rtr_socket->aspa_table == aspa_table),
	// access rtr_socket->aspa_array directly,
	// perform lookup on aspa_table->store insted.

	// Use fast lookup
	if (rtr_socket->aspa_table == aspa_table) {
		// Check if an ASPA array exists for this socket
		if (rtr_socket->aspa_array == NULL) {
			// Create a new ASPA array, store that array alongside with the socket in the table
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
	if (aspa_array_insert(array, record) < 0) {
		pthread_rwlock_unlock(&aspa_table->lock);
		return ASPA_ERROR;
	}

	pthread_rwlock_unlock(&aspa_table->lock);

	// Notify clients that the record has been added
	aspa_table_notify_clients(aspa_table, record, rtr_socket, true);

	return ASPA_SUCCESS;
}

RTRLIB_EXPORT int aspa_table_remove(struct aspa_table *aspa_table,
					struct aspa_record *record,
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

	struct aspa_record *aspa_record = aspa_array_search(array, record->customer_asn);

	if (aspa_record == NULL) { // error occured
		pthread_rwlock_unlock(&aspa_table->lock);
		return ASPA_RECORD_NOT_FOUND;
	}

	// Remove record aspa_array
	if (aspa_array_free_entry(array, aspa_record) < 0) {
		pthread_rwlock_unlock(&aspa_table->lock);
		return ASPA_ERROR;
	}

	pthread_rwlock_unlock(&aspa_table->lock);

	// Notify clients that the record has been removed
	aspa_table_notify_clients(aspa_table, record, rtr_socket, false);

	return ASPA_SUCCESS;
}
 */

RTRLIB_EXPORT enum aspa_rtvals aspa_table_apply_update(struct aspa_update *update)
{
	if (!update->aspa_table || !update->rtr_socket || !update->aspa_array)
		return ASPA_ERROR;
	
	struct aspa_table *aspa_table = update->aspa_table;
	
	pthread_rwlock_wrlock(&aspa_table->lock);
	struct aspa_array **array_ptr = aspa_store_search_array(&aspa_table->store, update->rtr_socket);
	struct aspa_array *existing_array = NULL;
	
	if (array_ptr) {
		existing_array = *array_ptr;
		*array_ptr = update->aspa_array;
	} else {
		// store has no entry for given socket
		// If destination table has no aspa_array associated with the given socket
		if (aspa_store_insert(&aspa_table->store, update->rtr_socket, update->aspa_array) != ASPA_SUCCESS) {
			// malloc failed, nothing has been written into the ASPA table yet
			pthread_rwlock_wrlock(&aspa_table->lock);
			return ASPA_ERROR;
		}
	}
	
	// We may need to replace the aspa_array reference in rtr_socket, too
	if (update->rtr_socket->aspa_table == aspa_table) {
		// Replace reference stored in socket so fast lookup keeps working
		update->rtr_socket->aspa_array = update->aspa_array;
	}
	
	// prevent this ASPA array from being freed when the update gets released
	update->aspa_array = NULL;
	
	pthread_rwlock_unlock(&aspa_table->lock);
	
	// Free provider arrays which were part of the old aspa array but aren't included in the new array
	for (size_t i = 0; i < update->stale_provider_arrays_count; i++) {
		lrtr_free(update->stale_provider_arrays[i]);
	}
	
	if (existing_array) {
		// store has existing array for given socket
		// Notify clients the old records are being removed
		for (size_t i = 0; i < existing_array->size; i++) {
			aspa_table_notify_clients(aspa_table, &(existing_array->data[i]), update->rtr_socket, false);
		}
		
		// Free the old array
		aspa_array_free(existing_array);
		
		// Notify clients new records have been added
		for (size_t i = 0; i < update->aspa_array->size; i++)
			aspa_table_notify_clients(aspa_table, &( update->aspa_array->data[i]), update->rtr_socket, true);
	}
	
	return ASPA_SUCCESS;
}

RTRLIB_EXPORT void aspa_table_free_update(struct aspa_update *update)
{
    if (update->aspa_array)
        aspa_array_free(update->aspa_array);
    
    for (size_t i = 0; i < update->stale_provider_arrays_count; i++)
        lrtr_free(update->stale_provider_arrays[i]);
    
    lrtr_free(update->stale_provider_arrays);
    lrtr_free(update);
}

static int compare_update_operations(const void *a, const void *b) {
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

static struct aspa_array *aspa_table_get_array(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket)
{
    if (!aspa_table)
        return NULL;
    // Find the socket's corresponding aspa_array.
    // If fast lookup suceeds (rtr_socket->aspa_table == aspa_table),
    // access rtr_socket->aspa_array directly,
    // perform lookup on aspa_table->store insted.

    // Use fast lookup
    if (rtr_socket->aspa_table == aspa_table) {
        // Check if an ASPA array exists for this socket
        if (!rtr_socket->aspa_array) {
            rtr_socket->aspa_array = *aspa_store_search_array(&aspa_table->store, rtr_socket);
        }

        return rtr_socket->aspa_array;
    } else {
        // This isn't the primary table (not the table the socket holds a reference to)
        // Find node matching the given socket
         return *aspa_store_search_array(&aspa_table->store, rtr_socket);
    }
}

RTRLIB_EXPORT enum aspa_rtvals aspa_table_compute_update(struct aspa_table *aspa_table, struct aspa_update_operation *operations, size_t count, struct rtr_socket *rtr_socket, struct aspa_update *update, struct aspa_update_operation **failed_operation)
{
    if (!rtr_socket || !operations || !update || count <= 0)
        return ASPA_ERROR;
    
    // stable sort operations, so operations dealing with the same customer ASN
    // are located right next to each other
    qsort(operations, count, sizeof(struct aspa_update_operation), compare_update_operations);
    
    update->stale_provider_arrays = NULL;
    update->stale_provider_arrays_count = 0;
    
    if (aspa_array_create(&update->aspa_array) != ASPA_SUCCESS) {
        return ASPA_ERROR;
    }
    
    // retrieve existing ASPA array belonging to the table and matching the given socket
    struct aspa_array *existing_array = aspa_table_get_array(aspa_table, rtr_socket);
    
    if (existing_array) {
        // use existing data
        size_t existing_index = 0;
        size_t existing_size = existing_array->size;
        for (size_t i = 0; i < count; i++) {
            
			while (existing_index < existing_size && existing_array->data[existing_index].customer_asn < operations[i].record.customer_asn) {
                // append record, sorting has been done above.
				if (aspa_array_append(update->aspa_array, &existing_array->data[existing_index]) != ASPA_SUCCESS) {
					aspa_array_free(update->aspa_array);
					return ASPA_ERROR;
				}
				
				existing_size++;
            }
            
            switch (operations[i].type) {
                case ASPA_ADD:
                    if (existing_array->data[existing_index].customer_asn == operations[i].record.customer_asn) {
						*failed_operation = &operations[i];
						aspa_array_free(update->aspa_array);
                        return ASPA_DUPLICATE_RECORD;
                    }
                    
                    bool next_same_asn = i < count - 1 && operations[i + 1].record.customer_asn == operations[i].record.customer_asn;
                    if (next_same_asn) {
                        switch (operations[i + 1].type) {
                            case ASPA_ADD:
                                // adding a record for an ASN already added represents an error.
								*failed_operation = &operations[i + 1];
								aspa_array_free(update->aspa_array);
                                return ASPA_DUPLICATE_RECORD;
                            case ASPA_REMOVE:
                                // skip this add operation and the next remove operation because it is nullified by the following remove operation.
                                i += 1;
                                continue;
                        }
                    }
                    
                    // append record, sorting has been done above.
					if (aspa_array_append(update->aspa_array, &operations[i].record) != ASPA_SUCCESS) {
						*failed_operation = &operations[i];
						aspa_array_free(update->aspa_array);
						return ASPA_ERROR;
					}
                    break;
                    
                case ASPA_REMOVE:
					// removing a records which doesn't exist is considered an error
                    if (existing_array->data[existing_index].customer_asn != operations[i].record.customer_asn) {
						*failed_operation = &operations[i];
						aspa_array_free(update->aspa_array);
                        return ASPA_RECORD_NOT_FOUND;
                    }
					
					// In case the update gets applied, this array of provider ASNs needs to be released
					update->stale_provider_arrays[update->stale_provider_arrays_count] = existing_array->data[existing_index].provider_asns;
					update->stale_provider_arrays_count += 1;
                    break;
            }
        }
    } else {
        // no existing data to use, so just append records from add operations.
        for (size_t i = 0; i < count; i++) {
            
            if (operations[i].type == ASPA_ADD) {
                bool next_same_asn = i < count - 1 && operations[i + 1].record.customer_asn == operations[i].record.customer_asn;
                if (next_same_asn) {
                    switch (operations[i + 1].type) {
                        case ASPA_ADD:
                            // adding a record for an ASN already added represents an error.
                            return ASPA_DUPLICATE_RECORD;
                        case ASPA_REMOVE:
                            // skip this add operation and the next remove operation because it is nullified by the following remove operation.
                            i += 1;
                            continue;
                    }
                }
            }
            
            // trying to remove an unknown record is not allowed.
            if (operations[i].type == ASPA_REMOVE) {
                return ASPA_RECORD_NOT_FOUND;
            }
            
            // append record, sorting has been done above.
            if (aspa_array_append(update->aspa_array, &operations[i].record) != ASPA_SUCCESS)
                return ASPA_ERROR;
        }
    }
	
	return ASPA_SUCCESS;
}


/*
int aspa_table_src_move(struct aspa_table *dst, struct aspa_table *src, struct rtr_socket *rtr_socket, bool notify_dst,
			bool notify_src)
{
	if (dst == NULL || src == NULL || rtr_socket == NULL)
		return ASPA_ERROR;

	pthread_rwlock_wrlock(&dst->lock);
	pthread_rwlock_wrlock(&src->lock);

	struct aspa_array *src_array = *aspa_store_search(&src->store, rtr_socket);
	struct aspa_array **dst_array_ptr = aspa_store_search(&dst->store, rtr_socket);
	struct aspa_array *dst_array = *dst_array_ptr;

	if (src_array == NULL) {
		pthread_rwlock_unlock(&src->lock);
		pthread_rwlock_unlock(&dst->lock);
		return ASPA_ERROR;
	}

	int res = ASPA_SUCCESS;

	if (dst_array == NULL) {
		// If destination table has no aspa_array associated with the given socket
		res = aspa_store_insert(&dst->store, rtr_socket, src_array);
	} else {
		// Destination table has an aspa_array associated with the given socket
		// Replace ref with new array
		*dst_array_ptr = src_array;
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
		rtr_socket->aspa_array = src_array;
	}

	pthread_rwlock_unlock(&src->lock);
	pthread_rwlock_unlock(&dst->lock);

	if (notify_src)
		// Notify src clients their records are being removed
		for (size_t i = 0; i < src_array->size; i++)
			aspa_table_notify_clients(src, &(src_array->data[i]), rtr_socket, false);

	if (dst_array != NULL) {
		if (notify_dst)
			// Notify dst clients of their existing records are being removed
			for (size_t i = 0; i < dst_array->size; i++)
				aspa_table_notify_clients(dst, &(dst_array->data[i]), rtr_socket, false);

		// Free the old array
		aspa_array_free(dst_array);
	}

	if (notify_dst)
		// Notify dst clients the records from src are added
		for (size_t i = 0; i < src_array->size; i++)
			aspa_table_notify_clients(src, &(src_array->data[i]), rtr_socket, true);

	return res;
}
*/
static void *binary_search_asns(const uint32_t key, uint32_t *array, size_t n)
{
       size_t mid, top;
       int val;
       uint32_t *piv, *base = array;

       mid = top = n;

       while (mid) {
               mid = top / 2;

               piv = base + mid;

               val = key - *piv;


               if (val == 0) {
                       return piv;
               }
               if (val >= 0) {
                       base = piv;
               }
               top -= mid;
       }
       return NULL;
}

enum aspa_hop_result aspa_check_hop(struct aspa_table *aspa_table, uint32_t customer_asn, uint32_t provider_asn)
{
	bool customer_found = false;
	
	for (struct aspa_store_node *node = aspa_table->store; node != NULL;
				node = node->next) {

		struct aspa_record *aspa_record = aspa_array_search(node->aspa_array,
				customer_asn);

		if (!aspa_record)
			continue;
			
		customer_found = true;

		uint32_t* provider = binary_search_asns(provider_asn, aspa_record->provider_asns,
				aspa_record->provider_count);

		if (!provider) {
			return ASPA_PROVIDER_PLUS;
		}
	}

	return customer_found ? ASPA_NOT_PROVIDER_PLUS : ASPA_NO_ATTESTATION;
}

enum aspa_verification_result aspa_verify_upstream(struct aspa_table *aspa_table, uint32_t as_path[], size_t len)
{
	// Optimized AS_PATH verification algorithm using zero based array
	// where the origin AS has index N - 1 and the latest AS in the AS_PATH
	// has index 0.
	// Doesn't check any hop twice.
	if (len <= 1)
		return ASPA_AS_PATH_VALID;
    
    pthread_rwlock_rdlock(&aspa_table->lock);

	// Find apex of up-ramp
	size_t r = len - 1;
	enum aspa_hop_result last_hop_right;
	while (r > 0 &&
			(last_hop_right = aspa_check_hop(aspa_table, as_path[r], as_path[r - 1]))
				== ASPA_PROVIDER_PLUS)
		r -= 1;

    if (r == 0) {
        pthread_rwlock_unlock(&aspa_table->lock);
        return ASPA_AS_PATH_VALID;
    }

	bool found_nP_from_right = false;

	/*
	 * I. Look for nP+ in the right-to-left/upwards direction
	 * Check if there's a nP+ hop in the gap from the right (facing left/up).
	 * a, The next hop right after the up-ramp was already retrieved from the database,
	 *    so just check if that hop was nP+.
	 * b, Also, don't check last hop before down-ramp starts
	 *    because there must be a hop of space in order for two
	 *    nP+ hops to oppose each other.
	 * c, RR points to the left end of the hop last checked.
	 * d, Checking stops if the hop is nP+.
	 *
	 *        Last chance of finding a relevant nP+ hop
	 *           /
	 *   L      /\                  R
	 *   * -- * -- * . . . . . * -- *
	 *   0   L+1              R-1    \
	 *         |<------------------|  *
	 *                               N-1
	 *
	 */

	size_t rr = r - 1;
	if (last_hop_right == ASPA_NOT_PROVIDER_PLUS) {
		found_nP_from_right = true;
	} else {
		while (rr > 0) {
			size_t c = rr;
			rr--;
			if (aspa_check_hop(aspa_table, as_path[c], as_path[rr])
					== ASPA_NOT_PROVIDER_PLUS) {
				found_nP_from_right = true;
				break;
			}
		}
	}

	// If nP+ occurs upstream customer-provider chain, return INVALID.
    if (found_nP_from_right) {
        pthread_rwlock_unlock(&aspa_table->lock);
        return ASPA_AS_PATH_INVALID;
    }

    pthread_rwlock_unlock(&aspa_table->lock);
	return ASPA_AS_PATH_UNKNOWN;
}

enum aspa_verification_result aspa_verify_downstream(struct aspa_table *aspa_table, uint32_t as_path[], size_t len)
{
	// Optimized AS_PATH verification algorithm using zero based array
	// where the origin AS has index N - 1 and the latest AS in the AS_PATH
	// has index 0.
	// Doesn't check any hop twice.
	if (len <= 2)
		return ASPA_AS_PATH_VALID;
    
    pthread_rwlock_rdlock(&aspa_table->lock);

	// Find apex of up-ramp
	size_t r = len - 1;
	enum aspa_hop_result last_hop_right;
	while (r > 0 &&
			(last_hop_right = aspa_check_hop(aspa_table, as_path[r], as_path[r - 1]))
				== ASPA_PROVIDER_PLUS)
		r--;

	bool found_nP_from_right = false;
	bool found_nP_from_left = false;

	size_t l = 0;
	enum aspa_hop_result last_hop_left;

	// Find down-ramp end
	while (l < r &&
		   (last_hop_left = aspa_check_hop(aspa_table, as_path[l], as_path[l + 1]))
				== ASPA_PROVIDER_PLUS)
		l++;
	assert(l <= r);

	// If gap does not exist (sharp tip) or is just a single hop wide,
	// there's no way to create a route leak, return VALID.
    if (r - l <= 1) {
        pthread_rwlock_unlock(&aspa_table->lock);
        return ASPA_AS_PATH_VALID;
    }

	/*
	 * I. Look for nP+ in the right-to-left/upwards direction
	 * Check if there's a nP+ hop in the gap from the right (facing left/up).
	 * a, The next hop right after the up-ramp was already retrieved from the database,
	 *    so just check if that hop was nP+.
	 * b, Also, don't check last hop before down-ramp starts
	 *    because there must be a hop of space in order for two
	 *    nP+ hops to oppose each other.
	 * c, RR points to the left end of the hop last checked.
	 * d, Checking stops if the hop is nP+.
	 *
	 *        Last chance of finding a relevant nP+ hop
	 *           /
	 *   L      /\                  R
	 *   * -- * -- * . . . . . * -- *
	 *  /    L+1              R-1    \
	 * *       |<------------------|  *
	 * 0                             N-1
	 *
	 */

	size_t rr = r - 1;
	if (last_hop_right == ASPA_NOT_PROVIDER_PLUS) {
		found_nP_from_right = true;
	} else {
		while (rr > l) {
			size_t c = rr;
			rr--;
			if (aspa_check_hop(aspa_table, as_path[c], as_path[rr])
					== ASPA_NOT_PROVIDER_PLUS) {
				found_nP_from_right = true;
				break;
			}
		}
	}

	// II. Look for nP+ in the left-to-right/down direction
	// Check if there's a nP+ hop in the gap from the right (facing left/down).
	if (found_nP_from_right) {
		/*
		 * a, There's no need to check for an nP+ from the left if we
		 *    didn't find an nP+ from the right before.
		 * b, The next hop right after the down-ramp was already retrieved from the database,
		 *    so just check if that hop was nP+.
		 * c, LL points to the right end of the hop last checked.
		 * d, Checking stops if the hop is nP+.
		 *
		 *                    Last chance of finding a relevant nP+ hop
		 *                       /
		 *  L    LL             /\
		 *   * -- * . . . . . * -- * . . . . .
		 *  /    L+1               RR
		 * *       |------------->|
		 * 0
		 *
		 */
		size_t ll = l + 1;
		if (last_hop_left == ASPA_NOT_PROVIDER_PLUS) {
			found_nP_from_left = true;
		} else {
			while (ll < rr) {
				size_t c = ll;
				ll++;
				if (aspa_check_hop(aspa_table, as_path[c], as_path[ll])
						== ASPA_NOT_PROVIDER_PLUS) {
					found_nP_from_left = true;
					break;
				}
			}
		}
	}

	// If two nP+ occur in opposing directions, return INVALID.
    if (found_nP_from_right && found_nP_from_left) {
        pthread_rwlock_unlock(&aspa_table->lock);
        return ASPA_AS_PATH_INVALID;
    }

    pthread_rwlock_unlock(&aspa_table->lock);
	return ASPA_AS_PATH_UNKNOWN;
}

RTRLIB_EXPORT enum aspa_verification_result aspa_verify_as_path(struct aspa_table *aspa_table, enum aspa_direction direction, uint32_t as_path[], size_t len)
{
	switch (direction) {
		case ASPA_UPSTREAM:
			return aspa_verify_upstream(aspa_table, as_path, len);
		case ASPA_DOWNSTREAM:
			return aspa_verify_downstream(aspa_table, as_path, len);
	}
	
	return ASPA_AS_PATH_UNKNOWN;
}


RTRLIB_EXPORT size_t aspa_collapse_as_path(uint32_t as_path[], size_t len)
{
	if (len == 0)
		return 0;

	size_t i = 1;

	while (i < len && as_path[i-1] != as_path[i])
		i++;

	if (i == len)
		return len;

	size_t j = i;

	i++;

	while (true) { // equivalent to while (i < len)
		while (i < len && as_path[i-1] == as_path[i])
			i++;

		if (i == len)
			break;

		as_path[j++] = as_path[i++];
	}

	return j;
}
