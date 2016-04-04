/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
*/

#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "rtrlib/lib/log.h"
#include "rtrlib/rtr/packets.h"
#include "rtrlib/spki/hashtable/ht-spkitable.h"
#include "rtrlib/rtr/rtr.h"
#include "rtrlib/lib/utils.h"

static void rtr_purge_outdated_records(struct rtr_socket *rtr_socket);
static void rtr_fsm_start(struct rtr_socket *rtr_socket);
static void sighandler(int b);
static int install_sig_handler();

static const char *socket_str_states[] = {
    [RTR_CONNECTING] = "RTR_CONNECTING",
    [RTR_ESTABLISHED] = "RTR_ESTABLISHED",
    [RTR_RESET] = "RTR_RESET",
    [RTR_SYNC] = "RTR_SYNC",
    [RTR_FAST_RECONNECT] = "RTR_FAST_RECONNECT",
    [RTR_ERROR_NO_DATA_AVAIL] = "RTR_ERROR_NO_DATA_AVAIL",
    [RTR_ERROR_NO_INCR_UPDATE_AVAIL] = "RTR_ERROR_NO_INCR_UPDATE_AVAIL",
    [RTR_ERROR_FATAL] = "RTR_ERROR_FATAL",
    [RTR_ERROR_TRANSPORT] = "RTR_ERROR_TRANSPORT",
    [RTR_SHUTDOWN] = "RTR_SHUTDOWN"
};

void sighandler(int b __attribute__((unused)) )
{
    return;
}

static int install_sig_handler()
{
    struct sigaction sa;
    sa.sa_handler = &sighandler;
    sigset_t mask;
    sigemptyset(&mask);
    sa.sa_mask = mask;
    sa.sa_flags = 0;

    return sigaction(SIGUSR1, &sa, NULL);
}

int rtr_init(struct rtr_socket *rtr_socket,
              struct tr_socket *tr,
              struct pfx_table *pfx_table,
              struct spki_table *spki_table,
              const unsigned int refresh_interval,
              const unsigned int expire_interval,
              const unsigned int retry_interval,
              rtr_connection_state_fp fp, void *fp_param)
{
    if(tr != NULL)
        rtr_socket->tr_socket = tr;

    if(refresh_interval > 86400 || refresh_interval < 1) {
        return RTR_INVALID_PARAM;
    } else {
        rtr_socket->refresh_interval = refresh_interval;
    }

    if ((expire_interval < 600) || (expire_interval > 172800)) {
        return RTR_INVALID_PARAM;
    } else {
        rtr_socket->expire_interval = expire_interval;
    }

    if ((retry_interval > 7200) || (retry_interval < 1)) {
        return RTR_INVALID_PARAM;
    } else {
        rtr_socket->retry_interval = retry_interval;
    }

    rtr_socket->state = RTR_SHUTDOWN;
    rtr_socket->request_session_id = true;
    rtr_socket->serial_number = 0;
    rtr_socket->last_update = 0;
    rtr_socket->pfx_table = pfx_table;
    rtr_socket->spki_table = spki_table;
    rtr_socket->connection_state_fp = fp;
    rtr_socket->connection_state_fp_param = fp_param;
    rtr_socket->thread_id = 0;
    rtr_socket->version = RTR_PROTOCOL_MAX_SUPPORTED_VERSION;
    rtr_socket->has_received_pdus = false;
    return RTR_SUCCESS;
}

int rtr_start(struct rtr_socket *rtr_socket)
{
    if(rtr_socket->thread_id)
        return RTR_ERROR;

    int rtval = pthread_create(&(rtr_socket->thread_id), NULL, (void* ( *)(void *)) &rtr_fsm_start, rtr_socket);
    if(rtval == 0)
        return RTR_SUCCESS;
    return RTR_ERROR;
}

void rtr_purge_outdated_records(struct rtr_socket *rtr_socket)
{
    if(rtr_socket->last_update == 0)
        return;
    time_t cur_time;
    int rtval = lrtr_get_monotonic_time(&cur_time);
    if(rtval == -1 || (rtr_socket->last_update + rtr_socket->expire_interval) < cur_time) {
        if(rtval == -1)
            RTR_DBG1("get_monotic_time(..) failed");
        pfx_table_src_remove(rtr_socket->pfx_table, rtr_socket);
        RTR_DBG1("Removed outdated records from pfx_table");
        spki_table_src_remove(rtr_socket->spki_table, rtr_socket);
        RTR_DBG1("Removed outdated router keys from spki_table");
        rtr_socket->request_session_id = true;
        rtr_socket->serial_number = 0;
        rtr_socket->last_update = 0;
    }
}

void rtr_fsm_start(struct rtr_socket *rtr_socket)
{
    rtr_socket->state = RTR_CONNECTING;
    install_sig_handler();
    while(1) {
        if(rtr_socket->state == RTR_CONNECTING) {
            RTR_DBG1("State: RTR_CONNECTING");
            rtr_socket->has_received_pdus = false;

            //old pfx_record could exists in the pfx_table, check if they are too old and must be removed
            //old key_entry could exists in the spki_table, check if they are too old and must be removed
            rtr_purge_outdated_records(rtr_socket);

            if(tr_open(rtr_socket->tr_socket) == TR_ERROR) {
                rtr_change_socket_state(rtr_socket, RTR_ERROR_TRANSPORT);
            } else if(rtr_socket->request_session_id) {
                //change to state RESET, if socket dont has a session_id
                rtr_change_socket_state(rtr_socket, RTR_RESET);
            } else {
                //if we already have a session_id, send a serial query and start to sync
                if(rtr_send_serial_query(rtr_socket) == RTR_SUCCESS)
                    rtr_change_socket_state(rtr_socket, RTR_SYNC);
                else
                    rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
            }

        }

        else if(rtr_socket->state == RTR_RESET) {
            RTR_DBG1("State: RTR_RESET");
            if (rtr_send_reset_query(rtr_socket) == RTR_SUCCESS) {
                RTR_DBG1("rtr_start: reset pdu sent");
                rtr_change_socket_state(rtr_socket, RTR_SYNC); //start to sync after connection is established
            }
        }

        else if(rtr_socket->state == RTR_SYNC) {
            RTR_DBG1("State: RTR_SYNC");
            if(rtr_sync(rtr_socket) == RTR_SUCCESS)
                rtr_change_socket_state(rtr_socket, RTR_ESTABLISHED); //wait for next sync after first successful sync
        }

        else if(rtr_socket->state == RTR_ESTABLISHED) {
            RTR_DBG1("State: RTR_ESTABLISHED");
            if(rtr_wait_for_sync(rtr_socket) == RTR_SUCCESS) { //blocks till expire_interval is expired or PDU was received
                //send serial query
                if(rtr_send_serial_query(rtr_socket) == RTR_SUCCESS)
                    rtr_change_socket_state(rtr_socket, RTR_SYNC);
            }
        }

        else if(rtr_socket->state == RTR_FAST_RECONNECT){
            RTR_DBG1("State: RTR_FAST_RECONNECT");
            tr_close(rtr_socket->tr_socket);
            rtr_change_socket_state(rtr_socket, RTR_CONNECTING);
        }

        else if(rtr_socket->state == RTR_ERROR_NO_DATA_AVAIL) {
            RTR_DBG1("State: RTR_ERROR_NO_DATA_AVAIL");
            rtr_socket->request_session_id = true;
            rtr_socket->serial_number = 0;
            rtr_change_socket_state(rtr_socket, RTR_RESET);
            sleep(rtr_socket->retry_interval);
            rtr_purge_outdated_records(rtr_socket);
        }

        else if(rtr_socket->state == RTR_ERROR_NO_INCR_UPDATE_AVAIL) {
            RTR_DBG1("State: RTR_ERROR_NO_INCR_UPDATE_AVAIL");
            rtr_socket->request_session_id = true;
            rtr_socket->serial_number = 0;
            rtr_change_socket_state(rtr_socket, RTR_RESET);
            rtr_purge_outdated_records(rtr_socket);
        }

        else if(rtr_socket->state == RTR_ERROR_TRANSPORT) {
            RTR_DBG1("State: RTR_ERROR_TRANSPORT");
            tr_close(rtr_socket->tr_socket);
            rtr_change_socket_state(rtr_socket, RTR_CONNECTING);
            RTR_DBG("Waiting %u", rtr_socket->retry_interval);
            sleep(rtr_socket->retry_interval);
        }

        else if(rtr_socket->state == RTR_ERROR_FATAL) {
            RTR_DBG1("State: RTR_ERROR_FATAL");
            tr_close(rtr_socket->tr_socket);
            rtr_change_socket_state(rtr_socket, RTR_CONNECTING);
            RTR_DBG("Waiting %u", rtr_socket->retry_interval);
            sleep(rtr_socket->retry_interval);
        }

        else if(rtr_socket->state == RTR_SHUTDOWN) {
            RTR_DBG1("State: RTR_SHUTDOWN");
            tr_close(rtr_socket->tr_socket);
            rtr_socket->request_session_id = true;
            rtr_socket->serial_number = 0;
            rtr_socket->last_update = 0;
            pfx_table_src_remove(rtr_socket->pfx_table, rtr_socket);
            spki_table_src_remove(rtr_socket->spki_table, rtr_socket);
            pthread_exit(NULL);
        }
    }
}

void rtr_stop(struct rtr_socket *rtr_socket)
{
    RTR_DBG1("rtr_stop()");
    rtr_change_socket_state(rtr_socket, RTR_SHUTDOWN);
    if(rtr_socket->thread_id != 0) {
        RTR_DBG1("pthread_kill()");
        pthread_kill(rtr_socket->thread_id, SIGUSR1);
        RTR_DBG1("pthread_join()");
        pthread_join(rtr_socket->thread_id, NULL);
        rtr_socket->thread_id = 0;
    }
    RTR_DBG1("Socket shut down");
}

const char *rtr_state_to_str(enum rtr_socket_state state)
{
    return socket_str_states[state];
}
