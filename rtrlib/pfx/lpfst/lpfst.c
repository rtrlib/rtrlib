/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include <stdlib.h>
#include <assert.h>
#include "rtrlib/pfx/lpfst/lpfst.h"

static void swap_nodes(struct lpfst_node *a, struct lpfst_node *b)
{
	struct lpfst_node tmp;

	tmp.prefix = a->prefix;
	tmp.len = a->len;
	tmp.data = a->data;

	a->prefix = b->prefix;
	a->len = b->len;
	a->data = b->data;

	b->prefix = tmp.prefix;
	b->len = tmp.len;
	b->data = tmp.data;
}

enum child_node_rel {
	LEFT,
	RIGHT
};

static void add_child_node(struct lpfst_node *parent, struct lpfst_node *child,
			   enum child_node_rel rel)
{
	assert(rel == LEFT || rel == RIGHT);

	if (rel == LEFT)
		parent->lchild = child;
	else
		parent->rchild = child;

	child->parent = parent;
}

static inline bool is_left_child(const struct lrtr_ip_addr *addr,
				 unsigned int lvl)
{
	/* A node must be inserted as left child if bit <lvl> of the IP address
	 * is 0 otherwise as right child
	 */
	return lrtr_ip_addr_is_zero(lrtr_ip_addr_get_bits(addr, lvl, 1));
}

void lpfst_insert(struct lpfst_node *root, struct lpfst_node *new,
		  const unsigned int lvl)
{
	if (new->len > root->len)
		swap_nodes(root, new);

	if (is_left_child(&new->prefix, lvl)) {
		if (!root->lchild)
			return add_child_node(root, new, LEFT);

		return lpfst_insert(root->lchild, new, lvl + 1);
	}

	if (!root->rchild)
		return add_child_node(root, new, RIGHT);

	lpfst_insert(root->rchild, new, lvl + 1);
}

struct lpfst_node *lpfst_lookup(const struct lpfst_node *root,
				const struct lrtr_ip_addr *prefix,
				const uint8_t mask_len, unsigned int *lvl)
{
	while (root) {
		if (root->len <= mask_len &&
		    lrtr_ip_addr_equal(lrtr_ip_addr_get_bits(&root->prefix, 0,
							     root->len),
				       lrtr_ip_addr_get_bits(prefix, 0,
							     root->len)))
			return (struct lpfst_node *)root;

		if ((is_left_child(prefix, *lvl) && root->lchild) ||
		    (!is_left_child(prefix, *lvl) && !root->rchild)) {
			root = root->lchild;
		} else {
			root = root->rchild;
		}

		(*lvl)++;
	}
	return NULL;
}

struct lpfst_node *lpfst_lookup_fallback(const struct lpfst_node *first,
					 const struct lpfst_node *second,
					 const struct lrtr_ip_addr *prefix,
					 const uint8_t mask_len,
					 unsigned int *lvl)
{
	unsigned int tmp_lvl = *lvl;
	struct lpfst_node *tmp_node;

	tmp_node = lpfst_lookup(first, prefix, mask_len, lvl);
	if (tmp_node)
		return (struct lpfst_node *)tmp_node;
	*lvl = tmp_lvl;
	return lpfst_lookup(second, prefix, mask_len, lvl);
}

struct lpfst_node *lpfst_lookup_exact(struct lpfst_node *root_node,
				      const struct lrtr_ip_addr *prefix,
				      const uint8_t mask_len,
				      unsigned int *lvl, bool *found)
{
	*found = false;

	while (root_node) {
		if (*lvl > 0 && root_node->len < mask_len) {
			(*lvl)--;
			return root_node->parent;
		}

		if (root_node->len == mask_len &&
		    lrtr_ip_addr_equal(root_node->prefix, *prefix)) {
			*found = true;
			return root_node;
		}

		if (is_left_child(prefix, *lvl)) {
			if (!root_node->lchild)
				return root_node;
			root_node = root_node->lchild;
		} else {
			if (!root_node->rchild)
				return root_node;
			root_node = root_node->rchild;
		}

		(*lvl)++;
	}
	return NULL;
}

static void deref_node(struct lpfst_node *n)
{
	if (!n->parent)
		return;

	if (n->parent->lchild == n) {
		n->parent->lchild = NULL;
		return;
	}

	n->parent->rchild = NULL;
}

static inline bool prefix_is_same(const struct lpfst_node *n,
				  const struct lrtr_ip_addr *p,
				  uint8_t mask_len)
{
	return n->len == mask_len && lrtr_ip_addr_equal(n->prefix, *p);
}

static void replace_node_data(struct lpfst_node *a, struct lpfst_node *b)
{
	a->prefix = b->prefix;
	a->len = b->len;
	a->data = b->data;
}

struct lpfst_node *lpfst_remove(struct lpfst_node *root,
				const struct lrtr_ip_addr *prefix,
				const uint8_t mask_len,
				const unsigned int lvl)
{
	/* If the node has no children we can simply remove it
	 * If the node has children, we swap the node with the child that
	 * has the bigger prefix length and drop the child.
	 */
	if (prefix_is_same(root, prefix, mask_len)) {
		void *tmp;

		if (lpfst_is_leaf(root)) {
			deref_node(root);
			return root;
		}

		/* swap with the left child and drop the child */
		if (root->lchild && (!root->rchild ||
				     root->lchild->len > root->rchild->len)) {
			tmp = root->data;
			replace_node_data(root, root->lchild);
			root->lchild->data = tmp;

			return lpfst_remove(root->lchild,
					    &root->lchild->prefix,
					    root->lchild->len, lvl + 1);
		}

		/* swap with the right child and drop the child */
		tmp = root->data;
		replace_node_data(root, root->rchild);
		root->rchild->data = tmp;

		return lpfst_remove(root->rchild,
				    &root->rchild->prefix,
				    root->rchild->len, lvl + 1);
	}

	if (is_left_child(prefix, lvl)) {
		if (!root->lchild)
			return NULL;
		return lpfst_remove(root->lchild, prefix, mask_len, lvl + 1);
	}

	if (!root->rchild)
		return NULL;
	return lpfst_remove(root->rchild, prefix, mask_len, lvl + 1);
}

static int append_node_to_array(struct lpfst_node ***ary,
				unsigned int *len,
				struct lpfst_node *n)
{
	struct lpfst_node **new;

	new = realloc(*ary, *len * sizeof(n));
	if (!new)
		return -1;

	*ary = new;
	(*ary)[*len - 1] = n;
	return 0;
}

int lpfst_get_children(const struct lpfst_node *root_node,
		       struct lpfst_node ***array, unsigned int *len)
{
	if (root_node->lchild) {
		*len += 1;
		if (append_node_to_array(array, len, root_node->lchild))
			goto err;

		if (lpfst_get_children(root_node->lchild, array, len) == -1)
			goto err;
	}

	if (root_node->rchild) {
		*len += 1;
		if (append_node_to_array(array, len, root_node->rchild))
			goto err;

		if (lpfst_get_children(root_node->rchild, array, len) == -1)
			goto err;
	}

	return 0;

err:
	free(*array);
	return -1;
}

inline bool lpfst_is_leaf(const struct lpfst_node *node)
{
	return !node->lchild && !node->rchild;
}
