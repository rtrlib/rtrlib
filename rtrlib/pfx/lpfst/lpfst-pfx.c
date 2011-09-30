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
 * written by Fabian Holler, in cooperation with:
 * INET group, Hamburg University of Applied Sciences,
 * CST group, Freie Universitaet Berlin
 * Website: http://rpki.realmv6.org/
 */

#include "rtrlib/pfx/lpfst/lpfst-pfx.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct data_elem_t{
    uint32_t asn;
    uint8_t max_len;
    uintptr_t socket_id;
} data_elem;

typedef struct{
    unsigned int len;
    data_elem* ary;
} node_data;

static lpfst_node* pfx_table_get_root(const struct pfx_table* pfx_table, const ip_version ver);
static int pfx_table_del_elem(node_data* data, const unsigned int index);
static int pfx_table_create_node(lpfst_node** node, const pfx_record* record);
static int pfx_table_append_elem(node_data* data, const pfx_record* record);
static data_elem* pfx_table_find_elem(const node_data* data, const pfx_record* record, unsigned int* index);
static bool pfx_table_elem_matches(node_data* entry, const uint32_t asn, const uint32_t max_len);
static void pfx_table_notify_clients(pfx_table* pfx_table, const pfx_record* record, const bool added);
static int pfx_table_get_state(const lpfst_node* node, const uint32_t asn, const uint8_t mask_len, pfxv_state* result);
static int pfx_table_remove_id(pfx_table* pfx_table, lpfst_node** root, lpfst_node* node, const uintptr_t socket_id, const unsigned int level);


void pfx_table_notify_clients(pfx_table* pfx_table, const pfx_record* record, const bool added){
    if(pfx_table->update_fp == NULL)
        return;

    for(unsigned int i=0;i<pfx_table->update_fp_len;i++)
        pfx_table->update_fp[i](pfx_table, *record, added);
}

void pfx_table_init(struct pfx_table* pfx_table, rtr_update_fp update_fp[], unsigned int update_fp_len){
    pfx_table->ipv4 = NULL;
    pfx_table->ipv6 = NULL;
    pfx_table->update_fp = update_fp;
    pfx_table->update_fp_len = update_fp_len;
    pthread_rwlock_init(&(pfx_table->lock), NULL);
}

void pfx_table_free(struct pfx_table* pfx_table){
    for(int i = 0; i < 2; i++){
        lpfst_node* root = (i == 0 ? pfx_table->ipv4 : pfx_table->ipv6);
        lpfst_node* rm_node;
        if(root != NULL){
            pthread_rwlock_wrlock(&(pfx_table->lock));
            do{
                node_data* data = (node_data*) (root->data);
                for(unsigned int i = 0; i < data->len; i++){
                    pfx_record record = { data->ary[i].asn, (root->prefix), root->len, data->ary[i].max_len, data->ary[i].socket_id};
                    pfx_table_notify_clients(pfx_table, &record, false);
                }
                free(data);
                rm_node = (lpfst_remove(root, &(root->prefix), 0));
                free(rm_node);
            }
            while(rm_node != root);
            if(i == 0)
                pfx_table->ipv4 = NULL;
            else
                pfx_table->ipv6 = NULL;

            pthread_rwlock_unlock(&(pfx_table->lock));
        }
    }
    pthread_rwlock_destroy(&(pfx_table->lock));
}

int pfx_table_append_elem(node_data* data, const pfx_record* record){
    data->len++;
    data->ary = realloc(data->ary, sizeof(data_elem) * data->len);
    if(data->ary == NULL)
        return PFX_ERROR;
    data->ary[data->len - 1].asn = record->asn;
    data->ary[data->len - 1].max_len = record->max_len;
    data->ary[data->len - 1].socket_id = record->socket_id;
    return PFX_SUCCESS;
}

int pfx_table_create_node(lpfst_node** node, const pfx_record* record){
    *node = malloc(sizeof(lpfst_node));
    if(*node == NULL)
        return PFX_ERROR;
    (*node)->prefix = record->prefix;
    (*node)->len = record->min_len;
    (*node)->lchild = NULL;
    (*node)->rchild = NULL;
    (*node)->parent = NULL;

    (*node)->data = malloc(sizeof(node_data));
    if((*node)->data == NULL)
        return PFX_ERROR;
    ((node_data*) (*node)->data)->len = 0;
    ((node_data*) (*node)->data)->ary = NULL;

    return pfx_table_append_elem(((node_data*) (*node)->data), record);
}

data_elem* pfx_table_find_elem(const node_data* data, const pfx_record* record, unsigned int* index){
    for(unsigned int i = 0; i < data->len; i++){
        if(data->ary[i].asn == record->asn && data->ary[i].max_len == record->max_len && data->ary[i].socket_id == record->socket_id){
            if(index != NULL)
                *index = i;
            return &(data->ary[i]);
        }
    }
    return NULL;
}

//returns pfx_table->ipv4 if record version is IPV4 else pfx_table->ipv6
inline lpfst_node* pfx_table_get_root(const struct pfx_table* pfx_table, const ip_version ver){
    return ver == IPV4 ? pfx_table->ipv4 : pfx_table->ipv6;
}

int pfx_table_del_elem(node_data* data, const unsigned int index){
    //if index is not the last elem in the list, move all other elems backwards in the array
    if(index != data->len - 1){
        for(unsigned int i = index; i < data->len -1; i++){
            data->ary[i] = data->ary[i+1];
        }
    }
    data->len--;
    data->ary = realloc(data->ary, sizeof(data_elem) * data->len);
    if(data->len != 0 && data->ary == NULL)
        return PFX_ERROR;

    return PFX_SUCCESS;
}

int pfx_table_add(struct pfx_table* pfx_table, const pfx_record* record){
    pthread_rwlock_wrlock(&(pfx_table->lock));

    lpfst_node* root = pfx_table_get_root(pfx_table, record->prefix.ver);
    unsigned int lvl = 0;
    if(root != NULL){
        lpfst_node* node = lpfst_lookup_exact(root, &(record->prefix), record->min_len, &lvl);
        if(node != NULL){ //node with prefix exists
            if(pfx_table_find_elem(node->data, record, NULL) != NULL){
                pthread_rwlock_unlock(&pfx_table->lock);
                return PFX_DUPLICATE_RECORD; 
            }
            //append record to note_data array
            int rtval = pfx_table_append_elem(node->data, record);
            pthread_rwlock_unlock(&pfx_table->lock);
            pfx_table_notify_clients(pfx_table, record, true);
            return rtval;
        }

        lpfst_node* new_node = NULL;
        if(pfx_table_create_node(&new_node, record) == PFX_ERROR){
            pthread_rwlock_unlock(&pfx_table->lock);
            return PFX_ERROR;
        }
        lvl = 0;
        lpfst_insert(root, new_node, lvl);
        pthread_rwlock_unlock(&pfx_table->lock);
        pfx_table_notify_clients(pfx_table, record, true);
        return PFX_SUCCESS;
    }
    //tree is empty, record will be the root_node
    lpfst_node* new_node = NULL;
    if(pfx_table_create_node(&new_node, record) == PFX_ERROR){
        pthread_rwlock_unlock(&pfx_table->lock);
        return PFX_ERROR;
    }
    if(record->prefix.ver == IPV4)
        pfx_table->ipv4 = new_node;
    else
        pfx_table->ipv6 = new_node;

    pthread_rwlock_unlock(&pfx_table->lock);
    pfx_table_notify_clients(pfx_table, record, true);
    return PFX_SUCCESS;
}

int pfx_table_remove(struct pfx_table* pfx_table, const pfx_record* record){
    pthread_rwlock_wrlock(&(pfx_table->lock));
    lpfst_node* root = pfx_table_get_root(pfx_table, record->prefix.ver);

    unsigned int lvl = 0; //tree depth were node was found
    lpfst_node* node = lpfst_lookup_exact(root, &(record->prefix), record->min_len, &lvl);
    if(node == NULL){
        pthread_rwlock_unlock(&pfx_table->lock);
        return PFX_RECORD_NOT_FOUND;
    }

    unsigned int index;
    data_elem* elem = pfx_table_find_elem(node->data, record, &index);
    if(elem == NULL){
        pthread_rwlock_unlock(&pfx_table->lock);
        return PFX_RECORD_NOT_FOUND;
    }

    node_data* ndata = (node_data*) node->data;

    if(pfx_table_del_elem(ndata, index) == PFX_ERROR){
        pthread_rwlock_unlock(&pfx_table->lock);
        return PFX_ERROR;
    }

    if(ndata->len == 0){
        node = lpfst_remove(node, &(record->prefix), lvl);

        if(node == root){
            if(record->prefix.ver == IPV4)
                pfx_table->ipv4 = NULL;
            else
                pfx_table->ipv6 = NULL;
        }
        free(node);
    }
    pthread_rwlock_unlock(&pfx_table->lock);

    pfx_table_notify_clients(pfx_table, record, false);

    return PFX_SUCCESS;
}

bool pfx_table_elem_matches(node_data* data, const uint32_t asn, const uint32_t max_len){
    for(unsigned int i = 0; i < data->len; i++){
        if(data->ary[i].asn == asn && max_len <= data->ary[i].max_len)
            return true;
    }
    return false;
}

int pfx_table_get_state(const lpfst_node* node, const uint32_t asn, const uint8_t mask_len, pfxv_state* result){
    if(pfx_table_elem_matches(node->data, asn, mask_len)){
        *result =  BGP_PFXV_STATE_VALID;
        return PFX_SUCCESS;
    }
    //TODO: optimize, walk tree instead of get_children(..)

    //check children
    unsigned int len = 0;
    lpfst_node** array = NULL;
    if(lpfst_get_children(node, &array, &len) == -1)
        return PFX_ERROR;

    for(unsigned int i = 0; i < len; i++){
        if(pfx_table_elem_matches(array[i]->data, asn, mask_len)){
            *result =  BGP_PFXV_STATE_VALID;
            free(array);
            return PFX_SUCCESS;
        }
    }
    free(array);
    *result = BGP_PFXV_STATE_INVALID;
    return PFX_SUCCESS;
}

int pfx_table_validate(struct pfx_table* pfx_table, const uint32_t asn, const ip_addr *prefix, const uint8_t mask_len, pfxv_state* result){
    pthread_rwlock_rdlock(&(pfx_table->lock));
    lpfst_node* root = pfx_table_get_root(pfx_table, prefix->ver);
    if(root == NULL){
        *result = BGP_PFXV_STATE_NOT_FOUND;
        pthread_rwlock_unlock(&pfx_table->lock);
        return PFX_SUCCESS;
    }

    unsigned int lvl = 0;
    lpfst_node* node = lpfst_lookup(root, prefix, mask_len, &lvl);

    if(node == NULL){
        *result = BGP_PFXV_STATE_NOT_FOUND;
        pthread_rwlock_unlock(&pfx_table->lock);
        return PFX_SUCCESS;
    }
    const int rtval = pfx_table_get_state(node, asn, mask_len, result);
    pthread_rwlock_unlock(&pfx_table->lock);
    return rtval;
}

int pfx_table_src_remove(struct pfx_table* pfx_table, const uintptr_t socket_id){
    for(unsigned int i = 0; i< 2; i++){
        lpfst_node** root = (i == 0 ? &(pfx_table->ipv4) : &(pfx_table->ipv6));
        if(*root != NULL){
            pthread_rwlock_rdlock(&(pfx_table->lock));
            int rtval = pfx_table_remove_id(pfx_table, root, *root, socket_id, 0);
            pthread_rwlock_unlock(&pfx_table->lock);
            if(rtval == PFX_ERROR)
                return PFX_ERROR;
        }
    }
    return PFX_SUCCESS;
}

int pfx_table_remove_id(pfx_table* pfx_table, lpfst_node** root, lpfst_node* node, const uintptr_t socket_id, const unsigned int level){
    bool check_node = true;

    while(check_node){ //data from removed node are replaced from data from child nodes (if children exists), same node must be checked again if it was replaced with previous child node data
        node_data* data = node->data;
        for(unsigned int i = 0; i < data->len; i++){
            while(data->len > i && data->ary[i].socket_id == socket_id){
                pfx_record record = { data->ary[i].asn, node->prefix, node->len, data->ary[i].max_len, data->ary[i].socket_id};
                if(pfx_table_del_elem(data, i) == PFX_ERROR){
                    return PFX_ERROR;
                }
				pfx_table_notify_clients(pfx_table, &record, false);
            }
        }
        if(data->len == 0){
            free(((node_data*) node->data));
            lpfst_node* rm_node = lpfst_remove(node, &(node->prefix), level);
            free(rm_node);
            if(rm_node == *root){
                *root = NULL;
                return PFX_SUCCESS;
            }
            else if(rm_node == node){
                return PFX_SUCCESS;
            }
        }
        else{
            check_node = false;
        }
    }

    if(node->lchild != NULL){
        if(pfx_table_remove_id(pfx_table, root, node->lchild, socket_id, level + 1) == PFX_ERROR)
            return PFX_ERROR;
    }
    if(node->rchild != NULL)
        return pfx_table_remove_id(pfx_table, root, node->rchild, socket_id, level + 1);
    return PFX_SUCCESS;
}
