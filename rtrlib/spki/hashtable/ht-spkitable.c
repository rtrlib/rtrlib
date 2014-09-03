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
 *
 * written by Andreas Reuter and Nils Bars, in cooperation with:
 * INET group, Hamburg University of Applied Sciences,
 * CST group, Freie Universitaet Berlin
 * Website: http://rpki.realmv6.org/
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


/**
 * @brief Calls the spki_table update function.
 * @param[in] spki_table spki_table to use.
 * @param[in] record spki_record that was added/removed.
 * @param[in] added True means record was added, False means removed
 */
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


int spki_table_get_all(struct spki_table *spki_table, uint32_t asn, uint8_t *ski, struct spki_record **result, size_t *result_size){
    uint32_t hash = tommy_inthash_u32(asn);
    *result = NULL;
    *result_size = 0;

    pthread_rwlock_rdlock(&spki_table->lock);

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
        if(element->asn == asn && memcmp(element->ski, ski, sizeof(element->ski)) == 0){
            (*result_size)++;
            void* tmp = realloc(*result, *result_size * sizeof(struct spki_record));
            if(tmp == NULL){
                free(*result);
                pthread_rwlock_unlock(&spki_table->lock);
                return SPKI_ERROR;
            }
            *result = tmp;
            key_entry_to_spki_record(element, *result + *result_size-1);
        }
        result_bucket = result_bucket->next;
    }

    pthread_rwlock_unlock(&spki_table->lock);
    return SPKI_SUCCESS;
}

int spki_table_search_by_ski(struct spki_table *spki_table, uint8_t *ski, struct spki_record **result, size_t *result_size){
    *result = NULL;
    *result_size = 0;
    struct key_entry *current_entry;

    pthread_rwlock_rdlock(&spki_table->lock);

    tommy_node *current_node = tommy_list_head(&spki_table->list);
    while(current_node){
        current_entry = (struct key_entry *)current_node->data;

        if(memcmp(current_entry->ski, ski, sizeof(current_entry->ski)) == 0){
            (*result_size)++;
            void* tmp = realloc(*result, sizeof(**result) * (*result_size));
            if(tmp == NULL){
                free(*result);
                pthread_rwlock_unlock(&spki_table->lock);
                return SPKI_ERROR;
            }
            *result = tmp;
            key_entry_to_spki_record(current_entry, (*result + *result_size-1));
        }
        current_node = current_node->next;
    }
    pthread_rwlock_unlock(&spki_table->lock);
    return SPKI_SUCCESS;
}

int spki_table_remove_entry(struct spki_table *spki_table, struct spki_record *spki_record) {

    struct key_entry entry;
    spki_record_to_key_entry(spki_record, &entry);

    uint32_t hash = tommy_inthash_u32(spki_record->asn);
    int rtval;

    pthread_rwlock_wrlock(&spki_table->lock);

    if(!tommy_hashlin_search(&spki_table->hashtable, spki_table->cmp_fp, &entry, hash)){
        rtval = SPKI_RECORD_NOT_FOUND;
    } else {

        //Remove from hashtable and list
        struct key_entry *rmv_elem = tommy_hashlin_remove(&spki_table->hashtable, spki_table->cmp_fp, &entry, hash);
        if(rmv_elem && tommy_list_remove_existing(&spki_table->list, &rmv_elem->list_node)){
            rtval = SPKI_SUCCESS;
            spki_table_notify_clients(spki_table, spki_record, false);
            free(rmv_elem);
        } else{
            rtval = SPKI_ERROR;
        }
    }
    pthread_rwlock_unlock(&spki_table->lock);
    return rtval;
}

int spki_table_src_remove(struct spki_table *spki_table, const struct rtr_socket *socket) {
    struct key_entry *current_entry = NULL;

    pthread_rwlock_wrlock(&spki_table->lock);

    tommy_node *current_node = tommy_list_head(&spki_table->list);
    while(current_node){
        current_entry = (struct key_entry *)current_node->data;

        if(current_entry->socket == socket){
                current_node = current_node->next;
                if(!tommy_list_remove_existing(&spki_table->list, &current_entry->list_node)){
                    pthread_rwlock_unlock(&spki_table->lock);
                    return SPKI_ERROR;
                }
                if(!tommy_hashlin_remove_existing(&spki_table->hashtable, &current_entry->hash_node)){
                    pthread_rwlock_unlock(&spki_table->lock);
                    return SPKI_ERROR;
                }
                free(current_entry);
        } else {
            current_node = current_node->next;
        }
    }
    pthread_rwlock_unlock(&spki_table->lock);
    return SPKI_SUCCESS;
}

//Local functions -------------------------

/**
 * @brief Compares two key_entrys by comparing ASN, SKI and SPKI
 * @param[in] arg Pointer to first key_entry
 * @param[in] obj Pointer to second key_entry
 * @return 1 if not equal
 * @return 0 if equal
 */
static int key_entry_cmp(const void *arg, const void *obj) {
    struct key_entry *param = (struct key_entry*)arg;
    struct key_entry *entry = (struct key_entry*)obj;

	if(param->asn != entry->asn)
		return 1;
    if(memcmp(param->ski,entry->ski, sizeof(entry->ski)))
		return 1;
    if(memcmp(param->spki,entry->spki,sizeof(entry->spki)))
		return 1;
    if(param->socket != entry->socket)
        return 1;

    return 0;
}

//Copying the content, target struct must already be allocated
static int key_entry_to_spki_record(struct key_entry *key_e, struct spki_record *spki_r){
    spki_r->asn = key_e->asn;
    spki_r->socket = key_e->socket;
    memcpy(spki_r->ski, key_e->ski, sizeof(key_e->ski));
    memcpy(spki_r->spki, key_e->spki, sizeof(key_e->spki));
    return SPKI_SUCCESS;
}

//Copying the content, target struct must already be allocated
static int spki_record_to_key_entry(struct spki_record *spki_r, struct key_entry *key_e){
    key_e->asn = spki_r->asn;
    key_e->socket = spki_r->socket;
    memcpy(key_e->ski, spki_r->ski, sizeof(spki_r->ski));
    memcpy(key_e->spki, spki_r->spki, sizeof(spki_r->spki));
    return SPKI_SUCCESS;
}
