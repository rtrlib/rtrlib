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
 * Webpage: http://rpki.realmv6.org/
 */

/**
 * @defgroup mod_rtr_mgr_h RTR connection manager
 * @brief The primary interface for library users.\n
 * Manages a set of rtr_sockets.
 * The RTR connection manager is initialized with a set of rtr_sockets with preference values.
 * It connects to the sockets with the lowest preference values. On errors a connection to the set of server with next
 * lowest priority will be established (see the IETF draft for details about error handling).
 *
 * @{
 *
 * @example rtr_mgr.c
 * Usage example of the RTR connection manager.
 */
#ifndef RTR_MGR
#define RTR_MGR
#include "rtrlib/pfx/pfx.h"
#include "rtrlib/rtr/rtr.h"



/**
 * @brief A linked list that holds a set of rtr_sockets and their preference values.
 * @param pref A rtr_socket with a lower preference value has a higher priority.
 * @param next Pointer to another rtr_server_pool struct or NULL.
 */
typedef struct rtr_server_pool{
    u_int pref;
    rtr_socket* rtr_socket;
    struct rtr_server_pool* next;
} rtr_server_pool;


/**
 * @brief A rtr_mgr_socket, holds data associated with this pool.
 * @param pfx_table Pointer to a pfx_table that holds the validation records for all rtr_socket in this pool.
 * @param config Configuration of this manager socket.
 */
typedef struct rtr_mgr_socket{
    struct pfx_table* pfx_table;
    rtr_server_pool* config;
} rtr_mgr_socket;



/**
 * @brief Initialize all rtr_sockets in config with the supplied values.
 * @param[in] config Includes all  rtr_sockets that will be initialized.
 * @param[in] polling_period If not -1 the polling_period member of the rtr_sockets will be set to this value. See @ref rtr_socket
 * @param[in] cache_timeout If not -1 the polling_period member of the rtr_sockets will be set to this value. See @ref rtr_socket
 * @param[in] update_fp If not NULL the update_fp member of the rtr_sockets will be set to this value. See @ref rtr_socket
 * @param[in] update_fp_len If update_fp is not NULL the update_fp_len member of the rtr_sockets will be set to this value. See @ref rtr_socket
 * @param[in] connection_state_fp If not NULL the connection_state_fp member of the rtr_sockets will be set to this value. See @ref rtr_socket
 * @param[in] connection_state_fp_len If connection_state_fp is not NULL the update_fp_len member of the rtr_sockets will be set to this value. See @ref rtr_socket
 */
void rtr_mgr_init(rtr_server_pool* config, const 
        int polling_period, const int cache_timeout, rtr_update_fp* update_fp, unsigned int
        update_fp_len, rtr_connection_state_fp* connection_state_fp, unsigned int connection_state_fp_len);

/**
 * @brief Initiates the rtr_connection_pool socket, establishes the connection with the set of rtr_sockets with the lowest preference value and treats error like
 * described in the rpki-rtr draft.
 * @param[out] socket Pointer to an allocated rtr_mgr_socket that will be initialized.
 * @param[in] config Configuration of the connection pool.
 * @return 0  On success 
 * @return -1 On error 
 */
int rtr_mgr_start(rtr_mgr_socket* socket, rtr_server_pool* config);

/**
 * @brief Terminates all rtr_socket connections in the pool and removes all entries from the pfx_tables.
 * @param[in] socket The rtr_mgr_socket.
 * @return 0  On success 
 * @return -1 On error 
 */
void rtr_mgr_stop(rtr_mgr_socket* socket);

/**
 * @brief Free all ressources that are associated with the rtr_mgr_socket.
 * @param[in] socket rtr_mgr_socket.
 */
void rtr_mgr_free(rtr_mgr_socket* socket);


/**
 * @brief Validate the origin of a BGP route.
 * @param[in] socket rtr_mgr_socket
 * @param[in] asn Autonomous system number.
 * @param[in] prefix Pointer to an IP-Prefix.
 * @param[in] mask_len IP-Prefix length.
 * @param[out] result Result of the validation.
 * @return 0 On success.
 * @return -1 On error.
 */
int rtr_mgr_validate(const rtr_mgr_socket* socket, const u_int32_t asn, const ip_addr* prefix, const u_int8_t mask_len, pfxv_state* result);

#endif
/* @} */
