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

static int aspa_store_insert(struct aspa_store_node **store, struct rtr_socket *rtr_socket,
			     struct aspa_array *aspa_array)
{
	// Allocate new node
	struct aspa_store_node *new = lrtr_malloc(sizeof(struct aspa_store_node));

	if (new == NULL)
		return ASPA_ERROR;

	// Store socket and ASPA array
	new->rtr_socket = rtr_socket;
	new->aspa_array = aspa_array;

	if (*store == NULL) {
		*store = new;
	} else {
		// prepend new node
		new->next = *store;
		*store = new;
	}

	return ASPA_SUCCESS;
}

static void aspa_store_remove(struct aspa_store_node *head, struct rtr_socket *rtr_socket)
{
	if (head == NULL)
		return;

	struct aspa_store_node *node = head;
	struct aspa_store_node *prev;

	// If first node matches
	if (node != NULL && node->rtr_socket == rtr_socket) {
		node = head;
		head = node->next;
		lrtr_free(node);
		return;
	}

	while (node != NULL && node->rtr_socket != rtr_socket) {
		prev = node;
		node = node->next;
	}

	if (node == NULL)
		return;

	prev->next = node->next;
	lrtr_free(node);
}

static struct aspa_array *aspa_store_search(struct aspa_store_node *head, const struct rtr_socket *rtr_socket)
{
	if (head == NULL)
		return NULL;

	struct aspa_store_node *node = head;

	while (node != NULL) {
		if (node->rtr_socket == rtr_socket) {
			return node->aspa_array;
		}
		node = node->next;
	}

	return NULL;
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
		array = aspa_store_search(aspa_table->store, rtr_socket);

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
		array = aspa_store_search(aspa_table->store, rtr_socket);

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

	struct aspa_array *array = aspa_store_search(aspa_table->store, rtr_socket);

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
	aspa_store_remove(aspa_table->store, rtr_socket);

	// Release all records associated with the socket
	if (aspa_array_free(array) < 0) {
		pthread_rwlock_unlock(&(aspa_table->lock));
		return ASPA_ERROR;
	}

	pthread_rwlock_unlock(&(aspa_table->lock));
	return ASPA_SUCCESS;
}

void aspa_table_swap(struct aspa_table *a, struct aspa_table *b)
{
	pthread_rwlock_wrlock(&a->lock);
	pthread_rwlock_wrlock(&b->lock);

	struct aspa_store_node *tmp = a->store;
	a->store = b->store;
	b->store = tmp;

	pthread_rwlock_unlock(&b->lock);
	pthread_rwlock_unlock(&a->lock);
}

int aspa_table_copy_except_socket(struct aspa_table *src_table, struct aspa_table *dst_table,
				  const struct rtr_socket *socket)
{
	if (src_table == NULL || dst_table == NULL)
		return ASPA_ERROR;

	// Check if we need to copy anything
	if (src_table->store == NULL)
		return ASPA_SUCCESS;

	struct aspa_store_node *original = src_table->store;

	struct aspa_store_node *new_head = lrtr_malloc(sizeof(struct aspa_store_node));

	if (new_head == NULL)
		return ASPA_ERROR;
	
	if (original->rtr_socket == socket)
		original = original->next;

	if (aspa_array_copy(&new_head->aspa_array, original->aspa_array) < 0) {
		lrtr_free(new_head);
		return ASPA_ERROR;
	}

	struct aspa_store_node *node = new_head;
	original = original->next;

	while (original != NULL) {
		if (original->rtr_socket != socket) {
			node->next = lrtr_malloc(sizeof(struct aspa_store_node));
			
			if (node->next == NULL) {
				// if malloc failed
				lrtr_free(new_head);
				return ASPA_ERROR;
			}
			
			node = node->next;
			
			if (aspa_array_copy(&node->aspa_array, original->aspa_array) < 0) {
				lrtr_free(new_head);
				lrtr_free(node);
				return ASPA_ERROR;
			}
		}

		original = original->next;
	}

	node->next = NULL;
	dst_table->store = new_head;

	return ASPA_SUCCESS;
}

void aspa_table_notify_diff(struct aspa_table *new_table, struct aspa_table *old_table, const struct rtr_socket *socket)
{
	if (new_table == NULL || old_table == NULL || new_table == old_table)
		return;

	struct aspa_array *new_array;
	struct aspa_array *old_array;

	// Try fast lookup
	if (socket->aspa_table == new_table)
		new_array = socket->aspa_array;
	else if (socket->aspa_table == old_table)
		old_array = socket->aspa_array;

	if (new_array == NULL)
		new_array = aspa_store_search(new_table->store, socket);

	if (old_array == NULL)
		new_array = aspa_store_search(old_table->store, socket);

	if (old_array != NULL) {
		// Notify clients about these records being removed
		for (size_t i = 0; i < old_array->size; i++)
			aspa_table_notify_clients(new_table, &(old_array->data[i]), socket, false);
	}

	if (new_array != NULL) {
		// Notify clients about these records being added
		for (size_t i = 0; i < old_array->size; i++)
			aspa_table_notify_clients(new_table, &(old_array->data[i]), socket, true);
	}
}
