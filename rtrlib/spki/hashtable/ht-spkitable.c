/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
*/

#include "rtrlib/spki/hashtable/ht-spkitable.h"

#include <string.h>
#include <stdio.h>
#include <pthread.h>

struct key_entry {
	uint8_t ski[SKI_SIZE];
	uint32_t asn;
	uint8_t spki[SPKI_SIZE];
	const struct rtr_socket *socket;
	tommy_node hash_node;
	tommy_node list_node;
};

/**
 * @brief Compares two key_entrys by comparing ASN, SKI and SPKI
 * @param[in] arg Pointer to first key_entry
 * @param[in] obj Pointer to second key_entry
 * @return 1 if not equal
 * @return 0 if equal
 */
static int key_entry_cmp(const void *arg, const void *obj)
{
	const struct key_entry *param = arg;
	const struct key_entry *entry = obj;

	if (param->asn != entry->asn)
		return 1;
	if (memcmp(param->ski, entry->ski, sizeof(entry->ski)))
		return 1;
	if (memcmp(param->spki, entry->spki, sizeof(entry->spki)))
		return 1;
	if (param->socket != entry->socket)
		return 1;

	return 0;
}

/* Copying the content, target struct must already be allocated */
static void key_entry_to_spki_record(struct key_entry *key_e,
				    struct spki_record *spki_r)
{
	spki_r->asn = key_e->asn;
	spki_r->socket = key_e->socket;
	memcpy(spki_r->ski, key_e->ski, sizeof(key_e->ski));
	memcpy(spki_r->spki, key_e->spki, sizeof(key_e->spki));
}

/* Copying the content, target struct must already be allocated */
static void spki_record_to_key_entry(struct spki_record *spki_r,
				    struct key_entry *key_e)
{
	key_e->asn = spki_r->asn;
	key_e->socket = spki_r->socket;
	memcpy(key_e->ski, spki_r->ski, sizeof(spki_r->ski));
	memcpy(key_e->spki, spki_r->spki, sizeof(spki_r->spki));
}

/**
 * @brief Calls the spki_table update function.
 * @param[in] spki_table spki_table to use.
 * @param[in] record spki_record that was added/removed.
 * @param[in] added True means record was added, False means removed
 */
static void spki_table_notify_clients(struct spki_table *spki_table,
				      const struct spki_record *record,
				      const bool added)
{
	if (spki_table->update_fp != NULL)
		spki_table->update_fp(spki_table, *record, added);
}

void spki_table_init(struct spki_table *spki_table, spki_update_fp update_fp)
{
	tommy_hashlin_init(&spki_table->hashtable);
	tommy_list_init(&spki_table->list);
	pthread_rwlock_init(&spki_table->lock, NULL);
	spki_table->cmp_fp = key_entry_cmp;
	spki_table->update_fp = update_fp;
}

void spki_table_free(struct spki_table *spki_table)
{
	pthread_rwlock_wrlock(&spki_table->lock);

	tommy_list_foreach(&spki_table->list, free);
	tommy_hashlin_done(&spki_table->hashtable);

	pthread_rwlock_unlock(&spki_table->lock);
	pthread_rwlock_destroy(&spki_table->lock);
}

int spki_table_add_entry(struct spki_table *spki_table,
			 struct spki_record *spki_record)
{
	uint32_t hash;
	struct key_entry *entry;

	entry = malloc(sizeof(*entry));
	if (entry == NULL)
		return SPKI_ERROR;

	spki_record_to_key_entry(spki_record, entry);
	hash = tommy_inthash_u32(spki_record->asn);

	pthread_rwlock_wrlock(&spki_table->lock);
	if (tommy_hashlin_search(&spki_table->hashtable, spki_table->cmp_fp,
				 entry, hash)) {
		free(entry);
		pthread_rwlock_unlock(&spki_table->lock);
		return SPKI_DUPLICATE_RECORD;
	}

	/* Insert into hashtable and list */
	tommy_hashlin_insert(&spki_table->hashtable, &entry->hash_node,
			     entry, hash);
	tommy_list_insert_tail(&spki_table->list, &entry->list_node,
			       entry);
	pthread_rwlock_unlock(&spki_table->lock);
	spki_table_notify_clients(spki_table, spki_record, true);
	return SPKI_SUCCESS;
}

int spki_table_get_all(struct spki_table *spki_table, uint32_t asn,
		       uint8_t *ski, struct spki_record **result,
		       unsigned int *result_size)
{
	uint32_t hash = tommy_inthash_u32(asn);
	tommy_node *result_bucket;
	struct key_entry *element;
	void *tmp;

	*result = NULL;
	*result_size = 0;

	pthread_rwlock_rdlock(&spki_table->lock);

	/**
	 * A tommy node contains its storing key_entry (->data) as well as
	 * next and prev pointer to accomodate multiple results.
	 * The bucket is guaranteed to contain ALL the elements
	 * with the specified hash, but it can contain also others.
	 */
	result_bucket = tommy_hashlin_bucket(&spki_table->hashtable, hash);

	if (!result_bucket) {
		pthread_rwlock_unlock(&spki_table->lock);
		return SPKI_SUCCESS;
	}

	/* Build the result array */
	while (result_bucket) {
		element = result_bucket->data;
		if (element->asn == asn &&
		    memcmp(element->ski, ski, sizeof(element->ski)) == 0) {
			(*result_size)++;
			tmp = realloc(*result, *result_size * sizeof(**result));
			if (tmp == NULL) {
				free(*result);
				pthread_rwlock_unlock(&spki_table->lock);
				return SPKI_ERROR;
			}
			*result = tmp;
			key_entry_to_spki_record(element,
						 *result + *result_size-1);
		}
		result_bucket = result_bucket->next;
	}

	pthread_rwlock_unlock(&spki_table->lock);
	return SPKI_SUCCESS;
}

int spki_table_search_by_ski(struct spki_table *spki_table, uint8_t *ski,
			     struct spki_record **result,
			     unsigned int *result_size)
{
	struct key_entry *current_entry;
	tommy_node *current_node;
	void *tmp;
	*result = NULL;
	*result_size = 0;

	pthread_rwlock_rdlock(&spki_table->lock);

	current_node = tommy_list_head(&spki_table->list);
	while (current_node) {
		current_entry = (struct key_entry *)current_node->data;

		if (memcmp(current_entry->ski, ski,
			   sizeof(current_entry->ski)) == 0) {
			(*result_size)++;
			tmp = realloc(*result,
				      sizeof(**result) * (*result_size));
			if (tmp == NULL) {
				free(*result);
				pthread_rwlock_unlock(&spki_table->lock);
				return SPKI_ERROR;
			}
			*result = tmp;
			key_entry_to_spki_record(current_entry,
						 *result + (*result_size-1));
		}
		current_node = current_node->next;
	}
	pthread_rwlock_unlock(&spki_table->lock);
	return SPKI_SUCCESS;
}

int spki_table_remove_entry(struct spki_table *spki_table,
			    struct spki_record *spki_record)
{
	uint32_t hash;
	struct key_entry entry;
	struct key_entry *rmv_elem;
	int rtval;

	spki_record_to_key_entry(spki_record, &entry);
	hash = tommy_inthash_u32(spki_record->asn);

	pthread_rwlock_wrlock(&spki_table->lock);

	if (!tommy_hashlin_search(&spki_table->hashtable, spki_table->cmp_fp,
				  &entry, hash)) {
		rtval = SPKI_RECORD_NOT_FOUND;
		goto out;
	}

	/* Remove from hashtable and list */
	rmv_elem = tommy_hashlin_remove(&spki_table->hashtable,
					spki_table->cmp_fp, &entry, hash);
	if (rmv_elem && tommy_list_remove_existing(&spki_table->list,
						   &rmv_elem->list_node)) {
		free(rmv_elem);
		spki_table_notify_clients(spki_table, spki_record, false);
		rtval = SPKI_SUCCESS;
	}

out:
	pthread_rwlock_unlock(&spki_table->lock);
	return rtval;
}

int spki_table_src_remove(struct spki_table *spki_table,
			  const struct rtr_socket *socket)
{
	struct key_entry *entry;
	tommy_node *current_node;

	pthread_rwlock_wrlock(&spki_table->lock);

	current_node = tommy_list_head(&spki_table->list);
	while (current_node) {
		entry = current_node->data;
		if (entry->socket == socket) {
			current_node = current_node->next;
			if (!tommy_list_remove_existing(&spki_table->list,
							&entry->list_node)) {
				pthread_rwlock_unlock(&spki_table->lock);
				return SPKI_ERROR;
			}
			if (!tommy_hashlin_remove_existing(&spki_table->hashtable,
							   &entry->hash_node)) {
				pthread_rwlock_unlock(&spki_table->lock);
				return SPKI_ERROR;
			}
			free(entry);
		} else {
			current_node = current_node->next;
		}
	}
	pthread_rwlock_unlock(&spki_table->lock);
	return SPKI_SUCCESS;
}
