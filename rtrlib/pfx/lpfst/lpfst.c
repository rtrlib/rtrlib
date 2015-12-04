/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
*/

#include <stdlib.h>
#include "rtrlib/pfx/lpfst/lpfst.h"

void lpfst_insert(struct lpfst_node *root_node, struct lpfst_node *new_node, const unsigned int level)
{
    if(new_node->len > root_node->len) {
        //switch node data
        struct lpfst_node tmp;
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
    if(lrtr_ip_addr_is_zero(lrtr_ip_addr_get_bits(&(new_node->prefix), level, 1))) {
        if(root_node->lchild == NULL) {
            root_node->lchild = new_node;
            new_node->parent = root_node;
        } else
            lpfst_insert(root_node->lchild, new_node, level+1);
    } else {
        if(root_node->rchild == NULL) {
            root_node->rchild = new_node;
            new_node->parent = root_node;
        } else
            lpfst_insert(root_node->rchild, new_node, level+1);
    }
}

struct lpfst_node *lpfst_lookup(const struct lpfst_node *root_node, const struct lrtr_ip_addr *prefix, const uint8_t mask_len, unsigned int *level)
{
    while(root_node != NULL) {
        if(root_node->len <= mask_len && lrtr_ip_addr_equal(lrtr_ip_addr_get_bits(&(root_node->prefix), 0, root_node->len), lrtr_ip_addr_get_bits(prefix, 0, root_node->len)))
            return (struct lpfst_node *) root_node;

        if(lrtr_ip_addr_is_zero(lrtr_ip_addr_get_bits(prefix, *level, 1)))
            root_node = root_node->lchild;
        else
            root_node = root_node->rchild;

        (*level)++;
    }
    return NULL;
}

struct lpfst_node *lpfst_lookup_exact(struct lpfst_node *root_node, const struct lrtr_ip_addr *prefix, const uint8_t mask_len, unsigned int *level, bool *found)
{
    *found = false;
    while(root_node != NULL) {
        if(*level > 0 && root_node->len < mask_len) {
            (*level)--;
            return root_node->parent;
        }
        if(root_node->len == mask_len && lrtr_ip_addr_equal(root_node->prefix, *prefix)) {
            *found = true;
            return (struct lpfst_node *) root_node;
        }

        if(lrtr_ip_addr_is_zero(lrtr_ip_addr_get_bits(prefix, *level, 1))) {
            if(root_node->lchild == NULL)
                return root_node;
            else
                root_node = root_node->lchild;
        } else {
            if(root_node->rchild == NULL)
                return root_node;
            else
                root_node = root_node->rchild;
        }

        (*level)++;
    }
    return NULL;
}

//returns node that isnt used anymore in the tree
struct lpfst_node *lpfst_remove(struct lpfst_node *root_node, const struct lrtr_ip_addr *prefix, const uint8_t mask_len, const unsigned int level)
{
    if(root_node->len == mask_len && lrtr_ip_addr_equal(root_node->prefix, *prefix)) {
        if(lpfst_is_leaf(root_node)) {
            if(level != 0) {
                if(root_node->parent->lchild == root_node)
                    root_node->parent->lchild = NULL;
                else if(root_node->parent->rchild == root_node)
                    root_node->parent->rchild = NULL;
            }
            return root_node;
        } else {
            if(root_node->lchild != NULL && (root_node->rchild == NULL || (root_node->lchild->len > root_node->rchild->len))) {
                root_node->prefix = root_node->lchild->prefix;
                root_node->len = root_node->lchild->len;
                void *tmp = root_node->data;
                root_node->data = root_node->lchild->data;
                root_node->lchild->data = tmp;
                return lpfst_remove(root_node->lchild, &(root_node->lchild->prefix), root_node->lchild->len, level+1);
            } else {
                root_node->prefix = root_node->rchild->prefix;
                root_node->len = root_node->rchild->len;
                void *tmp = root_node->data;
                tmp = root_node->data;
                root_node->data = root_node->rchild->data;
                root_node->rchild->data = tmp;
                return lpfst_remove(root_node->rchild, &(root_node->rchild->prefix), root_node->rchild->len, level+1);
            }
        }
    }
    if(lrtr_ip_addr_is_zero(lrtr_ip_addr_get_bits(prefix, level, 1))) {
        if(root_node->lchild == NULL)
            return NULL;
        return lpfst_remove(root_node->lchild, prefix, mask_len, level+1);
    } else {
        if(root_node->rchild == NULL)
            return NULL;
        return lpfst_remove(root_node->rchild, prefix, mask_len, level+1);
    }
}

int lpfst_get_children(const struct lpfst_node *root_node, struct lpfst_node ***array, unsigned int *len)
{
    if(root_node->lchild != NULL) {
        *len += 1;
        *array = realloc(*array, *len * sizeof(struct lpfst_node *));
        if(*array == NULL)
            return -1;
        (*array)[*len - 1] = root_node->lchild;
        if(lpfst_get_children(root_node->lchild, array, len) == -1)
            return -1;
    }

    if(root_node->rchild != NULL) {
        *len += 1;
        *array = realloc(*array, *len * sizeof(struct lpfst_node *));
        if(*array == NULL)
            return -1;
        (*array)[*len - 1] = root_node->rchild;
        if(lpfst_get_children(root_node->rchild, array, len) == -1)
            return -1;
    }
    return 0;
}

inline int lpfst_is_leaf(const struct lpfst_node *node)
{
    return node->lchild == NULL && node->rchild == NULL;
}
