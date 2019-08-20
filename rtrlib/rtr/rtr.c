/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "rtr_private.h"

#include "rtrlib/lib/log_private.h"
#include "rtrlib/lib/utils_private.h"
#include "rtrlib/pfx/pfx_private.h"
#include "rtrlib/rtr/packets_private.h"
#include "rtrlib/rtrlib_export_private.h"
#include "rtrlib/spki/hashtable/ht-spkitable_private.h"
#include "rtrlib/transport/transport_private.h"

#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

static void rtr_purge_outdated_records(struct rtr_socket *rtr_socket);
static void *rtr_fsm_start(struct rtr_socket *rtr_socket);

static const char *socket_str_states[] = {[RTR_CONNECTING] = "RTR_CONNECTING",
					  [RTR_ESTABLISHED] = "RTR_ESTABLISHED",
					  [RTR_RESET] = "RTR_RESET",
					  [RTR_SYNC] = "RTR_SYNC",
					  [RTR_FAST_RECONNECT] = "RTR_FAST_RECONNECT",
					  [RTR_ERROR_NO_DATA_AVAIL] = "RTR_ERROR_NO_DATA_AVAIL",
					  [RTR_ERROR_NO_INCR_UPDATE_AVAIL] = "RTR_ERROR_NO_INCR_UPDATE_AVAIL",
					  [RTR_ERROR_FATAL] = "RTR_ERROR_FATAL",
					  [RTR_ERROR_TRANSPORT] = "RTR_ERROR_TRANSPORT",
					  [RTR_SHUTDOWN] = "RTR_SHUTDOWN"};

int rtr_init(struct rtr_socket *rtr_socket, struct tr_socket *tr, struct pfx_table *pfx_table,
	     struct spki_table *spki_table, const unsigned int refresh_interval, const unsigned int expire_interval,
	     const unsigned int retry_interval, enum rtr_interval_mode iv_mode, rtr_connection_state_fp fp,
	     void *fp_param_config, void *fp_param_group)
{
	if (tr != NULL)
		rtr_socket->tr_socket = tr;

	// Check if one of the intervals is not in range of the predefined values.
	if (rtr_check_interval_range(refresh_interval, RTR_REFRESH_MIN, RTR_REFRESH_MAX) != RTR_INSIDE_INTERVAL_RANGE ||
	    rtr_check_interval_range(expire_interval, RTR_EXPIRATION_MIN, RTR_EXPIRATION_MAX) !=
		    RTR_INSIDE_INTERVAL_RANGE ||
	    rtr_check_interval_range(retry_interval, RTR_RETRY_MIN, RTR_RETRY_MAX) != RTR_INSIDE_INTERVAL_RANGE) {
		RTR_DBG("Interval value not in range.");
		return RTR_INVALID_PARAM;
	}
	rtr_socket->refresh_interval = refresh_interval;
	rtr_socket->expire_interval = expire_interval;
	rtr_socket->retry_interval = retry_interval;
	rtr_socket->iv_mode = iv_mode;

	rtr_socket->state = RTR_CLOSED;
	rtr_socket->request_session_id = true;
	rtr_socket->serial_number = 0;
	rtr_socket->last_update = 0;
	rtr_socket->pfx_table = pfx_table;
	rtr_socket->spki_table = spki_table;
	rtr_socket->connection_state_fp = fp;
	rtr_socket->connection_state_fp_param_config = fp_param_config;
	rtr_socket->connection_state_fp_param_group = fp_param_group;
	rtr_socket->thread_id = 0;
	rtr_socket->version = RTR_PROTOCOL_MAX_SUPPORTED_VERSION;
	rtr_socket->has_received_pdus = false;
	rtr_socket->is_resetting = false;
	return RTR_SUCCESS;
}

int rtr_start(struct rtr_socket *rtr_socket)
{
	if (rtr_socket->thread_id)
		return RTR_ERROR;

	int rtval = pthread_create(&(rtr_socket->thread_id), NULL, (void *(*)(void *)) & rtr_fsm_start, rtr_socket);
	if (rtval == 0)
		return RTR_SUCCESS;
	return RTR_ERROR;
}

void rtr_purge_outdated_records(struct rtr_socket *rtr_socket)
{
	if (rtr_socket->last_update == 0)
		return;
	time_t cur_time;
	int rtval = lrtr_get_monotonic_time(&cur_time);
	if (rtval == -1 || (rtr_socket->last_update + rtr_socket->expire_interval) < cur_time) {
		if (rtval == -1)
			RTR_DBG1("get_monotic_time(..) failed");
		pfx_table_src_remove(rtr_socket->pfx_table, rtr_socket);
		RTR_DBG1("Removed outdated records from pfx_table");
		spki_table_src_remove(rtr_socket->spki_table, rtr_socket);
		RTR_DBG1("Removed outdated router keys from spki_table");
		rtr_socket->request_session_id = true;
		rtr_socket->serial_number = 0;
		rtr_socket->last_update = 0;
		rtr_socket->is_resetting = true;
	}
}

/* WARNING: This Function has cancelable sections*/
void *rtr_fsm_start(struct rtr_socket *rtr_socket)
{
	if (rtr_socket->state == RTR_SHUTDOWN)
		return NULL;

	// We don't care about the old state, but POSIX demands a non null value for setcancelstate
	int oldcancelstate;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldcancelstate);

	rtr_socket->state = RTR_CONNECTING;
	while (1) {
		if (rtr_socket->state == RTR_CONNECTING) {
			RTR_DBG1("State: RTR_CONNECTING");
			rtr_socket->has_received_pdus = false;

			// old pfx_record could exists in the pfx_table, check if they are too old and must be removed old key_entry
			// could exists in the spki_table, check if they are too old and must be removed
			rtr_purge_outdated_records(rtr_socket);

			if (tr_open(rtr_socket->tr_socket) == TR_ERROR) {
				rtr_change_socket_state(rtr_socket, RTR_ERROR_TRANSPORT);
			} else if (rtr_socket->request_session_id) {
				// change to state RESET, if socket dont has a session_id
				rtr_change_socket_state(rtr_socket, RTR_RESET);
			} else {
				// if we already have a session_id, send a serial query and start to sync
				if (rtr_send_serial_query(rtr_socket) == RTR_SUCCESS)
					rtr_change_socket_state(rtr_socket, RTR_SYNC);
				else
					rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
			}

		}

		else if (rtr_socket->state == RTR_RESET) {
			RTR_DBG1("State: RTR_RESET");
			if (rtr_send_reset_query(rtr_socket) == RTR_SUCCESS) {
				RTR_DBG1("rtr_start: reset pdu sent");
				rtr_change_socket_state(rtr_socket,
							RTR_SYNC); // start to sync after connection is established
			}
		}

		else if (rtr_socket->state == RTR_SYNC) {
			RTR_DBG1("State: RTR_SYNC");
			if (rtr_sync(rtr_socket) == RTR_SUCCESS)
				rtr_change_socket_state(
					rtr_socket,
					RTR_ESTABLISHED); // wait for next sync after first successful sync
		}

		else if (rtr_socket->state == RTR_ESTABLISHED) {
			RTR_DBG1("State: RTR_ESTABLISHED");

			// Allow thread cancellation for recv code path only.
			// This should be enough since we spend most of the time blocking on recv
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldcancelstate);
			int ret = rtr_wait_for_sync(
				rtr_socket); // blocks till expire_interval is expired or PDU was received
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldcancelstate);

			if (ret == RTR_SUCCESS) { // send serial query
				if (rtr_send_serial_query(rtr_socket) == RTR_SUCCESS)
					rtr_change_socket_state(rtr_socket, RTR_SYNC);
			}
		}

		else if (rtr_socket->state == RTR_FAST_RECONNECT) {
			RTR_DBG1("State: RTR_FAST_RECONNECT");
			tr_close(rtr_socket->tr_socket);
			rtr_change_socket_state(rtr_socket, RTR_CONNECTING);
		}

		else if (rtr_socket->state == RTR_ERROR_NO_DATA_AVAIL) {
			RTR_DBG1("State: RTR_ERROR_NO_DATA_AVAIL");
			rtr_socket->request_session_id = true;
			rtr_socket->serial_number = 0;
			rtr_change_socket_state(rtr_socket, RTR_RESET);
			sleep(rtr_socket->retry_interval);
			rtr_purge_outdated_records(rtr_socket);
		}

		else if (rtr_socket->state == RTR_ERROR_NO_INCR_UPDATE_AVAIL) {
			RTR_DBG1("State: RTR_ERROR_NO_INCR_UPDATE_AVAIL");
			rtr_socket->request_session_id = true;
			rtr_socket->serial_number = 0;
			rtr_change_socket_state(rtr_socket, RTR_RESET);
			rtr_purge_outdated_records(rtr_socket);
		}

		else if (rtr_socket->state == RTR_ERROR_TRANSPORT) {
			RTR_DBG1("State: RTR_ERROR_TRANSPORT");
			tr_close(rtr_socket->tr_socket);
			rtr_change_socket_state(rtr_socket, RTR_CONNECTING);
			RTR_DBG("Waiting %u", rtr_socket->retry_interval);
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldcancelstate);
			sleep(rtr_socket->retry_interval);
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldcancelstate);
		}

		else if (rtr_socket->state == RTR_ERROR_FATAL) {
			RTR_DBG1("State: RTR_ERROR_FATAL");
			tr_close(rtr_socket->tr_socket);
			rtr_change_socket_state(rtr_socket, RTR_CONNECTING);
			RTR_DBG("Waiting %u", rtr_socket->retry_interval);
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldcancelstate);
			sleep(rtr_socket->retry_interval);
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldcancelstate);
		}

		else if (rtr_socket->state == RTR_SHUTDOWN) {
			RTR_DBG1("State: RTR_SHUTDOWN");
			pthread_exit(NULL);
		}
	}
}

void rtr_stop(struct rtr_socket *rtr_socket)
{
	RTR_DBG1("rtr_stop()");
	rtr_change_socket_state(rtr_socket, RTR_SHUTDOWN);
	if (rtr_socket->thread_id != 0) {
		RTR_DBG1("pthread_cancel()");
		pthread_cancel(rtr_socket->thread_id);
		RTR_DBG1("pthread_join()");
		pthread_join(rtr_socket->thread_id, NULL);

		tr_close(rtr_socket->tr_socket);
		rtr_socket->request_session_id = true;
		rtr_socket->serial_number = 0;
		rtr_socket->last_update = 0;
		pfx_table_src_remove(rtr_socket->pfx_table, rtr_socket);
		spki_table_src_remove(rtr_socket->spki_table, rtr_socket);
		rtr_socket->thread_id = 0;
	}
	RTR_DBG1("Socket shut down");
}

RTRLIB_EXPORT const char *rtr_state_to_str(enum rtr_socket_state state)
{
	return socket_str_states[state];
}

/* cppcheck-suppress unusedFunction */
RTRLIB_EXPORT enum rtr_interval_mode rtr_get_interval_mode(struct rtr_socket *rtr_socket)
{
	return rtr_socket->iv_mode;
}

/* cppcheck-suppress unusedFunction */
RTRLIB_EXPORT void rtr_set_interval_mode(struct rtr_socket *rtr_socket, enum rtr_interval_mode option)
{
	switch (option) {
	case RTR_INTERVAL_MODE_IGNORE_ANY:
	case RTR_INTERVAL_MODE_ACCEPT_ANY:
	case RTR_INTERVAL_MODE_DEFAULT_MIN_MAX:
	case RTR_INTERVAL_MODE_IGNORE_ON_FAILURE:
		rtr_socket->iv_mode = option;
		break;
	default:
		RTR_DBG1("Invalid interval mode. Mode remains unchanged.");
	}
}
