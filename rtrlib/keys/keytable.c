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
#include <stdio.h>


//****NOTE****
//All of the following code needs more error handling and of course testing, will come back to later

void key_table_init(struct key_table *key_table) {
	tommy_hashlin_init(&key_table->hashtable);
	tommy_list_init(&key_table->list);
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
	int rtval = KEY_ERROR;

	pthread_rwlock_wrlock(&key_table->lock);
	if(tommy_hashlin_search(&key_table->hashtable, key_table->cmp_fp, key_entry, hash)){
		rtval = KEY_DUPLICATE_RECORD;
	} else{
		//Insert into hashtable and list
		tommy_hashlin_insert(&key_table->hashtable, &key_entry->hash_node, key_entry, hash);
		tommy_list_insert_tail(&key_table->list, &key_entry->list_node, key_entry);
		printf("Add Router Key: ASN: %u\n",key_entry->asn);
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
	tommy_node *result = tommy_hashlin_bucket(&key_table->hashtable, hash);
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

	if(!tommy_hashlin_search(&key_table->hashtable, key_table->cmp_fp, key_entry, hash)){
		rtval = KEY_RECORD_NOT_FOUND;
		printf("Could not remove Router Key: ASN: %u (key not found)\n",key_entry->asn);
	} else {

		//Remove from hashtable and list, needs more error handling
		struct key_entry *rmv_elem = tommy_hashlin_remove(&key_table->hashtable, key_table->cmp_fp, key_entry, hash);
		rmv_elem = tommy_list_remove_existing(&key_table->list, &rmv_elem->list_node);

		if(!rmv_elem){
			rtval = KEY_ERROR;
		}
		else{
			rtval = KEY_SUCCESS;
			printf("Removed Router Key: ASN: %u\n",rmv_elem->asn);
			free(rmv_elem);
		}

	}

	pthread_rwlock_unlock(&key_table->lock);
	free(key_entry);
	return rtval;
}

//TODO: Add validate_router_key function
//TODO: Add notify_clients function

//Needs error handling and testing
int key_table_src_remove(struct key_table *key_table, const struct rtr_socket *socket) {

	//Find all key_entry that have key_entry->socket == socket, link them together
	tommy_node *current_node = tommy_list_head(&key_table->list);

	struct key_entry *del_head = NULL;
	struct key_entry *del_tail = NULL;
	struct key_entry *current_entry = NULL;


	while(current_node){
		current_entry = (struct key_entry *)current_node->data;

		if(current_entry->socket == socket){
			if(del_head){
				//Attach current_entry to the end, make it the new tail
				del_tail->next = current_entry;
				del_tail = current_entry;
				del_tail->next = NULL;
			} else {
				//If this is the first elem that maches the socket
				//Init head and tail
				del_head = current_entry;
				del_tail = current_entry;
				del_head->next = del_tail;
			}
		}
		//Iterate over key_table->list
		current_node = current_node->next;
	}

	current_entry = del_head;

	struct key_entry *temp;

	//Remove all elements we found from hashtable and list, free them
	pthread_rwlock_wrlock(&key_table->lock);
	while(current_entry){
		tommy_hashlin_remove_existing(&key_table->hashtable, &current_entry->hash_node);
		tommy_list_remove_existing(&key_table->list, &current_entry->list_node);
		temp = current_entry;
		current_entry = current_entry->next;
		free(temp);
	}
	pthread_rwlock_unlock(&key_table->lock);
	return KEY_SUCCESS;

}






