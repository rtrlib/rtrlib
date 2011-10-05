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

#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include "rtrlib/rtr/rtr.h"
#include "rtrlib/rtr/packets.h"
#include "rtrlib/lib/log.h"

static void rtr_fsm_start(rtr_socket* rtr_socket);
static void sighandler(int b){
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

void rtr_init(rtr_socket* rtr_socket, tr_socket* tr, struct pfx_table* pfx_table, const unsigned int polling_period, const unsigned int cache_timeout){
    rtr_socket->tr_socket = tr;
    assert(polling_period <= 3600);
    rtr_socket->polling_period = (polling_period > (3600 - RTR_RECV_TIMEOUT) ? (polling_period - RTR_RECV_TIMEOUT) : polling_period);
    //rtr_socket->cache_timeout = 2 * polling_period;
    rtr_socket->cache_timeout = cache_timeout;
    rtr_socket->state = RTR_CLOSED;
    rtr_socket->nonce = -1;
    rtr_socket->serial_number = 0;
    rtr_socket->last_update = 0;
    rtr_socket->pfx_table = pfx_table;
    rtr_socket->connection_state_fp = NULL;
    rtr_socket->thread_id = 0;
    rtr_socket->mgr_config = NULL;
    rtr_socket->mgr_config_len = 0;
}

int rtr_start(rtr_socket* rtr_socket){
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int rtval = pthread_create(&(rtr_socket->thread_id), &attr, (void * (*)(void *)) &rtr_fsm_start, rtr_socket);
    pthread_attr_destroy(&attr);
    if(rtval == 0)
        return RTR_SUCCESS;
    return RTR_ERROR;
}

void rtr_fsm_start(rtr_socket* rtr_socket){
    install_sig_handler();
    while(1){
        if(rtr_socket->state == RTR_CLOSED){
            dbg("%s", "State: RTR_CLOSED");
            dbg("%s", "rtr_start: open transport connection");
            if(tr_open(rtr_socket->tr_socket) == TR_ERROR){
                rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
            }
            else
                rtr_change_socket_state(rtr_socket, RTR_RESET); //send reset query after connection is established
        }

        else if(rtr_socket->state == RTR_RESET){
            dbg("%s", "State: RTR_RESET");
            if (rtr_send_reset_query(rtr_socket) == 0){
                dbg("%s", "rtr_start: reset pdu sent");
                rtr_change_socket_state(rtr_socket, RTR_SYNC); //send reset query after connection established
            }
        }

        else if(rtr_socket->state == RTR_SYNC){
            dbg("%s", "State: RTR_SYNC");
            if(rtr_sync(rtr_socket) == 0)
                rtr_change_socket_state(rtr_socket, RTR_ESTABLISHED); //send reset query after connection established
        }

        else if(rtr_socket->state == RTR_ESTABLISHED){
            dbg("%s", "State: RTR_ESTABLISHED");
            if(rtr_wait_for_sync(rtr_socket) == RTR_SUCCESS){ //blockiert bis cache_timout abgelaufen ist oder Paket eingetroffen ist
                //serial query senden
                if(rtr_send_serial_query(rtr_socket) == RTR_SUCCESS)
                    rtr_change_socket_state(rtr_socket, RTR_SYNC);
            }
        }

        else if(rtr_socket->state == RTR_ERROR_NO_DATA_AVAIL){
            dbg("%s", "State: RTR_ERROR_NO_DATA_AVAIL");
            //retry every 30sec to fetch data
            sleep(30);
            rtr_socket->nonce = -1;
            rtr_socket->serial_number = 0;
            pfx_table_remove_from_origin(rtr_socket->pfx_table, (uintptr_t) rtr_socket);
            rtr_change_socket_state(rtr_socket, RTR_RESET); 
        }

        else if(rtr_socket->state == RTR_ERROR_NO_INCR_UPDATE_AVAIL){
            dbg("%s", "State: RTR_ERROR_NO_INCR_UPDATE_AVAIL");
            rtr_socket->nonce = -1;
            pfx_table_remove_from_origin(rtr_socket->pfx_table, (uintptr_t) rtr_socket);
            rtr_change_socket_state(rtr_socket, RTR_RESET);
            //TODO: verbindung nicht schliessen sondern beim nächsten versucht weiter benutzen?
        }

        else if(rtr_socket->state == RTR_ERROR_TRANSPORT){
            dbg("%s", "State: RTR_ERROR_TRANSPORT");
            //nonce wont be resetted and reused for next reconnection, nonce nur behalten wenn das letzte reset query
            //kürzer als 60min her war
            tr_close(rtr_socket->tr_socket);
            return;
            //TODO
        }

        else if(rtr_socket->state == RTR_ERROR_FATAL){
            dbg("%s", "State: RTR_ERROR_FATAL");
            tr_close(rtr_socket->tr_socket);
            rtr_socket->nonce = -1;
            rtr_socket->serial_number = 0;
            //TODO: pfx_table daten löschen
            return;
        }

        else if(rtr_socket->state == RTR_SHUTDOWN){
            dbg("%s", "State: RTR_SHUTDOWN");
            tr_close(rtr_socket->tr_socket);
            rtr_socket->nonce = -1;
            rtr_socket->serial_number = 0;
            //TODO: pfx_table daten löschen
            //void pfx_table_remove_from_origin(struct pfx_table* pfx_table, unsigned int rtr_socket_id);
            pthread_exit(NULL);
        }
    }
}

void rtr_stop(rtr_socket* rtr_socket){
    rtr_change_socket_state(rtr_socket, RTR_SHUTDOWN);
    if(rtr_socket->thread_id != 0)
        pthread_kill(rtr_socket->thread_id, SIGUSR1);
}
