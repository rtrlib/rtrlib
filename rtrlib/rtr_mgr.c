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

static int rtr_mgr_find_group(const struct rtr_mgr_config *config,
			      const struct rtr_socket *sock,
			      unsigned int *ind);
static int rtr_mgr_config_cmp(const void *a, const void *b);
static bool rtr_mgr_config_status_is_synced(const struct rtr_mgr_group *config);

static void set_status(const struct rtr_mgr_config_ll *conf,
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

int rtr_mgr_find_group(const struct rtr_mgr_config *config,
		       const struct rtr_socket *sock,
		       unsigned int *ind)
{
	for (unsigned int i = 0; i < config->len; i++) {
		for (unsigned int j = 0;
		     j < config->groups[i].sockets_len; j++) {
			if (config->groups[i].sockets[j] == sock) {
				*ind = i;
				return RTR_SUCCESS;
			}
		}
	}
	MGR_DBG1("Error couldn't find a wanted rtr_socket in rtr_mgr_config");
	return RTR_ERROR;
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
						 unsigned int my_group_idx)
{
	for (unsigned int i = 0; i < config->len; i++) {
		struct rtr_mgr_group cg = config->groups[i];

		if ((cg.status != RTR_MGR_CLOSED) && (i != my_group_idx) &&
		    (cg.preference > config->groups[my_group_idx].preference)) {
			for (unsigned int j = 0; j < cg.sockets_len; j++) {
				pthread_mutex_unlock(&config->mutex);
				rtr_stop(cg.sockets[j]);
				pthread_mutex_lock(&config->mutex);
			}
			set_status(config, &cg, RTR_MGR_CLOSED, sock);
		}
	}
}

static struct rtr_mgr_group
*get_best_inactive_rtr_mgr_group(struct rtr_mgr_config *config,
				 unsigned int my_group_idx)
{
	/* groups are sorted by preference */
	for (int i = 0; i < config->len; i++) {
		if ((i != my_group_idx) &&
		    (config->groups[i].status == RTR_MGR_CLOSED))
			return &config->groups[i];
	}
	return NULL;
}

static bool is_some_rtr_mgr_group_established(struct rtr_mgr_config *config)
{
	for (int i = 0; i < config->len; i++) {
		if (config->groups[i].status == RTR_MGR_ESTABLISHED)
			return true;
	}
	return false;
}

static inline void _rtr_mgr_cb_state_shutdown(const struct rtr_socket *sock,
					      struct rtr_mgr_config *config,
					      unsigned int ind)
{
	bool all_down = true;

	for (unsigned int i = 0; i < config->groups[ind].sockets_len; i++) {
		if (config->groups[ind].sockets[i]->state != RTR_SHUTDOWN) {
			all_down = false;
			break;
		}
	}
	if (all_down)
		set_status(config, &config->groups[ind],
			   RTR_MGR_CLOSED, sock);
	else
		set_status(config, &config->groups[ind],
			   config->groups[ind].status, sock);
}

static inline void _rtr_mgr_cb_state_established(const struct rtr_socket *sock,
						 struct rtr_mgr_config *config,
						 unsigned int ind)
{
	/* socket established a connection to the rtr_server */
	if (config->groups[ind].status == RTR_MGR_CONNECTING) {
		/*
		 * if previous state was CONNECTING, check if all
		 * other sockets in the group also have a established
		 * connection, if yes change group state to ESTABLISHED
		 */
		if (rtr_mgr_config_status_is_synced(&config->groups[ind])) {
			set_status(config, &config->groups[ind],
				   RTR_MGR_ESTABLISHED, sock);
			rtr_mgr_close_less_preferable_groups(sock, config, ind);
		} else {
			set_status(config, &config->groups[ind],
				   RTR_MGR_CONNECTING, sock);
		}
	} else if (config->groups[ind].status == RTR_MGR_ERROR) {
		/*
		 * if previous state was ERROR, only change state to
		 * ESTABLISHED if all other more preferable socket
		 * groups are also in ERROR or SHUTDOWN state
		 */
		bool all_error = true;

		for (unsigned int i = 0; (i < config->len) && all_error; i++) {
			struct rtr_mgr_group cg = config->groups[i];

			if (i != ind &&
			    cg.status != RTR_MGR_ERROR &&
			    cg.status != RTR_MGR_CLOSED &&
			    cg.preference < config->groups[ind].preference)
				all_error = false;
		}
		if (all_error &&
		    rtr_mgr_config_status_is_synced(&config->groups[ind])) {
			set_status(config, &config->groups[ind],
				   RTR_MGR_ESTABLISHED, sock);
			rtr_mgr_close_less_preferable_groups(sock, config, ind);
		} else {
			set_status(config, &config->groups[ind],
				   RTR_MGR_ERROR, sock);
		}
	}
}

static inline void _rtr_mgr_cb_state_connecting(const struct rtr_socket *sock,
						struct rtr_mgr_config *config,
						unsigned int ind)
{
	if (config->groups[ind].status == RTR_MGR_ERROR)
		set_status(config, &config->groups[ind],
			   RTR_MGR_ERROR, sock);
	else
		set_status(config, &config->groups[ind],
			   RTR_MGR_CONNECTING, sock);
}

static inline void _rtr_mgr_cb_state_error(const struct rtr_socket *sock,
					   struct rtr_mgr_config *config,
					   unsigned int ind)
{
	set_status(config, &config->groups[ind],
		   RTR_MGR_ERROR, sock);

	if (!is_some_rtr_mgr_group_established(config)) {
		struct rtr_mgr_group *next_group =
			get_best_inactive_rtr_mgr_group(config, ind);

		if (next_group)
			rtr_mgr_start_sockets(next_group);
		else
		      MGR_DBG1("No other inactive groups found");
	}
}

static bool rtr_mgr_sock_in_group(const struct rtr_mgr_group* group, const struct rtr_socket* sock)
{
	for(unsigned int i = 0; group->sockets_len > i; i++) {
		if (group->sockets[i] == sock) {
			return true;
		}
	}
	return false;
}

static void rtr_mgr_cb(const struct rtr_socket *sock,
		       const enum rtr_socket_state state,
		       void *data)
{
	struct rtr_mgr_config_ll *config = data;

	if (rtr_mgr_sock_in_group(config->active_group, sock) != true) {
		MGR_DBG1("Active Socket is not in active group");
		return;
	}

	pthread_mutex_lock(&config->mutex);

	switch (state) {
	case RTR_SHUTDOWN:
		_rtr_mgr_cb_state_shutdown(sock, config, ind);
		break;
	case RTR_ESTABLISHED:
		_rtr_mgr_cb_state_established(sock, config, ind);
		break;
	case RTR_CONNECTING:
		_rtr_mgr_cb_state_connecting(sock, config, ind);
		break;
	case RTR_ERROR_FATAL:
	case RTR_ERROR_TRANSPORT:
	case RTR_ERROR_NO_DATA_AVAIL:
		_rtr_mgr_cb_state_error(sock, config, ind);
		break;
	default:
		set_status(config, config->active_group,
			   config->active_group->status, sock);
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

int rtr_mgr_init(struct rtr_mgr_config_ll **config_out,
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
	struct rtr_mgr_config_ll *config = NULL;
    struct rtr_mgr_group *active_group = NULL;
    uint8_t min_preference = UINT8_MAX;
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
	config->len = groups_len;


    //Init tommy_list that will hold our groups
    config->groups = NULL;

	/* sort array in asc preference order */
	//qsort(config->groups, config->len,
	//      sizeof(struct rtr_mgr_group), &rtr_mgr_config_cmp);

	pfxt = lrtr_malloc(sizeof(*pfxt));
	if (!pfxt)
		goto err;
	pfx_table_init(pfxt, update_fp);

	spki_table = lrtr_malloc(sizeof(*spki_table));
	if (!spki_table)
		goto err;
	spki_table_init(spki_table, spki_update_fp);

	config->status_fp_data = status_fp_data;
	config->status_fp = status_fp;

	for (unsigned int i = 0; i < config->len; i++) {
		struct rtr_mgr_group cg = groups[i];
		cg.status = RTR_MGR_CLOSED;
		if ((i > 0) && (cg.preference == last_preference)) {
			MGR_DBG1("Error Same preference for 2 socket groups!");
			goto err;
		}
		if (cg.sockets_len == 0) {
			MGR_DBG1("Error Empty sockets array in socket group!");
			goto err;
		}
		for (unsigned int j = 0; j < cg.sockets_len; j++) {
			if (rtr_init(cg.sockets[j], NULL,
				     pfxt, spki_table, refresh_interval,
				     expire_interval, retry_interval,
				     rtr_mgr_cb, config) != RTR_SUCCESS) {
				err_code = RTR_INVALID_PARAM;
				goto err;
			}
		}
		last_preference = cg.preference;
        struct rtr_mgr_group_node *group_node = lrtr_malloc(sizeof(struct rtr_mgr_group_node));
        struct rtr_mgr_group *group = lrtr_malloc(sizeof(struct rtr_mgr_group));
        memcpy(group, &groups[i], sizeof(struct rtr_mgr_group));
        group_node->group = &groups[i];
        tommy_list_insert_tail(&config->groups, &group_node->node, group_node); 

        if (cg.preference < min_preference) {
            min_preference = cg.preference;
            active_group = group;
        }
	}
    config->active_group = active_group;
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

int rtr_mgr_start(struct rtr_mgr_config_ll *config)
{
	MGR_DBG1("rtr_mgr_start()");
	return rtr_mgr_start_sockets(config->active_group);
}

bool rtr_mgr_conf_in_sync(struct rtr_mgr_config_ll *config)
{
    bool all_sync = true;
	for (unsigned int j = 0;
	     all_sync && (j < config->active_group->sockets_len); j++) {
		if (config->active_group->sockets[j]->last_update == 0)
			all_sync = false;
	}
	return all_sync;
}

void rtr_mgr_free(struct rtr_mgr_config *config)
{
	MGR_DBG1("rtr_mgr_free()");
	pthread_mutex_lock(&config->mutex);
	pfx_table_free(config->groups[0].sockets[0]->pfx_table);
	spki_table_free(config->groups[0].sockets[0]->spki_table);
	lrtr_free(config->groups[0].sockets[0]->spki_table);
	lrtr_free(config->groups[0].sockets[0]->pfx_table);
	lrtr_free(config->groups);
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
	return pfx_table_validate(config->groups[0].sockets[0]->pfx_table,
				  asn, prefix, mask_len, result);
}

/* cppcheck-suppress unusedFunction */
inline int rtr_mgr_get_spki(struct rtr_mgr_config *config,
			    const uint32_t asn,
			    uint8_t *ski,
			    struct spki_record **result,
			    unsigned int *result_count)
{
	return spki_table_get_all(config->groups[0].sockets[0]->spki_table,
				  asn, ski, result, result_count);
}

void rtr_mgr_stop(struct rtr_mgr_config *config)
{
	MGR_DBG1("rtr_mgr_stop()");
	for (unsigned int i = 0; i < config->len; i++) {
		for (unsigned int j = 0; j < config->groups[i].sockets_len; j++)
			rtr_stop(config->groups[i].sockets[j]);
	}
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
	pfx_table_for_each_ipv4_record(config->groups[0].sockets[0]->pfx_table,
				       fp, data);
}

/* cppcheck-suppress unusedFunction */
inline void rtr_mgr_for_each_ipv6_record(struct rtr_mgr_config *config,
					 void (fp)(const struct pfx_record *,
						   void *data),
					 void *data)
{
	pfx_table_for_each_ipv6_record(config->groups[0].sockets[0]->pfx_table,
				       fp, data);
}
