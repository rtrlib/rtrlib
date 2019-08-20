/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef RTR_TRIE_PRIVATE
#define RTR_TRIE_PRIVATE

#include "rtrlib/lib/ip_private.h"

#include <inttypes.h>

/**
 * @brief trie_node
 * @param prefix
 * @param rchild
 * @param lchild
 * @param parent
 * @param data
 * @param len number of elements in data array
 */
struct trie_node {
	struct lrtr_ip_addr prefix;
	struct trie_node *rchild;
	struct trie_node *lchild;
	struct trie_node *parent;
	void *data;
	uint8_t len;
};

/**
 * @brief Inserts new_node in the tree.
 * @param[in] root Root node of the tree for the inserting process.
 * @param[in] new_node Node that will be inserted.
 * @param[in] level Level of the the root node in the tree.
 */
void trie_insert(struct trie_node *root, struct trie_node *new_node, const unsigned int level);

/**
 * @brief Searches for a matching node matching the passed ip
 *	  prefix and prefix length. If multiple matching nodes exist, the one
 *	  with the shortest prefix is returned.
 * @param[in] root_node Node were the lookup process starts.
 * @param[in] lrtr_ip_addr IP-Prefix.
 * @param[in] mask_len Length of the network mask of the prefix.
 * @param[in,out] level of the the node root in the tree. Is set to the level of
 *		  the node that is returned.
 * @returns The trie_node with the short prefix in the tree matching the
 *	    passed ip prefix and prefix length.
 * @returns NULL if no node that matches the passed prefix and prefix length
 *	    could be found.
 */
struct trie_node *trie_lookup(const struct trie_node *root_node, const struct lrtr_ip_addr *prefix,
			      const uint8_t mask_len, unsigned int *level);

/**
 * @brief Search for a node with the same prefix and prefix length.
 * @param[in] root_node Node were the lookup process starts.
 * @param[in] lrtr_ip_addr IP-Prefix.
 * @param[in] mask_len Length of the network mask of the prefix.
 * @param[in,out] level of the the node root in the tree. Is set to the level of
 *		  the node that is returned.
 * @param[in] found Is true if a node which matches could be found else found is
 *	            set to false.
 * @return A node which matches the passed parameter (found==true).
 * @return The parent of the node where the lookup operation
 *	   stopped (found==false).
 * @return NULL if root_node is NULL.
 */
struct trie_node *trie_lookup_exact(struct trie_node *root_node, const struct lrtr_ip_addr *prefix,
				    const uint8_t mask_len, unsigned int *level, bool *found);

/**
 * @brief Removes the node with the passed IP prefix and mask_len from the tree.
 * @param[in] root Node were the inserting process starts.
 * @param[in] prefix Prefix that will removed from the tree.
 * @param[in] mask_len Length of the network mask of the prefix.
 * @param[in] level Level of the root node in the tree.
 * @returns Node that was removed from the tree. The caller has to free it.
 * @returns NULL If the Prefix could'nt be found in the tree.
 */
struct trie_node *trie_remove(struct trie_node *root_node, const struct lrtr_ip_addr *prefix, const uint8_t mask_len,
			      const unsigned int level);

/**
 * @brief Detects if a node is a leaf in the tree.
 * @param[in] node
 * @returns true if node is a leaf.
 * @returns false if node isn't a leaf.
 */
bool trie_is_leaf(const struct trie_node *node);

int trie_get_children(const struct trie_node *root_node, struct trie_node ***array, unsigned int *len);
#endif
