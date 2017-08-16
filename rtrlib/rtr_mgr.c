/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "rtrlib/rtr_mgr.h"
#include "rtrlib/pfx/trie/trie-pfx.h"
#include "rtrlib/spki/hashtable/ht-spkitable.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "rtrlib/lib/log.h"
#include "rtrlib/lib/alloc_utils.h"

#define MGR_DBG(fmt, ...) lrtr_dbg("RTR_MGR: " fmt, ## __VA_ARGS__)
#define MGR_DBG1(a) lrtr_dbg("RTR_MGR: " a)

static const char * const mgr_str_status[] = {
	[RTR_MGR_CLOSED] = "RTR_MGR_CLOSED",
	[RTR_MGR_CONNECTING] = "RTR_MGR_CONNECTING",
	[RTR_MGR_ESTABLISHED] = "RTR_MGR_ESTABLISHED",
	[RTR_MGR_ERROR] = "RTR_MGR_ERROR",
};

static struct rtr_mgr_group *rtr_mgr_find_group(struct rtr_mgr_config *config,
						const struct rtr_socket *sock);
static int rtr_mgr_config_cmp(const void *a, const void *b);
static bool rtr_mgr_config_status_is_synced(const struct rtr_mgr_group *group);
static struct rtr_mgr_group *rtr_mgr_get_first_group(struct rtr_mgr_config
						     *conf);

static void set_status(const struct rtr_mgr_config *conf,
		       struct rtr_mgr_group *group,
		       enum rtr_mgr_status mgr_status,
		       const struct rtr_socket *rtr_sock)
{
	MGR_DBG("Group(%u) status changed to: %s", group->preference,
		rtr_mgr_status_to_str(mgr_status));

	group->status = mgr_status;
	if (conf->status_fp)
		conf->status_fp(group, mgr_status,
				rtr_sock, conf->status_fp_data);
}

static int rtr_mgr_start_sockets(struct rtr_mgr_group *group)
{
	for (unsigned int i = 0; i < group->sockets_len; i++) {
		if (rtr_start(group->sockets[i]) != 0) {
			MGR_DBG1("rtr_mgr: Error starting rtr_socket pthread");
			return RTR_ERROR;
		}
	}
	group->status = RTR_MGR_CONNECTING;
	return RTR_SUCCESS;
}

static struct rtr_mgr_group *rtr_mgr_find_group(struct rtr_mgr_config *config,
						const struct rtr_socket *sock)
{
	tommy_node *node;
	struct rtr_mgr_group_node *group_node;

	node  = tommy_list_head(&config->groups);

	while (node) {
		group_node = node->data;
		for (unsigned int j = 0; j < group_node->group->sockets_len;
									 j++) {
			if (group_node->group->sockets[j] == sock)
				return group_node->group;
		}
	node = node->next;
	}
	return NULL;
}

bool rtr_mgr_config_status_is_synced(const struct rtr_mgr_group *group)
{
	for (unsigned int i = 0; i < group->sockets_len; i++) {
		enum rtr_socket_state state = group->sockets[i]->state;

		if ((group->sockets[i]->last_update == 0) ||
		    ((state != RTR_ESTABLISHED) && (state != RTR_RESET) &&
		     (state != RTR_SYNC)))
			return false;
	}
	return true;
}

static void rtr_mgr_close_less_preferable_groups(const struct rtr_socket *sock,
						 struct rtr_mgr_config *config,
						 struct rtr_mgr_group *my_group)
{
	tommy_node *node = tommy_list_head(&config->groups);
	struct rtr_mgr_group_node *group_node;
	struct rtr_mgr_group *current_group;

	while (node) {
		group_node = node->data;
		current_group = group_node->group;
		if ((current_group->status != RTR_MGR_CLOSED) &&
		    (current_group != my_group) &&
		    (current_group->preference > my_group->preference)) {
			for (unsigned int j = 0;
				j < current_group->sockets_len; j++) {
				pthread_mutex_unlock(&config->mutex);
				rtr_stop(current_group->sockets[j]);
				pthread_mutex_lock(&config->mutex);
			}
			set_status(config, current_group, RTR_MGR_CLOSED, sock);
		}
	node = node->next;
	}
}

static struct rtr_mgr_group *get_best_inactive_rtr_mgr_group(
						struct rtr_mgr_config *config,
						struct rtr_mgr_group *my_group)
{
	tommy_node *node = tommy_list_head(&config->groups);
	struct rtr_mgr_group_node *group_node;
	struct rtr_mgr_group *group;

	while (node) {
	group_node = node->data;
	group = group_node->group;
		if ((group != my_group) && (group->status == RTR_MGR_CLOSED))
			return group;
	node = node->next;
	}
	return NULL;
}

static bool is_some_rtr_mgr_group_established(struct rtr_mgr_config *config)
{
	struct rtr_mgr_group_node *group_node;
	tommy_node *node = tommy_list_head(&config->groups);

	while (node) {
		group_node = node->data;
		if (group_node->group->status == RTR_MGR_ESTABLISHED)
			return true;
		node = node->next;
	}
	return false;
}

static inline void _rtr_mgr_cb_state_shutdown(const struct rtr_socket *sock,
					      struct rtr_mgr_config *config,
					      struct rtr_mgr_group *group)
{
	bool all_down = true;

	for (unsigned int i = 0; i < group->sockets_len; i++) {
		if (group->sockets[i]->state != RTR_SHUTDOWN) {
			all_down = false;
			break;
		}
	}
	if (all_down)
		set_status(config, group,
			   RTR_MGR_CLOSED, sock);
	else
		set_status(config, group,
			   group->status, sock);
}

static inline void _rtr_mgr_cb_state_established(const struct rtr_socket *sock,
						 struct rtr_mgr_config *config,
						 struct rtr_mgr_group *my_group)
{
	/* socket established a connection to the rtr_server */
	if (my_group->status == RTR_MGR_CONNECTING) {
		/*
		 * if previous state was CONNECTING, check if all
		 * other sockets in the group also have a established
		 * connection, if yes change group state to ESTABLISHED
		 */
		if (rtr_mgr_config_status_is_synced(my_group)) {
			set_status(config, my_group,
				   RTR_MGR_ESTABLISHED, sock);
			rtr_mgr_close_less_preferable_groups(sock, config,
							     my_group);
		} else {
			set_status(config, my_group,
				   RTR_MGR_CONNECTING, sock);
		}
	} else if (my_group->status == RTR_MGR_ERROR) {
		/* if previous state was ERROR, only change state to
		 * ESTABLISHED if all other more preferable socket
		 * groups are also in ERROR or SHUTDOWN state
		 */
		bool all_error = true;
		tommy_node *node = tommy_list_head(&config->groups);
		struct rtr_mgr_group_node *group_node;
		struct rtr_mgr_group *current_group;

		while (node) {
			group_node = node->data;
			current_group = group_node->group;
			if (current_group != my_group &&
			    current_group->status != RTR_MGR_ERROR &&
			    current_group->status != RTR_MGR_CLOSED &&
			    current_group->preference < my_group->preference) {
				all_error = false;
			}
			node = node->next;
		}

		if (all_error && rtr_mgr_config_status_is_synced(my_group)) {
			set_status(config, my_group, RTR_MGR_ESTABLISHED, sock);
			rtr_mgr_close_less_preferable_groups(sock, config,
							     my_group);
		} else {
			set_status(config, my_group, RTR_MGR_ERROR, sock);
		}
	}
}

static inline void _rtr_mgr_cb_state_connecting(const struct rtr_socket *sock,
						struct rtr_mgr_config *config,
						struct rtr_mgr_group *my_group)
{
	if (my_group->status == RTR_MGR_ERROR)
		set_status(config, my_group,
			   RTR_MGR_ERROR, sock);
	else
		set_status(config, my_group,
			   RTR_MGR_CONNECTING, sock);
}

static inline void _rtr_mgr_cb_state_error(const struct rtr_socket *sock,
					   struct rtr_mgr_config *config,
					   struct rtr_mgr_group *my_group)
{
	set_status(config, my_group, RTR_MGR_ERROR, sock);

	if (!is_some_rtr_mgr_group_established(config)) {
		struct rtr_mgr_group *next_group =
			get_best_inactive_rtr_mgr_group(config, my_group);

		if (next_group)
			rtr_mgr_start_sockets(next_group);
		else
		      MGR_DBG1("No other inactive groups found");
	}
}

static void rtr_mgr_cb(const struct rtr_socket *sock,
		       const enum rtr_socket_state state,
		       void *data)
{
	if (state == RTR_SHUTDOWN)
		MGR_DBG1("Received RTR_SHUTDOWN callback");

	struct rtr_mgr_config *config = data;
	struct rtr_mgr_group *group = rtr_mgr_find_group(config, sock);

	if (!group) {
		MGR_DBG1("ERROR: Socket has no group");
		return;
	}
	pthread_mutex_lock(&config->mutex);

	switch (state) {
	case RTR_SHUTDOWN:
		_rtr_mgr_cb_state_shutdown(sock, config, group);
		break;
	case RTR_ESTABLISHED:
		_rtr_mgr_cb_state_established(sock, config, group);
		break;
	case RTR_CONNECTING:
		_rtr_mgr_cb_state_connecting(sock, config, group);
		break;
	case RTR_ERROR_FATAL:
	case RTR_ERROR_TRANSPORT:
	case RTR_ERROR_NO_DATA_AVAIL:
		_rtr_mgr_cb_state_error(sock, config, group);
		break;
	default:
		set_status(config, group, group->status, sock);
	}
	pthread_mutex_unlock(&config->mutex);
}

int rtr_mgr_config_cmp(const void *a, const void *b)
{
	const struct rtr_mgr_group *ar = a;
	const struct rtr_mgr_group *br = b;

	if (ar->preference > br->preference)
		return 1;
	else if (ar->preference < br->preference)
		return -1;
	return 0;
}

int rtr_mgr_init(struct rtr_mgr_config **config_out,
		 struct rtr_mgr_group groups[],
		 const unsigned int groups_len,
		 const unsigned int refresh_interval,
		 const unsigned int expire_interval,
		 const unsigned int retry_interval,
		 const pfx_update_fp update_fp,
		 const spki_update_fp spki_update_fp,
		 const rtr_mgr_status_fp status_fp,
		 void *status_fp_data)
{
	int err_code = RTR_ERROR;
	struct pfx_table *pfxt = NULL;
	struct spki_table *spki_table = NULL;
	struct rtr_mgr_config *config = NULL;
	uint8_t last_preference = UINT8_MAX;

	*config_out = NULL;

	if (groups_len == 0) {
		MGR_DBG1("Error Empty rtr_mgr_group array");
		return RTR_ERROR;
	}

	*config_out = config = lrtr_malloc(sizeof(*config));
	if (!config)
		return RTR_ERROR;

	if (pthread_mutex_init(&config->mutex, NULL) != 0) {
		MGR_DBG1("Mutex initialization failed");
		goto err;
	}
	/* sort array in asc order, so we can check for dupl. pref */
	qsort(groups, groups_len,
	      sizeof(struct rtr_mgr_group), &rtr_mgr_config_cmp);

	/* Check that the groups have unique pref and at least one socket */
	for (unsigned int i = 0; i < config->len; i++) {
		struct rtr_mgr_group cg = groups[i];

		if ((i > 0) && (cg.preference == last_preference)) {
			MGR_DBG1("Error Same preference for 2 socket groups!");
			goto err;
		}
		if (cg.sockets_len == 0) {
			MGR_DBG1("Error Empty sockets array in socket group!");
			goto err;
		}
	}

	/* Init data structures that we need to pass to the sockets */
	pfxt = lrtr_malloc(sizeof(*pfxt));
	if (!pfxt)
		goto err;
	pfx_table_init(pfxt, update_fp);

	spki_table = lrtr_malloc(sizeof(*spki_table));
	if (!spki_table)
		goto err;
	spki_table_init(spki_table, spki_update_fp);

	config->pfx_table = pfxt;
	config->spki_table = spki_table;

	/* Copy the groups from the array into linked list config->groups */
	config->len = groups_len;
	config->groups = NULL;
	struct rtr_mgr_group *cg;
	struct rtr_mgr_group_node *group_node;

	for (unsigned int i = 0; i < groups_len; i++) {
		cg = lrtr_malloc(sizeof(struct rtr_mgr_group));
		memcpy(cg, &groups[i], sizeof(struct rtr_mgr_group));

		cg->status = RTR_MGR_CLOSED;
		for (unsigned int j = 0; j < cg->sockets_len; j++) {
			if (rtr_init(cg->sockets[j], NULL,
				     pfxt, spki_table, refresh_interval,
				     expire_interval, retry_interval,
				     rtr_mgr_cb, config) != RTR_SUCCESS) {
				err_code = RTR_INVALID_PARAM;
				goto err;
			}
		}
		group_node = lrtr_malloc(sizeof(struct rtr_mgr_group_node));
		group_node->group = cg;
		tommy_list_insert_tail(&config->groups, &group_node->node,
				       group_node);
	}
	/* Our linked list should be sorted already, since the groups array was
	 * sorted. However, for safety reasons we sort again.
	 */
	tommy_list_sort(&config->groups, &rtr_mgr_config_cmp);

	config->status_fp_data = status_fp_data;
	config->status_fp = status_fp;
	return RTR_SUCCESS;

err:
	if (spki_table)
		spki_table_free(spki_table);
	if (pfxt)
		pfx_table_free(pfxt);
	lrtr_free(pfxt);
	lrtr_free(spki_table);

	lrtr_free(config->groups);
	lrtr_free(config);
	config = NULL;
	*config_out = NULL;
	return err_code;
}

static struct rtr_mgr_group *rtr_mgr_get_first_group(struct rtr_mgr_config
						     *config)
{
	tommy_node *head = tommy_list_head(&config->groups);
	struct rtr_mgr_group_node *group_node = head->data;

	return group_node->group;
}

int rtr_mgr_start(struct rtr_mgr_config *config)
{
	MGR_DBG1("rtr_mgr_start()");
	struct rtr_mgr_group *best_group = rtr_mgr_get_first_group(config);

	return rtr_mgr_start_sockets(best_group);
}

bool rtr_mgr_conf_in_sync(struct rtr_mgr_config *config)
{
	bool all_sync;
	struct rtr_mgr_group_node *group_node;

	pthread_mutex_lock(&config->mutex);
	tommy_node *node = tommy_list_head(&config->groups);

	while (node) {
		all_sync = true;
		group_node = node->data;
		for (unsigned int j = 0; all_sync &&
		     (j < group_node->group->sockets_len); j++) {
			if (group_node->group->sockets[j]->last_update == 0)
				all_sync = false;
		}
		if (all_sync) {
			pthread_mutex_unlock(&config->mutex);
			return true;
		}
		node = node->next;
	}
	pthread_mutex_unlock(&config->mutex);
	return false;
}

void rtr_mgr_free(struct rtr_mgr_config *config)
{
	MGR_DBG1("rtr_mgr_free()");
	pthread_mutex_lock(&config->mutex);

	pfx_table_free(config->pfx_table);
	spki_table_free(config->spki_table);
	lrtr_free(config->spki_table);
	lrtr_free(config->pfx_table);

	/* Free linked list */
	tommy_node *tmp;
	tommy_node *head = tommy_list_head(&config->groups);
	struct rtr_mgr_group_node *group_node;

	while (head) {
		tmp = head;
		head = head->next;
		group_node = tmp->data;
		for (unsigned int j = 0; j < group_node->group->sockets_len;
		     j++) {
			tr_free(group_node->group->sockets[j]->tr_socket);
		}

		lrtr_free(group_node->group);
		lrtr_free(group_node);
	}

	pthread_mutex_unlock(&config->mutex);
	pthread_mutex_destroy(&config->mutex);
	lrtr_free(config);
}

/* cppcheck-suppress unusedFunction */
inline int rtr_mgr_validate(struct rtr_mgr_config *config,
			    const uint32_t asn,
			    const struct lrtr_ip_addr *prefix,
			    const uint8_t mask_len,
			    enum pfxv_state *result)
{
	return pfx_table_validate(config->pfx_table, asn, prefix, mask_len,
				  result);
}

/* cppcheck-suppress unusedFunction */
inline int rtr_mgr_get_spki(struct rtr_mgr_config *config,
			    const uint32_t asn,
			    uint8_t *ski,
			    struct spki_record **result,
			    unsigned int *result_count)
{
	return spki_table_get_all(config->spki_table,
				  asn, ski, result, result_count);
}

void rtr_mgr_stop(struct rtr_mgr_config *config)
{
	tommy_node *node = tommy_list_head(&config->groups);
	struct rtr_mgr_group_node *group_node;

	MGR_DBG1("rtr_mgr_stop()");
	while (node) {
		group_node = node->data;
		for (unsigned int j = 0; j < group_node->group->sockets_len;
		     j++) {
			rtr_stop(group_node->group->sockets[j]);
		}
	node = node->next;
	}
}

int rtr_mgr_add_group(struct rtr_mgr_config *config,
		      const struct rtr_mgr_group *group)
{
	unsigned int refresh_iv = 3600;
	unsigned int retry_iv = 600;
	unsigned int expire_iv = 7200;
	int err_code = RTR_ERROR;

	pthread_mutex_lock(&config->mutex);

	struct rtr_mgr_group_node *gnode;
	tommy_node *node = tommy_list_head(&config->groups);

	while (node) {
		gnode = node->data;
		if (gnode->group->preference == group->preference) {
			RTR_DBG("Group with preference value already exists!");
			return RTR_INVALID_PARAM;
		}

		//TODO This is not pretty. It wants to get the same intervals
		//that are being used by other groups. Maybe intervals should
		//be store globally/per-group/per-socket?
		if (gnode->group->sockets[0]->refresh_interval)
			refresh_iv = gnode->group->sockets[0]->refresh_interval;
		if (gnode->group->sockets[0]->retry_interval)
			retry_iv = gnode->group->sockets[0]->retry_interval;
		if (gnode->group->sockets[0]->expire_interval)
			expire_iv = gnode->group->sockets[0]->expire_interval;
		node = node->next;
	}
	struct rtr_mgr_group *new_group = lrtr_malloc(sizeof(
							struct rtr_mgr_group));
	if (!new_group) {
		pthread_mutex_unlock(&config->mutex);
		return RTR_ERROR;
	}
	memcpy(new_group, group, sizeof(struct rtr_mgr_group));

	for (unsigned int j = 0; j < new_group->sockets_len; j++) {
		if (rtr_init(new_group->sockets[j], NULL, config->pfx_table,
			     config->spki_table, refresh_iv,
			     expire_iv, retry_iv, rtr_mgr_cb,
			     config) != RTR_SUCCESS) {
			err_code = RTR_INVALID_PARAM;
			lrtr_free(new_group);
			pthread_mutex_unlock(&config->mutex);
			return err_code;
		}
	}

	struct rtr_mgr_group_node *new_group_node = lrtr_malloc(
					sizeof(struct rtr_mgr_group_node));
	if (!new_group_node) {
		pthread_mutex_unlock(&config->mutex);
		lrtr_free(new_group);
		return RTR_ERROR;
	}

	new_group_node->group = new_group;
	tommy_list_insert_tail(&config->groups, &new_group_node->node,
			       new_group_node);
	config->len++;

	tommy_list_sort(&config->groups, &rtr_mgr_config_cmp);
	pthread_mutex_unlock(&config->mutex);

	struct rtr_mgr_group *best_group = rtr_mgr_get_first_group(config);
	if (best_group->status == RTR_MGR_CLOSED) {
		for (unsigned int j = 0; j < best_group->sockets_len; j++)
			rtr_start(best_group->sockets[j]);
	}
	return RTR_SUCCESS;
}

int rtr_mgr_remove_group(struct rtr_mgr_config *config,
			 unsigned int preference)
{
	pthread_mutex_lock(&config->mutex);
	tommy_node *remove_node = NULL;
	tommy_node *node = tommy_list_head(&config->groups);
	struct rtr_mgr_group_node *group_node;
	struct rtr_mgr_group *remove_group;

	// Find the node of the group we want to remove
	while (node && !remove_node) {
		group_node = node->data;
		if (group_node->group->preference == preference)
			remove_node = node;
		node = node->next;
	}

	if (!remove_node) {
		RTR_DBG("The group that should be removed does not exist!");
		pthread_mutex_unlock(&config->mutex);
		return RTR_ERROR;
	}

	group_node = remove_node->data;
	remove_group = group_node->group;
	tommy_list_remove_existing(&config->groups, remove_node);
	config->len--;
	tommy_list_sort(&config->groups, &rtr_mgr_config_cmp);
	pthread_mutex_unlock(&config->mutex);

	//If group isn't closed, make it so!
	if (remove_group->status != RTR_MGR_CLOSED) {
		for (unsigned int j = 0; j < remove_group->sockets_len; j++) {
			rtr_stop(remove_group->sockets[j]);
			tr_free(remove_group->sockets[j]->tr_socket);
		}
	}

	// Check whether best group is closed, if so then start it.
	struct rtr_mgr_group *best_group = rtr_mgr_get_first_group(config);

	if (best_group->status == RTR_MGR_CLOSED) {
		for (unsigned int j = 0; j < best_group->sockets_len; j++)
			rtr_start(best_group->sockets[j]);
	}

	lrtr_free(group_node->group);
	lrtr_free(group_node);
	return RTR_SUCCESS;
}

const char *rtr_mgr_status_to_str(enum rtr_mgr_status status)
{
	return mgr_str_status[status];
}

/* cppcheck-suppress unusedFunction */
inline void rtr_mgr_for_each_ipv4_record(struct rtr_mgr_config *config,
					 void (fp)(const struct pfx_record *,
						   void *data),
						   void *data)
{
	pfx_table_for_each_ipv4_record(config->pfx_table,
				       fp, data);
}

/* cppcheck-suppress unusedFunction */
inline void rtr_mgr_for_each_ipv6_record(struct rtr_mgr_config *config,
					 void (fp)(const struct pfx_record *,
						   void *data),
					 void *data)
{
	pfx_table_for_each_ipv6_record(config->pfx_table,
				       fp, data);
}
