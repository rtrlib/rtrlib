/*
 * This file is part of RTRlib.
 *
 * RTRlib is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * RTRlib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RTRlib; see the file COPYING.LESSER.
 */

#include "rtrlib/keys/keytable.h"
#include <string.h>


//****NOTE****
//All of the following code needs more error handling and of course testing, will come back to later

void key_table_init(struct key_table *key_table) {
	tommy_hashlin hashlin;
	key_table->hashtable = &hashlin;
	tommy_hashlin_init(key_table->hashtable);
	pthread_rwlock_init(&key_table->lock, NULL);
	key_table->cmp_fp = key_entry_cmp;
}


int asn_cmp(const void *arg, const void *obj) {
	return *(const uint32_t*)arg != ((const struct key_entry*)obj)->asn;
}


int key_entry_cmp(const void *arg, const void *obj) {
	struct key_entry *param = (struct key_entry*)arg;
	struct key_entry *entry = (struct key_entry*)obj;

	if(param->asn != entry->asn)
		return 1;
	if(memcmp(param->ski,entry->ski,SKI_SIZE))
		return 1;
	if(memcmp(param->spki,entry->spki,SPKI_SIZE))
		return 1;

	return 0;
}

//TODO: Handle KEY_ERROR case
int key_table_add_entry(struct key_table *key_table, struct key_entry *key_entry) {
	uint32_t hash = tommy_inthash_u32(key_entry->asn);
	int rtval;

	pthread_rwlock_wrlock(&key_table->lock);
	if(tommy_hashlin_search(key_table->hashtable, key_table->cmp_fp, key_entry, hash)){
		rtval = KEY_DUPLICATE_RECORD;
	} else{
		tommy_hashlin_insert(key_table->hashtable, &key_entry->node, key_entry, hash);
		rtval = KEY_SUCCESS;
	}
	pthread_rwlock_unlock(&key_table->lock);
	return rtval;
}



struct key_entry* key_table_get_all(struct key_table *key_table, uint32_t asn) {


	uint32_t hash = tommy_inthash_u32(asn);

	pthread_rwlock_wrlock(&key_table->lock);
	//A tommy node contains its storing key_entry (->data) as well as next and prev pointer
	//to accomodate multiple results
	tommy_node *result = tommy_hashlin_bucket(key_table->hashtable, hash);
	pthread_rwlock_unlock(&key_table->lock);

	if(!result){
		return NULL;
	}


	//We link up the key_entry structs so we don't end up using tommy nodes outside of the keys files.
	struct key_entry *head = result->data;
	struct key_entry *current = head;
	while(result){
		current->next = result->next->data;
		result = result->next;
		current = current->next;
	}
	return head;

}


//TODO: Handle KEY_ERROR case
int key_table_remove_entry(struct key_table *key_table, struct key_entry *key_entry) {

	uint32_t hash = tommy_inthash_u32(key_entry->asn);
	int rtval;

	pthread_rwlock_wrlock(&key_table->lock);

	struct key_entry *rmv_elem = tommy_hashlin_remove(key_table->hashtable, key_table->cmp_fp, key_entry, hash);

	if(rmv_elem){
		rtval = KEY_SUCCESS;
		free(rmv_elem);
	} else {
		rtval = KEY_RECORD_NOT_FOUND;
	}
	pthread_rwlock_unlock(&key_table->lock);

	return rtval;
}