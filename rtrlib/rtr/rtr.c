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

#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "rtrlib/lib/log.h"
#include "rtrlib/rtr/packets.h"
#include "rtrlib/rtr/rtr.h"
#include "rtrlib/lib/utils.h"

const unsigned int ERR_TIMEOUT = 20;

static void rtr_purge_outdated_records(rtr_socket* rtr_socket);
static void rtr_fsm_start(rtr_socket* rtr_socket);
static void sighandler(int b);
static int install_sig_handler();

void sighandler(int b __attribute__((unused)) ){
    return;
}

int install_sig_handler(){
    struct sigaction sa;
    sa.sa_handler = &sighandler;
    sigset_t mask;
    sigemptyset(&mask);
    sa.sa_mask = mask; 
    sa.sa_flags = 0;

    return sigaction(SIGUSR1, &sa, NULL);
}

void rtr_init(rtr_socket* rtr_socket, tr_socket* tr, struct pfx_table* pfx_table, const unsigned int polling_period, const unsigned int cache_timeout, rtr_connection_state_fp fp, void* fp_param){
    if(tr != NULL)
        rtr_socket->tr_socket = tr;
    assert(polling_period <= 3600);
    rtr_socket->polling_period = (polling_period > (3600 - RTR_RECV_TIMEOUT) ? (3600 - RTR_RECV_TIMEOUT) : polling_period);
    rtr_socket->cache_timeout = (cache_timeout == 0 ? (rtr_socket->polling_period / 2) : cache_timeout);
    rtr_socket->state = RTR_SHUTDOWN;
    rtr_socket->request_nonce = true;
    rtr_socket->serial_number = 0;
    rtr_socket->last_update = 0;
    rtr_socket->pfx_table = pfx_table;
    rtr_socket->connection_state_fp = fp;
    rtr_socket->connection_state_fp_param = fp_param;
    rtr_socket->thread_id = 0;
}

int rtr_start(rtr_socket* rtr_socket){
    rtr_socket->state = RTR_CONNECTING;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int rtval = pthread_create(&(rtr_socket->thread_id), &attr, (void* (*)(void*)) &rtr_fsm_start, rtr_socket);
    pthread_attr_destroy(&attr);
    if(rtval == 0)
        return RTR_SUCCESS;
    return RTR_ERROR;
}

void rtr_purge_outdated_records(rtr_socket* rtr_socket){
    if(rtr_socket->last_update == 0)
        return;
    time_t cur_time;
    int rtval = get_monotonic_time(&cur_time);
    if(rtval == -1 || (rtr_socket->last_update + rtr_socket->cache_timeout) > cur_time){
        if(rtval == -1)
            RTR_DBG1("get_monotic_time(..) failed");
        pfx_table_src_remove(rtr_socket->pfx_table, (uintptr_t) rtr_socket);
        RTR_DBG1("Removed outdated records from pfx_table");
        rtr_socket->request_nonce = true;
        rtr_socket->serial_number = 0;
        rtr_socket->last_update = 0;
    }
}

void rtr_fsm_start(rtr_socket* rtr_socket){
    install_sig_handler();
    while(1){
        if(rtr_socket->state == RTR_CONNECTING){
            RTR_DBG1("State: RTR_CONNECTING");
            //old pfx_record could exists in the pfx_table, check if they are too old and must be removed
            rtr_purge_outdated_records(rtr_socket);

            RTR_DBG1("rtr_start: open transport connection");
            if(tr_open(rtr_socket->tr_socket) == TR_ERROR){
                rtr_change_socket_state(rtr_socket, RTR_ERROR_TRANSPORT);
            }
            else
                if(rtr_socket->request_nonce){
                    //change to state RESET, if socket dont has a nonce
                    rtr_change_socket_state(rtr_socket, RTR_RESET);
                }
                else{
                    //if we already have a nonce, send a serial query and start to sync
                    if(rtr_send_serial_query(rtr_socket) == RTR_SUCCESS)
                        rtr_change_socket_state(rtr_socket, RTR_SYNC);
                    else
                        rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
                }
        }

        else if(rtr_socket->state == RTR_RESET){
            RTR_DBG1("State: RTR_RESET");
            if (rtr_send_reset_query(rtr_socket) == 0){
                RTR_DBG1("rtr_start: reset pdu sent");
                rtr_change_socket_state(rtr_socket, RTR_SYNC); //send reset query after connection established
            }
        }

        else if(rtr_socket->state == RTR_SYNC){
            RTR_DBG1("State: RTR_SYNC");
            if(rtr_sync(rtr_socket) == 0)
                rtr_change_socket_state(rtr_socket, RTR_ESTABLISHED); //send reset query after connection established
        }

        else if(rtr_socket->state == RTR_ESTABLISHED){
            RTR_DBG1("State: RTR_ESTABLISHED");
            if(rtr_wait_for_sync(rtr_socket) == RTR_SUCCESS){ //blockiert bis cache_timout abgelaufen ist oder Paket eingetroffen ist
                //serial query senden
                if(rtr_send_serial_query(rtr_socket) == RTR_SUCCESS)
                    rtr_change_socket_state(rtr_socket, RTR_SYNC);
            }
        }

        else if(rtr_socket->state == RTR_ERROR_NO_DATA_AVAIL){
            RTR_DBG1("State: RTR_ERROR_NO_DATA_AVAIL");
            rtr_socket->request_nonce = true;
            rtr_socket->serial_number = 0;
            rtr_change_socket_state(rtr_socket, RTR_RESET);
            sleep(ERR_TIMEOUT);
            rtr_purge_outdated_records(rtr_socket);
        }

        else if(rtr_socket->state == RTR_ERROR_NO_INCR_UPDATE_AVAIL){
            RTR_DBG1("State: RTR_ERROR_NO_INCR_UPDATE_AVAIL");
            rtr_socket->request_nonce = true;
            rtr_socket->serial_number = 0;
            rtr_change_socket_state(rtr_socket, RTR_RESET);
            sleep(ERR_TIMEOUT);
            rtr_purge_outdated_records(rtr_socket);
        }

        else if(rtr_socket->state == RTR_ERROR_TRANSPORT){
            RTR_DBG1("State: RTR_ERROR_TRANSPORT");
            tr_close(rtr_socket->tr_socket);
            rtr_change_socket_state(rtr_socket, RTR_CONNECTING);
            sleep(ERR_TIMEOUT);
        }

        else if(rtr_socket->state == RTR_ERROR_FATAL){
            RTR_DBG1("State: RTR_ERROR_FATAL");
            tr_close(rtr_socket->tr_socket);
            rtr_change_socket_state(rtr_socket, RTR_CONNECTING);
            sleep(ERR_TIMEOUT);
        }

        else if(rtr_socket->state == RTR_SHUTDOWN){
            RTR_DBG1("State: RTR_SHUTDOWN");
            tr_close(rtr_socket->tr_socket);
            rtr_socket->request_nonce = true;
            rtr_socket->serial_number = 0;
            rtr_socket->last_update = 0;
            pfx_table_src_remove(rtr_socket->pfx_table, (uintptr_t) rtr_socket);
            pthread_exit(NULL);
        }
    }
}

void rtr_stop(rtr_socket* rtr_socket){
    rtr_change_socket_state(rtr_socket, RTR_SHUTDOWN);
    if(rtr_socket->thread_id != 0)
        pthread_kill(rtr_socket->thread_id, SIGUSR1);
}
