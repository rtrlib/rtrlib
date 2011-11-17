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

#include "rtrlib/rtr_mgr.h"
#include "rtrlib/pfx/lpfst/lpfst-pfx.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "rtrlib/lib/log.h"

#define MGR_DBG(fmt, ...) dbg("RTR_MGR: " fmt, ## __VA_ARGS__)
#define MGR_DBG1(a) dbg("RTR_MGR: " a)

static int rtr_mgr_find_group(const rtr_mgr_config* config, const const rtr_socket* sock, unsigned int* ind);
static int rtr_mgr_start_sockets(rtr_mgr_group* config);
static int rtr_mgr_config_cmp(const void* a, const void* b);
static bool rtr_mgr_config_status_is_synced(const rtr_mgr_group* config);

int rtr_mgr_start_sockets(rtr_mgr_group* config){
    for(unsigned int i = 0; i < config->sockets_len; i++){
        if(rtr_start(config->sockets[i]) != 0){
            MGR_DBG1("rtr_mgr: Error starting rtr_socket pthread");
            return RTR_ERROR;
        }
    }
    config->status = RTR_MGR_CONNECTING;
    MGR_DBG("Group(%u) status changed to: CONNECTING", config->preference);
    return RTR_SUCCESS;
}

int rtr_mgr_find_group(const rtr_mgr_config* config, const rtr_socket* sock, unsigned int* ind){
    for(unsigned int i = 0; i < config->len; i++){
        for(unsigned int j = 0; j < config->groups[i].sockets_len; j++){
            if(config->groups[i].sockets[j] == sock){
                *ind = i;
                return RTR_SUCCESS;
            }
        }
    }
    MGR_DBG1("Error couldn't find a wanted rtr_socket in rtr_mgr_config");
    return RTR_ERROR;
}

bool rtr_mgr_config_status_is_synced(const rtr_mgr_group* group){
    for(unsigned int i = 0; i < group->sockets_len; i++){
        rtr_socket_state state = group->sockets[i]->state;
        if((group->sockets[i]->last_update == 0) || (state != RTR_ESTABLISHED && state != RTR_RESET && state != RTR_SYNC))
            return false;
    }
    return true;
}

void rtr_mgr_cb(const struct rtr_socket* sock, const rtr_socket_state state, void* data){

    if(state == RTR_SHUTDOWN)
        return;
    rtr_mgr_config* config = data;

    //return if group contains no other socket groups => nothing todo :-)
    if(config->len == 1){
        return;
    }

    //find the index in the rtr_mgr_config struct, for which this function was called
    unsigned int ind = 0;
    if(rtr_mgr_find_group(config, sock, &ind) == -1)
        return;
    pthread_mutex_lock(&(config->mutex));
    if(state == RTR_ESTABLISHED){
        //socket established successfull a connection to the rtr_server
        if(config->groups[ind].status == RTR_MGR_CONNECTING){
            //if previous state was CONNECTING, check if all other sockets in the group also have a established connection,
            //if yes change group state to ESTABLISHED
            if(rtr_mgr_config_status_is_synced(&(config->groups[ind])))
                config->groups[ind].status = RTR_MGR_ESTABLISHED;
                MGR_DBG("Group(%u) status changed to: ESTABLISHED", config->groups[ind].preference);
                for(unsigned int i = 0; i < config->len; i++){
                    if(config->groups[i].status != RTR_MGR_CLOSED && i != ind){
                        for(unsigned int j = 0; j < config->groups[i].sockets_len;j++){
                            rtr_stop(config->groups[i].sockets[j]);
                        }
                        config->groups[i].status = RTR_MGR_CLOSED;
                        MGR_DBG("Group(%u) status changed to: CLOSED", config->groups[i].preference);
                    }
                }
            }
        if(config->groups[ind].status == RTR_MGR_ERROR){
            //if previous state was ERROR, only change state to ESTABLISHED if all other socket groups are also in error or SHUTDOWN state
            bool all_error = true;
            for(unsigned int i = 0; (i < config->len) && (all_error); i++){
                if(config->groups[i].status != RTR_MGR_ERROR && config->groups[i].status != RTR_MGR_CLOSED)
                    all_error = false;
            }
            if(all_error && rtr_mgr_config_status_is_synced(&(config->groups[ind]))){
                config->groups[ind].status = RTR_MGR_ESTABLISHED;
                MGR_DBG("Group(%u) status changed to: ESTABLISHED", config->groups[ind].preference);
                for(unsigned int i = 0; i < config->len; i++){
                    if(i != ind && config->groups[i].status != RTR_MGR_CLOSED){
                        for(unsigned int j = 0; j < config->groups[i].sockets_len;j++){
                            rtr_stop(config->groups[i].sockets[j]);
                        }
                        config->groups[i].status = RTR_MGR_CLOSED;
                        MGR_DBG("Group(%u) status changed to: CLOSED", config->groups[i].preference);
                    }
                }
            }
        }
    }
    else if(state == RTR_ERROR_FATAL || state == RTR_ERROR_TRANSPORT || state == RTR_ERROR_NO_DATA_AVAIL){
        //if another group exist which has status CLOSED, start connecting to the group
        config->groups[ind].status = RTR_MGR_ERROR;
        MGR_DBG("Group(%u) status changed to: ERROR", config->groups[ind].preference);

        int next_config = ind + 1;
        bool found = false;
        //find next group with higher preference value
        for(unsigned int i = ind + 1; (i < config->len) && (!found); i++){
            if(config->groups[i].status == RTR_MGR_CLOSED){
                found = true;
                next_config = i;
            }
        }
        if(!found){
            //find group with lower preference value
            for(int i = ind -1; (i > -1) && (!found); i--){
                if(config->groups[i].status == RTR_MGR_CLOSED){
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
    else if(state == RTR_ERROR_NO_INCR_UPDATE_AVAIL){
        config->groups[ind].status = RTR_MGR_ERROR;
        //find a group with a lower preference value, if no other group exists, do nothing,
        //if it is the only active group it will get the status ESTABLISHED after the connection was restablished
        int next_config = ind - 1;
        while(next_config >= 0 && config->groups[next_config].status != RTR_MGR_CLOSED){
            next_config--;
        }
        if(next_config >= 0)
            rtr_mgr_start_sockets(&(config->groups[next_config]));
    }

    pthread_mutex_unlock(&(config->mutex));
}

int rtr_mgr_config_cmp(const void* a, const void* b){
    const rtr_mgr_group* ar = a;
    const rtr_mgr_group* br = b;
    if(ar->preference > br->preference)
        return 1;
    else if(ar->preference < br->preference)
        return RTR_ERROR;
    return RTR_SUCCESS;
}

int rtr_mgr_init(rtr_mgr_config* config, const unsigned int polling_period, const unsigned int cache_timeout, pfx_update_fp update_fp){
    if(pthread_mutex_init(&(config->mutex), NULL) != 0)
        MGR_DBG1("Mutex initialization failed");

    //sort array in asc preference order
    qsort(config->groups, config->len, sizeof(rtr_mgr_group), &rtr_mgr_config_cmp);

    for(unsigned int i = 0; i < config->len; i++){
        if(config->groups[i].sockets_len == 0){
            MGR_DBG1("Error Socket group contains an empty sockets array");
            return RTR_ERROR;
        }
        if(i > 0 && config->groups[i-1].preference == config->groups[i].preference){
            MGR_DBG1("Error Socket group contains 2 members with the same preference value");
            return RTR_ERROR;
        }
    }

    pfx_table* pfxt = malloc(sizeof(pfx_table));
    if(pfxt == NULL)
        return RTR_ERROR;
    pfx_table_init(pfxt, update_fp);

    for(unsigned int i = 0; i < config->len; i++){
        config->groups[i].status = RTR_MGR_CLOSED;
        for(unsigned int j = 0; j < config->groups[i].sockets_len; j++){
            rtr_init(config->groups[i].sockets[j], NULL, pfxt, polling_period, cache_timeout, rtr_mgr_cb, config);
        }
    }
    return RTR_SUCCESS;
}

int rtr_mgr_start(rtr_mgr_config* config){
    return rtr_mgr_start_sockets(&(config->groups[0]));
}
bool rtr_mgr_conf_in_sync(rtr_mgr_config* config){
    for(unsigned int i = 0; i < config->len; i++){
        bool all_sync = true;
        for(unsigned int j = 0; all_sync && (j < config->groups[i].sockets_len); j++){
            if(config->groups[i].sockets[j]->last_update == 0)
                all_sync = false;
        }
        if(all_sync)
            return true;
    }
    return false;
}

void rtr_mgr_free(rtr_mgr_config* config){
    pthread_mutex_lock(&(config->mutex));
    pfx_table_free(config->groups[0].sockets[0]->pfx_table);
    free(config->groups[0].sockets[0]->pfx_table);
    pthread_mutex_unlock(&(config->mutex));
    pthread_mutex_destroy(&(config->mutex));
}

inline int rtr_mgr_validate(rtr_mgr_config* config, const uint32_t asn, const ip_addr* prefix, const uint8_t mask_len, pfxv_state* result){
    return pfx_table_validate(config->groups[0].sockets[0]->pfx_table, asn, prefix, mask_len, result);
}

void rtr_mgr_stop(rtr_mgr_config* config){
    pthread_mutex_lock(&(config->mutex));
    for(unsigned int i = 0; i < config->len; i++){
        for(unsigned int j = 0; j < config->groups[i].sockets_len; j++){
            rtr_stop(config->groups[i].sockets[j]);
            MGR_DBG("Group(%u) status changed to: CLOSED", config->groups[i].preference);
        }
        config->groups[i].status = RTR_MGR_CLOSED;
    }
    pthread_mutex_unlock(&(config->mutex));
}
