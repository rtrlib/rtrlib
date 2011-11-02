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
 * @defgroup mod_rtr_mgr_h RTR connection manager
 * @brief The RTR connection manager maintains a set of @ref rtr_socket "RTR sockets".
 * @warning This function is not yet implemented.
 * @details The RTR connection manager is initialized with a set of
 * rtr_sockets. Each of them is configured with a preference value.
 * It connects to the socket with the lowest preference value.\n
 * In case of failures, the connection manager establishes
 * connections to RTR servers with the next lowest preference value (see
 * the <a href="http://tools.ietf.org/html/draft-ietf-sidr-rpki-rtr">IETF
 * draft-ietf-sidr-rpki-rtr</a> for details about error handling).
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

typedef enum {
    RTR_MGR_CLOSED,
    RTR_MGR_CONNECTING,
    RTR_MGR_ESTABLISHED,
    RTR_MGR_ERROR,
} rtr_mgr_status;

typedef struct {
    rtr_socket** sockets;
    unsigned int sockets_len;
    uint8_t preference;
    rtr_mgr_status status;
} rtr_mgr_group;

typedef struct rtr_mgr_config {
    unsigned int len;
    rtr_mgr_group* groups;
    pthread_mutex_t mutex;
} rtr_mgr_config;

/**
 * @brief Initialize all rtr_sockets in config with the supplied values.
 * @param[in] config a rtr_mgr_config struct with initialized len and groups members.
 The preference value must be unique in the rtr_mgr_config array, a reference to the same rtr_socket
 * in two different rtr_socket** arrays will produce unpredictable behaviour.
 * @return RTR_SUCCESS On success 
 * @return RTR_ERROR On error 
 *
 */
int rtr_mgr_init(rtr_mgr_config* config, const unsigned int polling_period, const unsigned int cache_timeout, pfx_update_fp update_fp);

/**
 * @brief Free all ressources that are associated with the rtr_mgr_socket.
 * @param[in] socket rtr_mgr_socket.
 */
void rtr_mgr_free(rtr_mgr_config* config);

/**
 * @brief Initiates the rtr_connection_pool socket, establishes the connection with the set of rtr_sockets with the lowest preference value and treats error like
 * described in the rpki-rtr draft.
 * @param[in] socket Pointer to an allocated rtr_mgr_socket that will be initialized.
 * @return RTR_SUCCESS On success 
 * @return RTR_ERROR On error 
 */
int rtr_mgr_start(rtr_mgr_config* config);

/**
 * @brief Terminates all rtr_socket connections in the pool and removes all entries from the pfx_tables.
 * @param[in] socket The rtr_mgr_socket.
 * @return RTR_SUCCESS  On success 
 * @return RTR_ERROR On error 
 */
void rtr_mgr_stop(rtr_mgr_config* config);

/**
 * @brief Terminates all rtr_socket connections in the pool and removes all entries from the pfx_tables.
 * @param[in] socket The rtr_mgr_socket.
 * @return true If the used pfx_table holds non-outdated pfx_records from at least one socket group.
 * @return false If the pfx_table doesn't hold non-outdated pfx_records from at least one socket group.
 */
bool rtr_mgr_group_in_sync(rtr_mgr_config* config);

int rtr_mgr_validate(rtr_mgr_config* config, const uint32_t asn, const ip_addr* prefix, const uint8_t mask_len, pfxv_state* result);

#endif
/* @} */
