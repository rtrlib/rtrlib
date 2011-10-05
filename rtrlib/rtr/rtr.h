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
#include "rtrlib/pfx/pfx.h"
#include "rtrlib/transport/transport.h"


/**
 * @brief States of the RTR socket.
 */
typedef enum rtr_socket_state{
/** Socket is closed. */
    RTR_CLOSED,
/** Socket connection is established. */
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
/** Socket is shutting down */
    RTR_SHUTDOWN 
} rtr_socket_state;

struct rtr_socket;
struct rtr_mgr_config;

/**
 * @brief A function pointer that is called if the state of the rtr socket has changed.
 */
typedef void (*rtr_connection_state_fp)(const struct rtr_socket* rtr_socket, const rtr_socket_state state, struct rtr_mgr_config* mgr_config, unsigned int mgr_config_len);

/**
 * @brief A RTR socket.
 * @param tr_socket Pointer to an initialized tr_socket that will be used to communicate with the RTR server.
 * @param polling_period Interval in seconds between update queries to the server. Must be <= 3600
 * @param last_update Timestamp of the last validation record update
 * @param cache_timout Stored validation records will be deleted if cache was unable to refresh data for this period.\n
 * The default value is twice the polling_period.
 * @param state Current state of the socket.
 * @param nonce Nonce of the RTR session.
 * @param serial_number Last serial number of the obtained validation records.
 * @param pfx_table pfx_table that stores the validation records obtained from the connected rtr server.
 */
typedef struct rtr_socket{
    tr_socket* tr_socket;
    unsigned int polling_period;
    time_t last_update;
    unsigned int cache_timeout;
    rtr_socket_state state;
    uint32_t nonce;
    uint32_t serial_number;
    struct pfx_table* pfx_table;
    pthread_t thread_id;
    rtr_connection_state_fp connection_state_fp;
    struct rtr_mgr_config* mgr_config;
    unsigned int mgr_config_len;
} rtr_socket;


/**
 * @brief Initialize a rtr_socket
 * @param[out] rtr_socket Pointer to the allocated rtr_socket that will be initialized.
 * @param[in] tr Pointer to a tr_socket that will be used for the transport connection.
 * @param[in] polling_period
 * @param[in] cache_timeout
 * @param[in] pfx_table
 * @param[in] connection_state_fp
 * @param[in] connection_state_fp_len
 */
void rtr_init(rtr_socket* rtr_socket, tr_socket* tr, struct pfx_table* pfx_table, const unsigned int polling_period, const unsigned int cache_timeout);

/**
 * @brief Starts the RTR protocol state machine in a pthread. Connection to the rtr_server will be established and the
 * pfx_records will be synced.
 * @param[in] rtr_socket rtr_socket that will be used.
 * @return RTR_ERROR
 */
int rtr_start(rtr_socket* rtr_socket);

/**
 * @brief Stop the RTR connection and terminate the transport connection
 * @param[in] rtr_socket rtr_socket that will be used.
 */
void rtr_stop(rtr_socket* rtr_socket);

/**
 * @brief Free all memory that was allocated by the rtr_socket.
 * @param[in] rtr_socket rtr_socket that will be used.
 */
void rtr_free(rtr_socket* rtr_socket);

#endif
/* @} */
