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

#include <stdlib.h>
#include "rtrlib/pfx/lpfst/lpfst.h"

void lpfst_insert(lpfst_node* root_node, lpfst_node* new_node, const unsigned int level){
    if(new_node->len > root_node->len)
    {
        //switch node data
        lpfst_node tmp;
        tmp.prefix = root_node->prefix;
        tmp.len = root_node->len;
        tmp.data = root_node->data;
        root_node->prefix = new_node->prefix;
        root_node->len = new_node->len;
        root_node->data = new_node->data;
        new_node->prefix = tmp.prefix;
        new_node->len = tmp.len;
        new_node->data = tmp.data;
    }
    if(ip_addr_is_zero(ip_addr_get_bits(&(new_node->prefix), level, 1))){
        if(root_node->lchild == NULL){
            root_node->lchild = new_node;
            new_node->parent = root_node;
        }
        else{
            lpfst_insert(root_node->lchild, new_node, level+1);
        }
    }
    else{
        if(root_node->rchild == NULL){
            root_node->rchild = new_node;
            new_node->parent = root_node;
        }
        else{
            lpfst_insert(root_node->rchild, new_node, level+1);
        }
    }
}

//returns node with most matching prefix bits
lpfst_node* lpfst_lookup(const lpfst_node* root_node, const ip_addr* prefix, const uint8_t mask_len, unsigned int* level){
    while(root_node != NULL)
    {
        //if the first prefix_len bits from root_node->prefix match prefix, return root_node
        if(root_node->len <= mask_len && ip_addr_equal(ip_addr_get_bits(&(root_node->prefix), 0, root_node->len), ip_addr_get_bits(prefix, 0, root_node->len)))
            return (lpfst_node*) root_node;

        if(ip_addr_is_zero(ip_addr_get_bits(prefix, *level, 1)))
            root_node = root_node->lchild;
        else
            root_node = root_node->rchild;

        (*level)++;
    }
    return NULL;
}

lpfst_node* lpfst_lookup_exact(lpfst_node* root_node, const ip_addr* prefix, const uint8_t mask_len, unsigned int* level, bool* found){
    *found = false;
    while(root_node != NULL)
    {
        if(*level > 0 && root_node->len < mask_len){
                (*level)--;
            return root_node->parent;
        }
        if(root_node->len == mask_len && ip_addr_equal(root_node->prefix, *prefix)){
            *found = true;
            return (lpfst_node*) root_node;
        }

        if(ip_addr_is_zero(ip_addr_get_bits(prefix, *level, 1))){
            if(root_node->lchild == NULL){
                return root_node;
            }
            else{
                root_node = root_node->lchild;
            }
        }
        else{
            if(root_node->rchild == NULL){
                return root_node;
            }
            else{
                root_node = root_node->rchild;
            }
        }

        (*level)++;
    }
    return NULL;
}


//returns node that isnt used anymore in the tree
lpfst_node* lpfst_remove(lpfst_node* root_node, const ip_addr* prefix, const unsigned int level){
    if(ip_addr_equal(root_node->prefix, *prefix)){
        if(lpfst_is_leaf(root_node)){
            if(level != 0){
                if(root_node->parent->lchild == root_node)
                    root_node->parent->lchild = NULL;
                else if(root_node->parent->rchild == root_node)
                    root_node->parent->rchild = NULL;
            }
            return root_node;
        }
        else{
            if(root_node->lchild != NULL && (root_node->rchild == NULL || (root_node->lchild->len > root_node->rchild->len))){
                    root_node->prefix = root_node->lchild->prefix;
                    root_node->len = root_node->lchild->len;
                    void* tmp = root_node->data;
                    root_node->data = root_node->lchild->data;
                    root_node->lchild->data = tmp;
                    return lpfst_remove(root_node->lchild, &(root_node->lchild->prefix), level+1);
            }
            else{
                root_node->prefix = root_node->rchild->prefix;
                root_node->len = root_node->rchild->len;
                void* tmp = root_node->data;
                tmp = root_node->data;
                root_node->data = root_node->rchild->data;
                root_node->rchild->data = tmp;
                return lpfst_remove(root_node->rchild, &(root_node->rchild->prefix), level+1);
            }
        }
    }
    if(ip_addr_is_zero(ip_addr_get_bits(prefix, level, 1)))
        return lpfst_remove(root_node->lchild, prefix, level+1);
    else
        return lpfst_remove(root_node->rchild, prefix, level+1);
}

int lpfst_get_children(const lpfst_node* root_node, lpfst_node*** array, unsigned int* len){
    if(root_node->lchild != NULL){
        *len += 1;
        *array = realloc(*array, (*len) * sizeof(lpfst_node*));
        if(*array == NULL)
            return -1;
        (*array)[(*len) - 1] = root_node->lchild;
        if(lpfst_get_children(root_node->lchild, array, len) == -1)
            return -1;
    }

    if(root_node->rchild != NULL){
        *len += 1;
        *array = realloc(*array, (*len) * sizeof(lpfst_node*));
        if(*array == NULL)
            return -1;
        (*array)[(*len) - 1] = root_node->rchild;
        if(lpfst_get_children(root_node->rchild, array, len) == -1)
            return -1;
    }
    return 0;
}

inline int lpfst_is_leaf(const lpfst_node* node){
    return node->lchild == NULL && node->rchild == NULL;
}
