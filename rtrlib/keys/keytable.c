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


//****NOTE****
//All of the following code needs more error handling and of course testing, will come back to later

void key_table_init(struct key_table *key_table) {
	tommy_hashlin hashlin;
	key_table->hashtable = &hashlin;
	tommy_hashlin_init(key_table->hashtable);
	pthread_rwlock_init(&key_table->lock, NULL);
	key_table->cmp_fp = asn_cmp;
}


int asn_cmp(const void *arg, const void *obj) {
	return *(const uint32_t*)arg != ((const struct key_entry*)obj)->asn;
}


int key_table_add_entry(struct key_table *key_table, struct key_entry *key_entry) {
	pthread_rwlock_wrlock(&key_table->lock);
	tommy_hashlin_insert(key_table->hashtable, &key_entry->node, key_entry, tommy_inthash_u32(key_entry->asn));
	pthread_rwlock_unlock(&key_table->lock);
	return KEY_SUCCESS;
}



struct key_entry* key_table_get(struct key_table *key_table, uint32_t asn) {

	pthread_rwlock_wrlock(&key_table->lock);

	//A tommy node contains its storing key_entry (->data) as well as next and prev pointer
	//to accomodate multiple results
	tommy_node *result = tommy_hashlin_bucket(key_table->hashtable,tommy_inthash_u32(asn));

	pthread_rwlock_unlock(&key_table->lock);

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


//Barebones still..
int key_table_remove(struct key_table *key_table, struct key_entry *key_entry) {

	pthread_rwlock_wrlock(&key_table->lock);
	tommy_hashlin_remove_existing(key_table->hashtable,&key_entry->node);
	pthread_rwlock_unlock(&key_table->lock);
	return KEY_SUCCESS;
}