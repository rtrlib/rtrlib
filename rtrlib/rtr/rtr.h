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
 * Webpage: https://rpki.realmv6.org/
 */


/**
 * @defgroup mod_rtr_h RTR socket
 * @brief An Implementation of the RPKI-RTR protocol\n
 * One rtr_socket communicates with a single RPKI-RTR server.
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
 * @brief The states of RTR socket.
 */
typedef enum rtr_socket_state{
/** Socket is closed. */
    RTR_CLOSED,
/** Socket connection is established. */
    RTR_ESTABLISHED,
/** Reseting RTR connection. */
    RTR_RESET,
/** Receiving validation records from the RTR-Server.  */
    RTR_SYNC,
/** No validation records available on RTR-Server. */
    RTR_ERROR_NO_DATA_AVAIL,
/** Server was unable to answer the last serial or reset query. */
    RTR_ERROR_NO_INCR_UPDATE_AVAIL,
/** Fatal protocol error occured. */
    RTR_ERROR_FATAL,
/** Error on the transport socket occured. */
    RTR_ERROR_TRANSPORT,
/** rtr_socket is shutting down */
    RTR_SHUTDOWN 
} rtr_socket_state;

struct rtr_socket;



/**
 * @brief A function pointer that is called if the state of the rtr socket has changed.
 */
typedef void (*rtr_connection_state_fp)(const struct rtr_socket* rtr_socket, const rtr_socket_state state);

/**
 * @brief A rtr socket.
 * @param tr_socket Pointer to an initialized tr_socket that will be used to communicate with the RTR server.
 * @param polling_period Interval in seconds between update queries to the server. Must be <= 3600
 * @param last_update Timestamp of the last validation record update
 * @param cache_timout Stored validation records will be deleted if cache was unable to refresh data for this period.\n
 * The default value is twice the polling_period.
 * @param state Current state of the socket.
 * @param nonce Nonce of the RTR session.
 * @param serial_number Last serial number of the obtained validation records.
 * @param pfx_table pfx_table that stores the validation records obtained from the connected rtr server.
 * @param connection_state_fp Array of function pointers that will be called if the state of the RTR socket changed.
 * @param connection_state_fp_len Number of elements in the connection_state_fp array.
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
    rtr_connection_state_fp* connection_state_fp;
    unsigned int connection_state_fp_len;
} rtr_socket;


/*
  Function: rtr_init

  initializes a rtr_socket

  Parameters:

    transport - transport for the rtr_socket
    polling_period - period between serial querys
    rtr_socket - socket that will be initialized
    pfx_table - pfx_table that will be used
    update_cb - function pointer, will be called at updates of validation record
    connection_state_cb - function pointer, called if the state of the rtr connection changes
    rtr_connection_state_fp_len - elements of array
 */

/**
 * @brief Initialize a rtr_socket
 * @param[out] rtr_socket Pointer to allocated rtr_socket that will be initialized.
 * @param[in] tr Pointer to a tr_socket that will be used for the transport connection.
 * @param[in] polling_period
 * @param[in] cache_timeout
 * @param[in] pfx_table
 * @param[in] connection_state_fp
 * @param[in] connection_state_fp_len
 */
void rtr_init(rtr_socket* rtr_socket, tr_socket* tr, struct pfx_table* pfx_table, unsigned int polling_period, unsigned int cache_timeout, rtr_connection_state_fp connection_state_fp[], unsigned int connection_state_fp_len);


/**
 * @brief Start the RTR connection. Transport connection will be established and a RTR session started.
 * @param[in] rtr_socket rtr_socket that will be used.
 * @return RTR_ERROR If transport connection could'nt be established
 */
void rtr_start(rtr_socket* rtr_socket);


/**
 * @brief Stop the RTR connection and terminate the transport connection
 * @param[in] rtr_socket rtr_socket that will be used.
 * @param[in] thread_id ID of the thread in which rtr_start was executed.
 */
void rtr_stop(rtr_socket* rtr_socket, const pthread_t thread_id);


/**
 * @brief Free all memory that was allocated by the rtr_socket.
 * @param[in] rtr_socket rtr_socket that will be used.
 */
void rtr_free(rtr_socket* rtr_socket);

#endif
/* @} */
