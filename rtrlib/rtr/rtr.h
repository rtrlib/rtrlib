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


/**
 * @defgroup mod_rtr_h RTR socket
 * @brief An RTR socket implements the RPKI-RTR protocol scheme.
 * @details One rtr_socket communicates with a single RPKI-RTR server.
 * @{
 */

#ifndef RTR_H
#define RTR_H
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include "rtrlib/pfx/pfx.h"
#include "rtrlib/transport/transport.h"

#define RTR_DBG(fmt, ...) dbg("RTR Socket: " fmt, ## __VA_ARGS__)
#define RTR_DBG1(a) dbg("RTR Socket: " a)

enum rtr_rtvals {
    RTR_SUCCESS = 0,
    RTR_ERROR = -1
};

/**
 * @brief States of the RTR socket.
 */
typedef enum {
    /** Socket is establishing the transport connection. */
    RTR_CONNECTING,

    /** Connection is established, socket is waiting for a Serial Notify or expiration of the polling_period timer */
    RTR_ESTABLISHED,

    /** Resetting RTR connection. */
    RTR_RESET,

    /** Receiving validation records from the RTR server.  */
    RTR_SYNC,

    /** No validation records are available on the RTR server. */
    RTR_ERROR_NO_DATA_AVAIL,

    /** Server was unable to answer the last serial or reset query. */
    RTR_ERROR_NO_INCR_UPDATE_AVAIL,

    /** Fatal protocol error occurred. */
    RTR_ERROR_FATAL,

    /** Error on the transport socket occurred. */
    RTR_ERROR_TRANSPORT,

    /** RTR Socket is stopped. */
    RTR_SHUTDOWN,
} rtr_socket_state;

struct rtr_socket;

/**
 * @brief A function pointer that is called if the state of the rtr socket has changed.
 */
typedef void (*rtr_connection_state_fp)(const struct rtr_socket *rtr_socket, const rtr_socket_state state, void *connection_state_fp_param);

/**
 * @brief A RTR socket.
 * @param tr_socket Pointer to an initialized tr_socket that will be used to communicate with the RTR server.
 * @param polling_period Interval in seconds between serial queries that are sent to the server. Must be <= 3600. If 0
 * is specified the polling_period is set to 300 seconds.
 * @param last_update Timestamp of the last validation record update. Is 0 if the pfx_table doesn't stores any
 * validation reords from this rtr_socket.
 * @param cache_timout Time period in seconds. Received pfx_records are deleted if the client was unable to refresh data for this time period.
 * If 0 is specified, the cache_timeout will be half the polling_period.
 * @param state Current state of the socket.
 * @param session_id session_id of the RTR session.
 * @param request_session_id True, if the rtr_client have to request a new none from the server.
 * @param serial_number Last serial number of the obtained validation records.
 * @param pfx_table pfx_table that stores the validation records obtained from the connected rtr server.
 * @param connection_state_fp A callback function that is executed when the state of the socket changes.
 * @param connection_state_fp_param Parameter that is passed to the connection_state_fp callback.
 */
struct rtr_socket {
    struct tr_socket *tr_socket;
    unsigned int polling_period;
    time_t last_update;
    unsigned int cache_timeout;
    rtr_socket_state state;
    uint32_t session_id;
    bool request_session_id;
    uint32_t serial_number;
    struct pfx_table *pfx_table;
    pthread_t thread_id;
    rtr_connection_state_fp connection_state_fp;
    void *connection_state_fp_param;
};

/**
 * @brief Initializes a rtr_socket.
 * @param[out] rtr_socket Pointer to the allocated rtr_socket that will be initialized.
 * @param[in] tr_socket Pointer to a tr_socket that will be used for the transport connection. If NULL the tr_socket element of
 * the rtr_socket won't be changed.
 * @param[in] pfx_table pfx_table that stores the validation records obtained from the connected rtr server.
 * @param[in] polling_period Interval in seconds between serial queries that are sent to the server. Must be <= 3600
 * @param[in] cache_timeout Stored validation records will be deleted if cache was unable to refresh data for this period.\n
 * The default value is twice the polling_period.
 * @param[in] fp A callback function that is executed when the state of the socket changes.
 * @param[in] fp_data Parameter that is passed to the connection_state_fp callback.
 */
void rtr_init(struct rtr_socket *rtr_socket, struct tr_socket *tr_socket, struct pfx_table *pfx_table, const unsigned int polling_period, const unsigned int cache_timeout, rtr_connection_state_fp fp, void *fp_data);

/**
 * @brief Starts the RTR protocol state machine in a pthread. Connection to the rtr_server will be established and the
 * pfx_records will be synced.
 * @param[in] rtr_socket rtr_socket that will be used.
 * @return RTR_ERROR On error.
 * @return RTR_SUCCESS On success.
 */
int rtr_start(struct rtr_socket *rtr_socket);

/**
 * @brief Stops the RTR connection and terminate the transport connection.
 * @param[in] rtr_socket rtr_socket that will be used.
 */
void rtr_stop(struct rtr_socket *rtr_socket);

/**
 * @brief Converts a rtr_socket_state to a String.
 * @param[in] state state to convert to a string
 * @return NULL If state isn't a valid rtr_socket_state
 * @return !=NULL The rtr_socket_state as String.
 */
const char *rtr_state_to_str(rtr_socket_state state);

#endif
/* @} */
