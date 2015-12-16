/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
*/

#ifndef RTR_LPFST
#define RTR_LPFST
#include <inttypes.h>
#include "rtrlib/lib/ip.h"

/**
 * @brief lpfst_node
 * @param prefix
 * @param len number of elements in data array
 * @param rchild
 * @param lchild
 * @param parent
 * @param data
 */
struct lpfst_node {
    struct lrtr_ip_addr prefix;
    uint8_t len;
    struct lpfst_node *rchild;
    struct lpfst_node *lchild;
    struct lpfst_node *parent;
    void *data;
};

/**
 * @brief Inserts new_node in the tree.
 * @param[in] root Root node of the tree for the inserting process.
 * @param[in] new_node Node that will be inserted.
 * @param[in] level Level of the the root node in the tree.
 */
void lpfst_insert(struct lpfst_node *root, struct lpfst_node *new_node, const unsigned int level);

/**
 * @brief Searches for the node with the longest prefix, matching the passed ip prefix and prefix length.
 * @param[in] root_node Node were the lookup process starts.
 * @param[in] lrtr_ip_addr IP-Prefix.
 * @param[in] mask_len Length of the network mask of the prefix.
 * @param[in,out] level of the the node root in the tree. Is set to the level of the node that is returned.
 * @returns A The lpfst_node with the longest prefix in the tree matching the passed ip prefix and prefix length.
 * @returns NULL if no node that matches the passed prefix and prefix length could be found.
 */
struct lpfst_node *lpfst_lookup(const struct lpfst_node *root_node, const struct lrtr_ip_addr *prefix, const uint8_t mask_len,  unsigned int *level);

/**
 * @brief Search for a node with the same prefix and prefix length.
 * @param[in] root_node Node were the lookup process starts.
 * @param[in] lrtr_ip_addr IP-Prefix.
 * @param[in] mask_len Length of the network mask of the prefix.
 * @param[in,out] level of the the node root in the tree. Is set to the level of the node that is returned.
 * @param[in] found Is true if a node which matches could be found else found is set to false.
 * @return A node which matches the passed parameter (found==true).
 * @return the parent of the node where the lookup operation stopped (found==false).
 * @return NULL if root_node is NULL.
*/
struct lpfst_node *lpfst_lookup_exact(struct lpfst_node *root_node, const struct lrtr_ip_addr *prefix, const uint8_t mask_len, unsigned int *level, bool *found);

/**
 * @brief Removes the node with the passed IP prefix and mask_len from the tree.
 * @param[in] root Node were the inserting process starts.
 * @param[in] prefix Prefix that will removed from the tree.
 * @param[in] mask_len Length of the network mask of the prefix.
 * @param[in] level Level of the root node in the tree.
 * @returns Node that was removed from the tree.
 * @returns NULL If the Prefix could'nt be found in the tree.
 */
struct lpfst_node *lpfst_remove(struct lpfst_node *root_node, const struct lrtr_ip_addr *prefix, const uint8_t mask_len, const unsigned int level);

/**
 * @brief Detects if a node is a leaf in tree.
 * @param[in] node
 * @returns true if node is a leaf:
 * @returns false if node isn't a leaf.
 */
int lpfst_is_leaf(const struct lpfst_node *node);

int lpfst_get_children(const struct lpfst_node *root_node, struct lpfst_node ***array, unsigned int *len);
#endif
