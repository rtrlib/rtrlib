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



static int key_entry_cmp(const void *arg, const void *obj);
static int key_entry_to_spki_record(struct key_entry *key_e, struct spki_record *spki_r);
static int spki_record_to_key_entry(struct spki_record *spki_r, struct key_entry *key_e);
static void spki_table_notify_clients(struct spki_table *spki_table, const struct spki_record *record, const bool added);

//****NOTE****
//All of the following code needs more error handling and of course testing, will come back to later
void spki_table_notify_clients(struct spki_table *spki_table, const struct spki_record *record, const bool added) {
    if(spki_table->update_fp != NULL)
        spki_table->update_fp(spki_table, *record, added);
}


void spki_table_init(struct spki_table *spki_table, spki_update_fp update_fp) {
    tommy_hashlin_init(&spki_table->hashtable);
    tommy_list_init(&spki_table->list);
    pthread_rwlock_init(&spki_table->lock, NULL);
    spki_table->cmp_fp = key_entry_cmp;
    spki_table->update_fp = update_fp;
}

void spki_table_free(struct spki_table *spki_table){

    pthread_rwlock_wrlock(&spki_table->lock);

    tommy_list_foreach(&spki_table->list, free);

    //Deinitialize the hashtable
    tommy_hashlin_done(&spki_table->hashtable);

    pthread_rwlock_unlock(&spki_table->lock);
    pthread_rwlock_destroy(&spki_table->lock);
}

//TODO: Remove printf statements and do real output (notify clients?)
int spki_table_add_entry(struct spki_table *spki_table, struct spki_record *spki_record) {
    struct key_entry *entry = malloc(sizeof(struct key_entry));
    if(entry == NULL){
        return SPKI_ERROR;
    }

    spki_record_to_key_entry(spki_record, entry);

    uint32_t hash = tommy_inthash_u32(spki_record->asn);
    int rtval = SPKI_ERROR;

    pthread_rwlock_wrlock(&spki_table->lock);
    if(tommy_hashlin_search(&spki_table->hashtable, spki_table->cmp_fp, entry, hash)){
        rtval = SPKI_DUPLICATE_RECORD;
        free(entry);
	} else{
		//Insert into hashtable and list
        tommy_hashlin_insert(&spki_table->hashtable, &entry->hash_node, entry, hash);
        tommy_list_insert_tail(&spki_table->list, &entry->list_node, entry);
        spki_table_notify_clients(spki_table, spki_record, true);
        rtval = SPKI_SUCCESS;
	}
    pthread_rwlock_unlock(&spki_table->lock);
	return rtval;
}


int spki_table_get_all(struct spki_table *spki_table, uint32_t asn, uint8_t *ski, struct spki_record **result, unsigned int *result_size){
    uint32_t hash = tommy_inthash_u32(asn);
    (*result) = NULL;
    (*result_size) = 0;

    pthread_rwlock_wrlock(&spki_table->lock);

    //A tommy node contains its storing key_entry (->data) as well as next and prev pointer
    //to accomodate multiple results.
    //The bucket is guaranteed to contain ALL the elements with the specified hash,
    //but it can contain also others.
    tommy_node *result_bucket = tommy_hashlin_bucket(&spki_table->hashtable, hash);

    if(!result_bucket){
        pthread_rwlock_unlock(&spki_table->lock);
        return SPKI_SUCCESS;
    }

    //Build the result array
    struct key_entry *element;
    while(result_bucket){
        element = result_bucket->data;
        if(element->asn == asn && memcmp(element->ski, ski, SKI_SIZE) == 0){
            (*result_size)++;
            (*result) = (struct spki_record*) realloc(*result, *result_size * sizeof(struct spki_record));
            if(!(*result)){
                free(*result);
                *result = NULL;
                pthread_rwlock_unlock(&spki_table->lock);
                return SPKI_ERROR;
            }
            key_entry_to_spki_record(element, &((*result)[(*result_size-1)]));
        }
        result_bucket = result_bucket->next;
    }

    pthread_rwlock_unlock(&spki_table->lock);
    return SPKI_SUCCESS;
}


int spki_table_remove_entry(struct spki_table *spki_table, struct spki_record *spki_record) {

    struct key_entry *entry = malloc(sizeof(struct key_entry));
    if(entry == NULL){
        return SPKI_ERROR;
    }
    spki_record_to_key_entry(spki_record, entry);

    uint32_t hash = tommy_inthash_u32(spki_record->asn);
    int rtval;

    pthread_rwlock_wrlock(&spki_table->lock);

    if(!tommy_hashlin_search(&spki_table->hashtable, spki_table->cmp_fp, entry, hash)){
        rtval = SPKI_RECORD_NOT_FOUND;
    } else {

        //Remove from hashtable and list
        struct key_entry *rmv_elem = tommy_hashlin_remove(&spki_table->hashtable, spki_table->cmp_fp, entry, hash);
        if(rmv_elem && tommy_list_remove_existing(&spki_table->list, &rmv_elem->list_node)){
            rtval = SPKI_SUCCESS;
            spki_table_notify_clients(spki_table, spki_record, false);
            free(rmv_elem);
        } else{
            rtval = SPKI_ERROR;
        }
    }
    pthread_rwlock_unlock(&spki_table->lock);
    free(entry);
    return rtval;
}



int spki_table_src_remove(struct spki_table *spki_table, const struct rtr_socket *socket) {

    //Find all spki_record that have spki_record->socket == socket, link them together
    tommy_node *current_node = tommy_list_head(&spki_table->list);

    struct key_entry *del_head = NULL;
    struct key_entry *del_tail = NULL;
    struct key_entry *current_entry = NULL;


    while(current_node){
        current_entry = (struct key_entry *)current_node->data;

        //If socket matches, mark for deletion by linking
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
        //Iterate over spki_table->list
        current_node = current_node->next;
    }
    //Now we have a singly linked list of key_entry to delete, starting at del_head 
    current_entry = del_head;

    struct key_entry *temp;

    pthread_rwlock_wrlock(&spki_table->lock);
    
    //Remove all elements we found from hashtable and list, free them
    while(current_entry){

        temp = current_entry;
        //Remove from list
        if(!tommy_list_remove_existing(&spki_table->list, &current_entry->list_node)){
            return SPKI_ERROR;
        }

        //Remove from hashtable
        if(!tommy_hashlin_remove_existing(&spki_table->hashtable, &current_entry->hash_node)){
            return SPKI_ERROR;
        }
        current_entry = current_entry->next;
        free(temp);
    }
    pthread_rwlock_unlock(&spki_table->lock);
    return SPKI_SUCCESS;

}

//Local functions -------------------------

//TODO Add enum for rtvals?
static int key_entry_cmp(const void *arg, const void *obj) {
    struct key_entry *param = (struct key_entry*)arg;
    struct key_entry *entry = (struct key_entry*)obj;

	if(param->asn != entry->asn)
		return 1;
	if(memcmp(param->ski,entry->ski,SKI_SIZE))
		return 1;
	if(memcmp(param->spki,entry->spki,SPKI_SIZE))
		return 1;
    if(param->socket != entry->socket)
        return 1;

    return 0;
}

//Copying the content, target struct must already be allocated
static int key_entry_to_spki_record(struct key_entry *key_e, struct spki_record *spki_r){
    spki_r->asn = key_e->asn;
    spki_r->socket = key_e->socket;
    memcpy(spki_r->ski, key_e->ski, SKI_SIZE);
    memcpy(spki_r->spki, key_e->spki, SPKI_SIZE);
    return SPKI_SUCCESS;
}

//Copying the content, target struct must already be allocated
static int spki_record_to_key_entry(struct spki_record *spki_r, struct key_entry *key_e){
    key_e->asn = spki_r->asn;
    key_e->socket = spki_r->socket;
    memcpy(key_e->ski, spki_r->ski, SKI_SIZE);
    memcpy(key_e->spki, spki_r->spki, SPKI_SIZE);
    return SPKI_SUCCESS;
}









