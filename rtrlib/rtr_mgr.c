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
 * INET group, Hamburg University of Applied Sciences,
 * CST group, Freie Universitaet Berlin
 * Website: http://rpki.realmv6.org/
 */

#include "rtrlib/rtr_mgr.h"
#include "rtrlib/pfx/lpfst/lpfst-pfx.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "rtrlib/lib/log.h"

#define MGR_DBG(fmt, ...) dbg("RTR_MGR: " fmt, ## __VA_ARGS__)
#define MGR_DBG1(a) dbg("RTR_MGR: " a)

static const char *mgr_str_status[] = {
    [RTR_MGR_CLOSED] = "RTR_MGR_CLOSED",
    [RTR_MGR_CONNECTING] = "RTR_MGR_CLOSED",
    [RTR_MGR_ESTABLISHED] = "RTR_MGR_ESTABLISHED",
    [RTR_MGR_ERROR] = "RTR_MGR_ERROR",
};

static int rtr_mgr_find_group(const struct rtr_mgr_config *config, const struct rtr_socket *sock, unsigned int *ind);
static int rtr_mgr_config_cmp(const void *a, const void *b);
static bool rtr_mgr_config_status_is_synced(const struct rtr_mgr_group *config);

static void set_status(const struct rtr_mgr_config *conf, struct rtr_mgr_group *group,
			   enum rtr_mgr_status mgr_status, const struct rtr_socket *rtr_sock)
{
	MGR_DBG("Group(%u) status changed to: %s", group->preference,
			rtr_mgr_status_to_str(mgr_status));

    group->status = mgr_status;
    if(conf->status_fp != NULL)
	conf->status_fp(group, mgr_status, rtr_sock, conf->status_fp_data);
}

static int rtr_mgr_start_sockets(struct rtr_mgr_group *group) {
    for(unsigned int i = 0; i < group->sockets_len; i++) {
        if(rtr_start(group->sockets[i]) != 0) {
            MGR_DBG1("rtr_mgr: Error starting rtr_socket pthread");
            return RTR_ERROR;
        }
    }
    return RTR_SUCCESS;
}

int rtr_mgr_find_group(const struct rtr_mgr_config *config, const struct rtr_socket *sock, unsigned int *ind) {
    for(unsigned int i = 0; i < config->len; i++) {
        for(unsigned int j = 0; j < config->groups[i].sockets_len; j++) {
            if(config->groups[i].sockets[j] == sock) {
                *ind = i;
                return RTR_SUCCESS;
            }
        }
    }
    MGR_DBG1("Error couldn't find a wanted rtr_socket in rtr_mgr_config");
    return RTR_ERROR;
}

bool rtr_mgr_config_status_is_synced(const struct rtr_mgr_group *group) {
    for(unsigned int i = 0; i < group->sockets_len; i++) {
        enum rtr_socket_state state = group->sockets[i]->state;
        if((group->sockets[i]->last_update == 0) || (state != RTR_ESTABLISHED && state != RTR_RESET && state != RTR_SYNC))
            return false;
    }
    return true;
}

static void rtr_mgr_cb(const struct rtr_socket *sock, const enum rtr_socket_state state, void *data) {
    struct rtr_mgr_config *config = data;

    //find the index in the rtr_mgr_config struct, for which this function was called
    unsigned int ind = 0;
    if(rtr_mgr_find_group(config, sock, &ind) == -1)
        return;

    pthread_mutex_lock(&(config->mutex));

    if(config->len == 1) {
        if (state == RTR_CONNECTING)
            set_status(config, &config->groups[ind], RTR_MGR_CONNECTING, sock);
        else if (state == RTR_ESTABLISHED)
            set_status(config, &config->groups[ind], RTR_MGR_ESTABLISHED, sock);
        else if(state == RTR_ERROR_FATAL || state == RTR_ERROR_TRANSPORT ||
                state == RTR_ERROR_NO_DATA_AVAIL)
            set_status(config, &config->groups[ind], RTR_MGR_ERROR, sock);
        else if (state == RTR_SHUTDOWN)
            set_status(config, &config->groups[ind], RTR_MGR_CLOSED, sock);
	else
	    set_status(config, &config->groups[ind],
		       config->groups[ind].status, sock);
        goto out;
    }

    if(state == RTR_SHUTDOWN)
        goto out;

    if(state == RTR_ESTABLISHED) {
        //socket established successfull a connection to the rtr_server
        if(config->groups[ind].status == RTR_MGR_CONNECTING) {
            //if previous state was CONNECTING, check if all other sockets in the group also have a established connection,
            //if yes change group state to ESTABLISHED
            if(rtr_mgr_config_status_is_synced(&(config->groups[ind]))) {
                set_status(config, &config->groups[ind], RTR_MGR_ESTABLISHED, sock);
                for(unsigned int i = 0; i < config->len; i++) {
                    if(config->groups[i].status != RTR_MGR_CLOSED && i != ind) {
                        for(unsigned int j = 0; j < config->groups[i].sockets_len; j++) {
                            rtr_stop(config->groups[i].sockets[j]);
                        }
			set_status(config, &config->groups[i],
				   RTR_MGR_ESTABLISHED, sock);
                    }
                }
            }
        }
        else if(config->groups[ind].status == RTR_MGR_ERROR) {
            //if previous state was ERROR, only change state to ESTABLISHED if all other socket groups are also in error or SHUTDOWN state
            bool all_error = true;
            for(unsigned int i = 0; (i < config->len) && (all_error); i++) {
                if(config->groups[i].status != RTR_MGR_ERROR && config->groups[i].status != RTR_MGR_CLOSED)
                    all_error = false;
            }
            if(all_error && rtr_mgr_config_status_is_synced(&(config->groups[ind]))) {
		set_status(config, &config->groups[ind], RTR_MGR_ESTABLISHED,
			   sock);
                for(unsigned int i = 0; i < config->len; i++) {
                    if(config->groups[i].status != RTR_MGR_CLOSED && i != ind) {
                        for(unsigned int j = 0; j < config->groups[i].sockets_len; j++) {
                            rtr_stop(config->groups[i].sockets[j]);
                        }
			set_status(config, &config->groups[i], RTR_MGR_CLOSED,
				   sock);
                    }
                }
            }
        }
    }
    else if(state == RTR_ERROR_FATAL || state == RTR_ERROR_TRANSPORT || state == RTR_ERROR_NO_DATA_AVAIL) {
        set_status(config, &config->groups[ind], RTR_MGR_ERROR, sock);

        int next_config = ind + 1;
        bool found = false;
        //find next group with higher preference value
        for(unsigned int i = ind + 1; (i < config->len) && !found; i++) {
            if(config->groups[i].status == RTR_MGR_CLOSED) {
                found = true;
                next_config = i;
            }
        }
        if(!found) {
            //find group with lower preference value
            for(int i = ind - 1; (i > - 1) && (!found); i--) {
                if(config->groups[i].status == RTR_MGR_CLOSED) {
                    found = true;
                    next_config = i;
                }
            }
        }

        if(found)
            rtr_mgr_start_sockets(&(config->groups[next_config]));
        else
            MGR_DBG1("No other inactive groups found");
    }
    else if(state == RTR_ERROR_NO_INCR_UPDATE_AVAIL) {
        set_status(config, &config->groups[ind], RTR_MGR_ERROR, sock);
        //find a group with a lower preference value, if no other group exists do nothing
        int next_config = ind - 1;
        while(next_config >= 0 && config->groups[next_config].status != RTR_MGR_CLOSED) {
            next_config--;
        }
        if(next_config >= 0)
            rtr_mgr_start_sockets(&(config->groups[next_config]));
    }

out:
    pthread_mutex_unlock(&(config->mutex));
    return;
}

int rtr_mgr_config_cmp(const void *a, const void *b) {
    const struct rtr_mgr_group *ar = a;
    const struct rtr_mgr_group *br = b;
    if(ar->preference > br->preference)
        return 1;
    else if(ar->preference < br->preference)
        return -1;
    return 0;
}

struct rtr_mgr_config *rtr_mgr_init(struct rtr_mgr_group groups[],
                             const unsigned int groups_len,
                             const unsigned int polling_period,
                             const unsigned int cache_timeout,
                             const pfx_update_fp update_fp,
                             const rtr_mgr_status_fp status_fp,
                             void *status_fp_data) {
    struct rtr_mgr_config *config = malloc(sizeof(*config));
    if (config == NULL)
        return NULL;

    if(pthread_mutex_init(&(config->mutex), NULL) != 0) {
        MGR_DBG1("Mutex initialization failed");
        goto err;
    }

    config->len = groups_len;
    config->groups = malloc(groups_len * sizeof(*groups));
    if (config->groups == NULL)
        goto err;
    memcpy(config->groups, groups, groups_len * sizeof(*groups));

    //sort array in asc preference order
    qsort(config->groups, config->len, sizeof(struct rtr_mgr_group), &rtr_mgr_config_cmp);

    for(unsigned int i = 0; i < config->len; i++) {
        if(config->groups[i].sockets_len == 0) {
            MGR_DBG1("Error Socket group contains an empty sockets array");
            goto err;
        }
        if(i > 0 && config->groups[i-1].preference == config->groups[i].preference) {
            MGR_DBG1("Error Socket group contains 2 members with the same preference value");
            goto err;
        }
    }

    struct pfx_table *pfxt = malloc(sizeof(*pfxt));
    if(pfxt == NULL)
            goto err;
    pfx_table_init(pfxt, update_fp);

    for(unsigned int i = 0; i < config->len; i++) {
        config->groups[i].status = RTR_MGR_CLOSED;
        for(unsigned int j = 0; j < config->groups[i].sockets_len; j++) {
            rtr_init(config->groups[i].sockets[j], NULL, pfxt, polling_period, cache_timeout, rtr_mgr_cb, config);
        }
    }
    config->status_fp_data = status_fp_data;
    config->status_fp = status_fp;
    return config;

err:
    free(config->groups);
    free(config);
    return NULL;
}

int rtr_mgr_start(struct rtr_mgr_config *config) {
    return rtr_mgr_start_sockets(&(config->groups[0]));
}

bool rtr_mgr_conf_in_sync(struct rtr_mgr_config *config) {
    for(unsigned int i = 0; i < config->len; i++) {
        bool all_sync = true;
        for(unsigned int j = 0; all_sync && (j < config->groups[i].sockets_len); j++) {
            if(config->groups[i].sockets[j]->last_update == 0)
                all_sync = false;
        }
        if(all_sync)
            return true;
    }
    return false;
}

void rtr_mgr_free(struct rtr_mgr_config *config) {
    pthread_mutex_lock(&(config->mutex));
    pfx_table_free(config->groups[0].sockets[0]->pfx_table);
    free(config->groups[0].sockets[0]->pfx_table);
    free(config);
    pthread_mutex_unlock(&(config->mutex));
    pthread_mutex_destroy(&(config->mutex));
}

inline int rtr_mgr_validate(struct rtr_mgr_config *config, const uint32_t asn, const struct ip_addr *prefix, const uint8_t mask_len, enum pfxv_state *result) {
    return pfx_table_validate(config->groups[0].sockets[0]->pfx_table, asn, prefix, mask_len, result);
}

void rtr_mgr_stop(struct rtr_mgr_config *config) {
    for(unsigned int i = 0; i < config->len; i++) {
        for(unsigned int j = 0; j < config->groups[i].sockets_len; j++) {
            rtr_stop(config->groups[i].sockets[j]);
        }
    }
}

const char *rtr_mgr_status_to_str(enum rtr_mgr_status status)
{
    return mgr_str_status[status];
}

inline void rtr_mgr_for_each_ipv4_record(struct rtr_mgr_config *config, void (fp)(const struct pfx_record *, void *data), void *data) {
	pfx_table_for_each_ipv4_record(config->groups[0].sockets[0]->pfx_table, fp, data);
}

inline void rtr_mgr_for_each_ipv6_record(struct rtr_mgr_config *config, void (fp)(const struct pfx_record *, void *data), void *data) {
	pfx_table_for_each_ipv6_record(config->groups[0].sockets[0]->pfx_table, fp, data);
}
