/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "trie-pfx.h"

#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/lib/ip_private.h"
#include "rtrlib/pfx/pfx_private.h"
#include "rtrlib/pfx/trie/trie_private.h"
#include "rtrlib/rtrlib_export_private.h"

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

struct data_elem {
	uint32_t asn;
	uint8_t max_len;
	const struct rtr_socket *socket;
};

struct node_data {
	unsigned int len;
	struct data_elem *ary;
};

struct copy_cb_args {
	struct pfx_table *pfx_table;
	const struct rtr_socket *socket;
	bool error;
};

struct notify_diff_cb_args {
	struct pfx_table *old_table;
	struct pfx_table *new_table;
	const struct rtr_socket *socket;
	pfx_update_fp pfx_update_fp;
	bool added;
};

static struct trie_node *pfx_table_get_root(const struct pfx_table *pfx_table, const enum lrtr_ip_version ver);
static int pfx_table_del_elem(struct node_data *data, const unsigned int index);
static int pfx_table_create_node(struct trie_node **node, const struct pfx_record *record);
static int pfx_table_append_elem(struct node_data *data, const struct pfx_record *record);
static struct data_elem *pfx_table_find_elem(const struct node_data *data, const struct pfx_record *record,
					     unsigned int *index);
static bool pfx_table_elem_matches(struct node_data *data, const uint32_t asn, const uint8_t prefix_len);
static void pfx_table_notify_clients(struct pfx_table *pfx_table, const struct pfx_record *record, const bool added);
static int pfx_table_remove_id(struct pfx_table *pfx_table, struct trie_node **root, struct trie_node *node,
			       const struct rtr_socket *socket, const unsigned int level);
static int pfx_table_node2pfx_record(struct trie_node *node, struct pfx_record records[], const unsigned int ary_len);
static void pfx_table_free_reason(struct pfx_record **reason, unsigned int *reason_len);

void pfx_table_notify_clients(struct pfx_table *pfx_table, const struct pfx_record *record, const bool added)
{
	if (pfx_table->update_fp != NULL)
		pfx_table->update_fp(pfx_table, *record, added);
}

RTRLIB_EXPORT void pfx_table_init(struct pfx_table *pfx_table, pfx_update_fp update_fp)
{
	pfx_table->ipv4 = NULL;
	pfx_table->ipv6 = NULL;
	pfx_table->update_fp = update_fp;
	pthread_rwlock_init(&(pfx_table->lock), NULL);
}

void pfx_table_free_without_notify(struct pfx_table *pfx_table)
{
	pfx_table->update_fp = NULL;
	pfx_table_free(pfx_table);
}

RTRLIB_EXPORT void pfx_table_free(struct pfx_table *pfx_table)
{
	for (int i = 0; i < 2; i++) {
		struct trie_node *root = (i == 0 ? pfx_table->ipv4 : pfx_table->ipv6);
		if (root != NULL) {
			struct trie_node *rm_node;

			pthread_rwlock_wrlock(&(pfx_table->lock));
			do {
				struct node_data *data = (struct node_data *)(root->data);
				for (unsigned int j = 0; j < data->len; j++) {
					struct pfx_record record = {data->ary[j].asn, (root->prefix), root->len,
								    data->ary[j].max_len, data->ary[j].socket};
					pfx_table_notify_clients(pfx_table, &record, false);
				}
				rm_node = (trie_remove(root, &(root->prefix), root->len, 0));
				assert(rm_node != NULL);
				lrtr_free(((struct node_data *)rm_node->data)->ary);
				lrtr_free(rm_node->data);
				lrtr_free(rm_node);
			} while (rm_node != root);
			if (i == 0)
				pfx_table->ipv4 = NULL;
			else
				pfx_table->ipv6 = NULL;

			pthread_rwlock_unlock(&(pfx_table->lock));
		}
	}
	pthread_rwlock_destroy(&(pfx_table->lock));
}

int pfx_table_append_elem(struct node_data *data, const struct pfx_record *record)
{
	struct data_elem *tmp = lrtr_realloc(data->ary, sizeof(struct data_elem) * ((data->len) + 1));
	if (tmp == NULL)
		return PFX_ERROR;
	data->len++;
	data->ary = tmp;
	data->ary[data->len - 1].asn = record->asn;
	data->ary[data->len - 1].max_len = record->max_len;
	data->ary[data->len - 1].socket = record->socket;
	return PFX_SUCCESS;
}

int pfx_table_create_node(struct trie_node **node, const struct pfx_record *record)
{
	int err;

	*node = lrtr_malloc(sizeof(struct trie_node));
	if (*node == NULL)
		return PFX_ERROR;

	(*node)->prefix = record->prefix;
	(*node)->len = record->min_len;
	(*node)->lchild = NULL;
	(*node)->rchild = NULL;
	(*node)->parent = NULL;

	(*node)->data = lrtr_malloc(sizeof(struct node_data));
	if ((*node)->data == NULL) {
		err = PFX_ERROR;
		goto free_node;
	}

	((struct node_data *)(*node)->data)->len = 0;
	((struct node_data *)(*node)->data)->ary = NULL;

	err = pfx_table_append_elem(((struct node_data *)(*node)->data), record);
	if (err)
		goto free_node_data;

	return PFX_SUCCESS;

free_node_data:
	lrtr_free((*node)->data);
free_node:
	lrtr_free(*node);

	return err;
}

struct data_elem *pfx_table_find_elem(const struct node_data *data, const struct pfx_record *record,
				      unsigned int *index)
{
	for (unsigned int i = 0; i < data->len; i++) {
		if (data->ary[i].asn == record->asn && data->ary[i].max_len == record->max_len &&
		    data->ary[i].socket == record->socket) {
			if (index != NULL)
				*index = i;
			return &(data->ary[i]);
		}
	}
	return NULL;
}

// returns pfx_table->ipv4 if record version is LRTR_IPV4 else pfx_table->ipv6
inline struct trie_node *pfx_table_get_root(const struct pfx_table *pfx_table, const enum lrtr_ip_version ver)
{
	return ver == LRTR_IPV4 ? pfx_table->ipv4 : pfx_table->ipv6;
}

int pfx_table_del_elem(struct node_data *data, const unsigned int index)
{
	struct data_elem *tmp;
	struct data_elem deleted_elem = data->ary[index];

	// if index is not the last elem in the list, move all other elems backwards in the array
	if (index != data->len - 1) {
		for (unsigned int i = index; i < data->len - 1; i++) {
			data->ary[i] = data->ary[i + 1];
		}
	}

	data->len--;
	if (!data->len) {
		lrtr_free(data->ary);
		data->ary = NULL;
		return PFX_SUCCESS;
	}

	tmp = lrtr_realloc(data->ary, sizeof(struct data_elem) * data->len);
	if (!tmp) {
		data->ary[data->len] = deleted_elem;
		data->len++;
		return PFX_ERROR;
	}

	data->ary = tmp;

	return PFX_SUCCESS;
}

RTRLIB_EXPORT int pfx_table_add(struct pfx_table *pfx_table, const struct pfx_record *record)
{
	pthread_rwlock_wrlock(&(pfx_table->lock));

	struct trie_node *root = pfx_table_get_root(pfx_table, record->prefix.ver);
	unsigned int lvl = 0;
	if (root != NULL) {
		bool found;
		struct trie_node *node = trie_lookup_exact(root, &(record->prefix), record->min_len, &lvl, &found);
		if (found) { // node with prefix exists
			if (pfx_table_find_elem(node->data, record, NULL) != NULL) {
				pthread_rwlock_unlock(&pfx_table->lock);
				return PFX_DUPLICATE_RECORD;
			}
			// append record to note_data array
			int rtval = pfx_table_append_elem(node->data, record);
			pthread_rwlock_unlock(&pfx_table->lock);
			if (rtval == PFX_SUCCESS)
				pfx_table_notify_clients(pfx_table, record, true);
			return rtval;
		} else {
			// no node with same prefix and prefix_len found
			struct trie_node *new_node = NULL;
			if (pfx_table_create_node(&new_node, record) == PFX_ERROR) {
				pthread_rwlock_unlock(&pfx_table->lock);
				return PFX_ERROR;
			}
			trie_insert(node, new_node, lvl);
			pthread_rwlock_unlock(&pfx_table->lock);
			pfx_table_notify_clients(pfx_table, record, true);
			return PFX_SUCCESS;
		}
	} else {
		// tree is empty, record will be the root_node
		struct trie_node *new_node = NULL;
		if (pfx_table_create_node(&new_node, record) == PFX_ERROR) {
			pthread_rwlock_unlock(&pfx_table->lock);
			return PFX_ERROR;
		}
		if (record->prefix.ver == LRTR_IPV4)
			pfx_table->ipv4 = new_node;
		else
			pfx_table->ipv6 = new_node;

		pthread_rwlock_unlock(&pfx_table->lock);
		pfx_table_notify_clients(pfx_table, record, true);
	}
	return PFX_SUCCESS;
}

RTRLIB_EXPORT int pfx_table_remove(struct pfx_table *pfx_table, const struct pfx_record *record)
{
	pthread_rwlock_wrlock(&(pfx_table->lock));
	struct trie_node *root = pfx_table_get_root(pfx_table, record->prefix.ver);

	unsigned int lvl = 0; // tree depth were node was found
	bool found;
	struct trie_node *node = trie_lookup_exact(root, &(record->prefix), record->min_len, &lvl, &found);
	if (!found) {
		pthread_rwlock_unlock(&pfx_table->lock);
		return PFX_RECORD_NOT_FOUND;
	}

	unsigned int index;
	struct data_elem *elem = pfx_table_find_elem(node->data, record, &index);
	if (elem == NULL) {
		pthread_rwlock_unlock(&pfx_table->lock);
		return PFX_RECORD_NOT_FOUND;
	}

	struct node_data *ndata = (struct node_data *)node->data;

	if (pfx_table_del_elem(ndata, index) == PFX_ERROR) {
		pthread_rwlock_unlock(&pfx_table->lock);
		return PFX_ERROR;
	}

	if (ndata->len == 0) {
		node = trie_remove(node, &(record->prefix), record->min_len, lvl);
		assert(node != NULL);

		if (node == root) {
			if (record->prefix.ver == LRTR_IPV4)
				pfx_table->ipv4 = NULL;
			else
				pfx_table->ipv6 = NULL;
		}
		assert(((struct node_data *)node->data)->len == 0);
		lrtr_free(node->data);
		lrtr_free(node);
	}
	pthread_rwlock_unlock(&pfx_table->lock);

	pfx_table_notify_clients(pfx_table, record, false);

	return PFX_SUCCESS;
}

bool pfx_table_elem_matches(struct node_data *data, const uint32_t asn, const uint8_t prefix_len)
{
	for (unsigned int i = 0; i < data->len; i++) {
		if (data->ary[i].asn != 0 && data->ary[i].asn == asn && prefix_len <= data->ary[i].max_len)
			return true;
	}
	return false;
}

int pfx_table_node2pfx_record(struct trie_node *node, struct pfx_record *records, const unsigned int ary_len)
{
	struct node_data *data = node->data;
	if (ary_len < data->len)
		return PFX_ERROR;

	for (unsigned int i = 0; i < data->len; i++) {
		records[i].asn = data->ary[i].asn;
		records[i].prefix = node->prefix;
		records[i].min_len = node->len;
		records[i].max_len = data->ary[i].max_len;
		records[i].socket = data->ary[i].socket;
	}
	return data->len;
}

inline void pfx_table_free_reason(struct pfx_record **reason, unsigned int *reason_len)
{
	if (reason != NULL) {
		lrtr_free(*reason);
		*reason = NULL;
	}
	if (reason_len != NULL)
		*reason_len = 0;
}

RTRLIB_EXPORT int pfx_table_validate_r(struct pfx_table *pfx_table, struct pfx_record **reason,
				       unsigned int *reason_len, const uint32_t asn, const struct lrtr_ip_addr *prefix,
				       const uint8_t prefix_len, enum pfxv_state *result)
{
	// assert(reason_len == NULL || *reason_len  == 0);
	// assert(reason == NULL || *reason == NULL);

	pthread_rwlock_rdlock(&(pfx_table->lock));
	struct trie_node *root = pfx_table_get_root(pfx_table, prefix->ver);
	if (root == NULL) {
		pthread_rwlock_unlock(&pfx_table->lock);
		*result = BGP_PFXV_STATE_NOT_FOUND;
		pfx_table_free_reason(reason, reason_len);
		return PFX_SUCCESS;
	}

	unsigned int lvl = 0;
	struct trie_node *node = trie_lookup(root, prefix, prefix_len, &lvl);
	if (node == NULL) {
		pthread_rwlock_unlock(&pfx_table->lock);
		*result = BGP_PFXV_STATE_NOT_FOUND;
		pfx_table_free_reason(reason, reason_len);
		return PFX_SUCCESS;
	}

	if (reason_len != NULL && reason != NULL) {
		*reason_len = ((struct node_data *)node->data)->len;
		*reason = lrtr_realloc(*reason, *reason_len * sizeof(struct pfx_record));
		if (*reason == NULL) {
			pthread_rwlock_unlock(&pfx_table->lock);
			pfx_table_free_reason(reason, reason_len);
			return PFX_ERROR;
		}
		if (pfx_table_node2pfx_record(node, *reason, *reason_len) == PFX_ERROR) {
			pthread_rwlock_unlock(&pfx_table->lock);
			pfx_table_free_reason(reason, reason_len);
			return PFX_ERROR;
		}
	}

	while (!pfx_table_elem_matches(node->data, asn, prefix_len)) {
		if (lrtr_ip_addr_is_zero(lrtr_ip_addr_get_bits(
			    prefix, lvl++,
			    1))) //post-incr lvl, trie_lookup is performed on child_nodes => parent lvl + 1
			node = trie_lookup(node->lchild, prefix, prefix_len, &lvl);
		else
			node = trie_lookup(node->rchild, prefix, prefix_len, &lvl);

		if (node == NULL) {
			pthread_rwlock_unlock(&pfx_table->lock);
			*result = BGP_PFXV_STATE_INVALID;
			return PFX_SUCCESS;
		}

		if (reason_len != NULL && reason != NULL) {
			unsigned int r_len_old = *reason_len;
			*reason_len += ((struct node_data *)node->data)->len;
			*reason = lrtr_realloc(*reason, *reason_len * sizeof(struct pfx_record));
			struct pfx_record *start = *reason + r_len_old;
			if (*reason == NULL) {
				pthread_rwlock_unlock(&pfx_table->lock);
				pfx_table_free_reason(reason, reason_len);
				return PFX_ERROR;
			}
			if (pfx_table_node2pfx_record(node, start, ((struct node_data *)node->data)->len) ==
			    PFX_ERROR) {
				pthread_rwlock_unlock(&pfx_table->lock);
				pfx_table_free_reason(reason, reason_len);
				return PFX_ERROR;
			}
		}
	}

	pthread_rwlock_unlock(&pfx_table->lock);
	*result = BGP_PFXV_STATE_VALID;
	return PFX_SUCCESS;
}

RTRLIB_EXPORT int pfx_table_validate(struct pfx_table *pfx_table, const uint32_t asn, const struct lrtr_ip_addr *prefix,
				     const uint8_t prefix_len, enum pfxv_state *result)
{
	return pfx_table_validate_r(pfx_table, NULL, NULL, asn, prefix, prefix_len, result);
}

RTRLIB_EXPORT int pfx_table_src_remove(struct pfx_table *pfx_table, const struct rtr_socket *socket)
{
	for (unsigned int i = 0; i < 2; i++) {
		struct trie_node **root = (i == 0 ? &(pfx_table->ipv4) : &(pfx_table->ipv6));
		pthread_rwlock_wrlock(&(pfx_table->lock));
		if (*root != NULL) {
			int rtval = pfx_table_remove_id(pfx_table, root, *root, socket, 0);
			if (rtval == PFX_ERROR) {
				pthread_rwlock_unlock(&pfx_table->lock);
				return PFX_ERROR;
			}
		}
		pthread_rwlock_unlock(&pfx_table->lock);
	}
	return PFX_SUCCESS;
}

int pfx_table_remove_id(struct pfx_table *pfx_table, struct trie_node **root, struct trie_node *node,
			const struct rtr_socket *socket, const unsigned int level)
{
	assert(node != NULL);
	assert(root != NULL);
	assert(*root != NULL);
	bool check_node = true;

	while (check_node) { // data from removed node are replaced from data from child nodes (if children exists), same node must be checked again if it was replaced with previous child node data
		struct node_data *data = node->data;
		for (unsigned int i = 0; i < data->len; i++) {
			while (data->len > i && data->ary[i].socket == socket) {
				struct pfx_record record = {data->ary[i].asn, node->prefix, node->len,
							    data->ary[i].max_len, data->ary[i].socket};
				if (pfx_table_del_elem(data, i) == PFX_ERROR) {
					return PFX_ERROR;
				}
				pfx_table_notify_clients(pfx_table, &record, false);
			}
		}
		if (data->len == 0) {
			struct trie_node *rm_node = trie_remove(node, &(node->prefix), node->len, level);
			assert(rm_node != NULL);
			assert(((struct node_data *)rm_node->data)->len == 0);
			lrtr_free(((struct node_data *)rm_node->data));
			lrtr_free(rm_node);

			if (rm_node == *root) {
				*root = NULL;
				return PFX_SUCCESS;
			} else if (rm_node == node)
				return PFX_SUCCESS;
		} else
			check_node = false;
	}

	if (node->lchild != NULL) {
		if (pfx_table_remove_id(pfx_table, root, node->lchild, socket, level + 1) == PFX_ERROR)
			return PFX_ERROR;
	}
	if (node->rchild != NULL)
		return pfx_table_remove_id(pfx_table, root, node->rchild, socket, level + 1);
	return PFX_SUCCESS;
}

static void pfx_table_for_each_rec(struct trie_node *n, pfx_for_each_fp fp, void *data)
{
	struct pfx_record pfxr;
	struct node_data *nd;

	assert(n != NULL);
	assert(fp != NULL);

	nd = (struct node_data *)n->data;
	assert(nd != NULL);

	if (n->lchild != NULL)
		pfx_table_for_each_rec(n->lchild, fp, data);

	for (unsigned int i = 0; i < nd->len; i++) {
		pfxr.asn = nd->ary[i].asn;
		pfxr.prefix = n->prefix;
		pfxr.min_len = n->len;
		pfxr.max_len = nd->ary[i].max_len;
		pfxr.socket = nd->ary[i].socket;
		fp(&pfxr, data);
	}

	if (n->rchild != NULL)
		pfx_table_for_each_rec(n->rchild, fp, data);
}

RTRLIB_EXPORT void pfx_table_for_each_ipv4_record(struct pfx_table *pfx_table, pfx_for_each_fp fp, void *data)
{
	assert(pfx_table != NULL);

	if (pfx_table->ipv4 == NULL)
		return;

	pthread_rwlock_rdlock(&(pfx_table->lock));
	pfx_table_for_each_rec(pfx_table->ipv4, fp, data);
	pthread_rwlock_unlock(&pfx_table->lock);
}

RTRLIB_EXPORT void pfx_table_for_each_ipv6_record(struct pfx_table *pfx_table, pfx_for_each_fp fp, void *data)
{
	assert(pfx_table != NULL);

	if (pfx_table->ipv6 == NULL)
		return;

	pthread_rwlock_rdlock(&(pfx_table->lock));
	pfx_table_for_each_rec(pfx_table->ipv6, fp, data);
	pthread_rwlock_unlock(&pfx_table->lock);
}

static void pfx_table_copy_cb(const struct pfx_record *record, void *data)
{
	struct copy_cb_args *args = data;
	if (record->socket != args->socket) {
		if (pfx_table_add(args->pfx_table, record) != PFX_SUCCESS) {
			args->error = true;
		}
	}
}

int pfx_table_copy_except_socket(struct pfx_table *src_table, struct pfx_table *dst_table,
				 const struct rtr_socket *socket)
{
	struct copy_cb_args args = {dst_table, socket, false};
	pfx_table_for_each_ipv4_record(src_table, pfx_table_copy_cb, &args);
	if (args.error) {
		return PFX_ERROR;
	}

	pfx_table_for_each_ipv6_record(src_table, pfx_table_copy_cb, &args);
	if (args.error) {
		return PFX_ERROR;
	}

	return PFX_SUCCESS;
}

void pfx_table_swap(struct pfx_table *a, struct pfx_table *b)
{
	struct trie_node *ipv4_tmp;
	struct trie_node *ipv6_tmp;

	pthread_rwlock_wrlock(&(a->lock));
	pthread_rwlock_wrlock(&(b->lock));

	ipv4_tmp = a->ipv4;
	ipv6_tmp = a->ipv6;

	a->ipv4 = b->ipv4;
	a->ipv6 = b->ipv6;

	b->ipv4 = ipv4_tmp;
	b->ipv6 = ipv6_tmp;

	pthread_rwlock_unlock(&(b->lock));
	pthread_rwlock_unlock(&(a->lock));
}

static void pfx_table_notify_diff_cb(const struct pfx_record *record, void *data)
{
	struct notify_diff_cb_args *args = data;

	if (args->socket == record->socket && args->added) {
		if (pfx_table_remove(args->old_table, record) != PFX_SUCCESS) {
			pfx_table_notify_clients(args->new_table, record, args->added);
		}
	} else if (args->socket == record->socket && !args->added) {
		pfx_table_notify_clients(args->new_table, record, args->added);
	}
}

void pfx_table_notify_diff(struct pfx_table *new_table, struct pfx_table *old_table, const struct rtr_socket *socket)
{
	pfx_update_fp old_table_fp;
	struct notify_diff_cb_args args = {old_table, new_table, socket, new_table->update_fp, true};

	// Disable update callback for old_table table
	old_table_fp = old_table->update_fp;
	old_table->update_fp = NULL;

	// Iterate new_table and try to delete every prefix from the given socket in old_table
	// If the prefix could not be removed it was added in new_table and the update cb must be called
	pfx_table_for_each_ipv4_record(new_table, pfx_table_notify_diff_cb, &args);
	pfx_table_for_each_ipv6_record(new_table, pfx_table_notify_diff_cb, &args);

	// Iterate over old_table table and search and remove remaining prefixes from the socket
	// issue a remove notification for every one of them, because they are not present in new_table.
	args.added = false;
	pfx_table_for_each_ipv4_record(old_table, pfx_table_notify_diff_cb, &args);
	pfx_table_for_each_ipv6_record(old_table, pfx_table_notify_diff_cb, &args);

	// Restore original state of old_tables update_fp
	old_table->update_fp = old_table_fp;
}
