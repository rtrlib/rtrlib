#include "rtrlib/rtr_mgr.h"
#include "rtrlib/pfx/lpfst/lpfst-pfx.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "rtrlib/lib/log.h"

#define MGR_DBG(fmt, ...) dbg("RTR_MGR: " fmt, ## __VA_ARGS__)
#define MGR_DBG1(a) dbg("RTR_MGR: " a)

static int find_config(rtr_mgr_config config[], unsigned int config_len, const rtr_socket* sock, unsigned int* ind);
static int start_sockets(rtr_mgr_config* config);
static int config_cmp(const void* a, const void* b);

int start_sockets(rtr_mgr_config* config){
    for(unsigned int i = 0; i < config->sockets_len; i++){
        if(rtr_start(config->sockets[i]) != 0){
            MGR_DBG1("rtr_mgr: Error starting rtr_socket pthread");
            return -1;
        }
    }
    config->status = CONNECTING;
    MGR_DBG("Group(%u) status changed to: CONNECTING", config->preference);
    return 0;
}

int find_config(rtr_mgr_config config[], unsigned int config_len, const rtr_socket* sock, unsigned int* ind){
    for(unsigned int i = 0; i < config_len; i++){
        for(unsigned int j = 0; j < config[i].sockets_len; j++){
            if(config[i].sockets[j] == sock){
                *ind = i;
                return 0;
            }
        }
    }
    MGR_DBG1("Error couldn't find a wanted rtr_socket in rtr_mgr_config");
    return -1;
}

void rtr_mgr_cb(const rtr_socket* sock, const rtr_socket_state state, rtr_mgr_config config[], unsigned int config_len){
    //TODO: locks needed?

    //return if group contains no other socket groups => nothing todo :-)
    if(config_len == 1){
        MGR_DBG1("rtr_mgr_config contains no other rtr_socket groups");
        return;
    }

    //find the index in the rtr_mgr_config struct, for which this function was called
    unsigned int ind = 0;
    if(find_config(config, config_len, sock, &ind) == -1)
        return;
    if(state == RTR_ESTABLISHED || state == RTR_RESET || state == RTR_SYNC){
    //socket established successfull a connection to the rtr_server
        if(config[ind].status == CONNECTING){
            //if previous state was CONNECTING, check if all other sockets in the group also have a established connection,
            //if yes change group state to ESTABLISHED
            bool connected = true;
            for(unsigned int i = 0; i < config[ind].sockets_len; i++){
                rtr_socket_state state = config[ind].sockets[i]->state;
                if(state != RTR_ESTABLISHED && state != RTR_RESET && state != RTR_SYNC){
                    connected = false;
                    break;
                }
            }
            if(connected){
                config[ind].status = ESTABLISHED;
                MGR_DBG("Group(%u) status changed to: ESTABLISHED", config[ind].preference);
                //TODO: Gruppe finden, welche State = Established hat + höchste priorität hat
                //alle anderen socket gruppen beenden
                //
                for(unsigned int i = 0; i < config_len; i++){
                    if(config[i].status != CLOSED && i != ind){
                        for(unsigned int j = 0; j < config[i].sockets_len;j++){
                            rtr_stop(config[i].sockets[j]);
                        }
                        config[i].status = CLOSED;
                        MGR_DBG("Group(%u) status changed to: CLOSED", config[i].preference);
                    }
                }
            }
        }
        if(config[ind].status == ERROR){
            //if previous state was ERROR, only change state to ESTABLISHED if all other socket groups are also in error or SHUTDOWN state
            bool all_error = true;
            for(unsigned int i = 0; (i < config_len) && (all_error); i++){
                if(config[i].status != ERROR && config[i].status != CLOSED)
                    all_error = false;
            }
            if(all_error){
                config[ind].status = ESTABLISHED;
                MGR_DBG("Group(%u) status changed to: ESTABLISHED", config[ind].preference);
                for(unsigned int i = 0; i < config_len; i++){
                    if(i != ind && config[i].status != CLOSED){
                        for(unsigned int j = 0; j < config[i].sockets_len;j++){
                            rtr_stop(config[i].sockets[j]);
                        }
                        config[i].status = CLOSED;
                        MGR_DBG("Group(%u) status changed to: CLOSED", config[i].preference);
                    }
                }
            }
        }
    }
    else if(state == RTR_ERROR_FATAL || state == RTR_ERROR_TRANSPORT || state == RTR_ERROR_NO_DATA_AVAIL){
        //if another group exist which has status CLOSED, start connecting to the group
        config[ind].status = ERROR;
        MGR_DBG("Group(%u) status changed to: ERROR", config[ind].preference);

        //find next group with higher preference value
        int next_config = ind + 1;
        while(next_config < (int) config_len && config[next_config].status != CLOSED){
            next_config++;
        }

        //if no unconnected group with higher pref. value exist, find group with lower pref value
        if(next_config >= (int) config_len){
            next_config = ind - 1;
            while(next_config >= 0 && config[next_config].status != CLOSED){
                next_config--;
            }
        }
        if(next_config != (int) ind && config[next_config].status == CLOSED){
            start_sockets(&(config[next_config]));
        }
        else
            MGR_DBG1("No other inactive groups found");
    }
    else if(state == RTR_ERROR_NO_INCR_UPDATE_AVAIL){
        config[ind].status = ERROR;
        //find a group with a lower preference value, if no exists, do nothing,
        //if it is the only active group it will be get status ESTABLISHED after the connection was restablished
        int next_config = ind - 1;
        while(next_config >= 0 && config[next_config].status != CLOSED){
            next_config--;
        }
        if(next_config >= 0)
            start_sockets(&(config[next_config]));
    }
    return;
}


int config_cmp(const void* a, const void* b){
    const rtr_mgr_config* ar = a;
    const rtr_mgr_config* br = b;
    if(ar->preference > br->preference)
        return 1;
    else if(ar->preference < br->preference)
        return -1;
    return 0;
}

void rtr_mgr_init(rtr_mgr_config config[], const unsigned int config_len){
    //sort array in asc preference order
    qsort(config, config_len, sizeof(rtr_mgr_config), &config_cmp);

    for(unsigned int i = 0; i < config_len; i++){
        config[i].status = CLOSED;
        for(unsigned int j = 0; j < config[i].sockets_len; j++){
            config[i].sockets[j]->connection_state_fp = &rtr_mgr_cb;
            config[i].sockets[j]->mgr_config = config;
            config[i].sockets[j]->mgr_config_len = config_len;
        }
    }
}

int rtr_mgr_start(rtr_mgr_config config[]){
    return start_sockets(&(config[0]));
}
bool rtr_mgr_group_in_sync(rtr_mgr_config config[], const unsigned int config_len){
    for(unsigned int i = 0; i < config_len; i++){
        bool all_sync = true;
        for(unsigned int j = 0; all_sync && (j < config[i].sockets_len); j++){
            if(config[i].sockets[j]->last_update != 0)
                all_sync = false;
        }
        if(all_sync)
            return true;
    }
    return false;
}
