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
 * @brief The RTR connection manager maintains multiple groups of @ref
 * rtr_socket "RTR sockets".
 * @details The RTR connection manager is initialized with one or multiple
 * groups of rtr_sockets. Each group is configured with a preference
 * value and contains a set of rtr_socket "RTR sockets. It connects to all
 * sockets of the group with the lowest preference value.\n
 * In case of failures, the connection manager establishes connections to
 * RTR servers of another group with the next lowest preference value (see
 * the <a href="http://tools.ietf.org/html/draft-ietf-sidr-rpki-rtr">IETF
 * draft-ietf-sidr-rpki-rtr</a> for details about error handling).
 *
 * @{
 * @example rtr_mgr.c
 * Usage example of the RTR connection manager.
 */
#ifndef RTR_MGR
#define RTR_MGR
#include "rtrlib/pfx/pfx.h"
#include "rtrlib/rtr/rtr.h"

/**
 * @brief Status of a rtr_mgr_group.
 */
typedef enum {
    /** RTR sockets are disconnected */
    RTR_MGR_CLOSED,

    /** RTR sockets trying to establish a connection. */
    RTR_MGR_CONNECTING,

    /** All RTR sockets of the group are synchronized with the rtr servers. */
    RTR_MGR_ESTABLISHED,

    /** Error occured on at least one RTR socket. */
    RTR_MGR_ERROR,
} rtr_mgr_status;

/**
 * @brief A set of RTR sockets.
 * @param sockets Array of rtr_socket pointer. The tr_socket element of the rtr_socket must be associated with an initialized transport socket.
 * @param sockets_len Number of elements in the sockets array.
 * @param preference The preference value of this group. Groups with lower preference values are preferred.
 * @param status Status of the group.
 */
typedef struct {
    rtr_socket **sockets;
    unsigned int sockets_len;
    uint8_t preference;
    rtr_mgr_status status;
} rtr_mgr_group;

typedef struct rtr_mgr_config rtr_mgr_config;

typedef void (*rtr_mgr_status_fp)(const rtr_mgr_group *, rtr_mgr_status, const rtr_socket *,
				  void *);

/**
 * @brief Initializes a rtr_mgr_config.
 * @param[in] groups Array of rtr_mgr_group. Every RTR socket in an
 *		     rtr_mgr_group must be assoziated with an initialized
 *		     transport socket. A Transport socket is only allowed to be
 *		     associated with one rtr socket. The preference values must
 *		     be unique in the group array. More than one rtr_mgr_group
 *		     with the same preference value isn't allowed.
 * @param[in] groups_len Number of elements in the groups array.
 * @param[in] polling_period Interval in seconds between serial queries that are sent to the server. Must be <= 3600.
 * If 0 is specified the polling_period is set to 300 seconds.
 * @param[in] cache_timeout Time period in seconds. Received pfx_records are deleted if the client was unable to refresh data for this time period.
 * If 0 is specified, the cache_timeout will be half the polling_period.
 * The default value is twice the polling_period.
 * @param[in] update_fp A Pointer to a pfx_update_fp callback, that is executed for every added and removed pfx_record.
 * @param[in] status_fp Pointer to a function that is called if the connection
 *	                status from one of the socket groups is changed.
 * @param[in] status_fp_data Pointer to a memory area that is passed to the
 *			     status_fp function. Memory area can be freely used
 *			     to pass user-defined data to the status_fp
 *			     callback.
 * @return !NULL On success
 * @return NULL On error
 */
rtr_mgr_config *rtr_mgr_init(rtr_mgr_group groups[], const unsigned int groups_len,
		 const unsigned int polling_period, const unsigned int cache_timeout, 
		 const pfx_update_fp update_fp,
		 const rtr_mgr_status_fp status_fp,
		 void *status_fp_data);

/**
 * @brief Frees all resources that were allocated from the rtr_mgr.
 * rtr_mgr_stop(..) must be called before, to shutdown all RTR socket connections.
 * @param[in] config rtr_mgr_config.
 */
void rtr_mgr_free(rtr_mgr_config *config);

/**
 * @brief Establishes the connection with the rtr_sockets of the group with the lowest preference value and handles errors as defined in the RPKI-RTR protocol.
 * @param[in] config Pointer to an initialized rtr_mgr_config.
 * @return RTR_SUCCESS On success
 * @return RTR_ERROR On error
 */
int rtr_mgr_start(rtr_mgr_config *config);

/**
 * @brief Terminates all rtr_socket connections that are defined in the config. All pfx_records received from these sockets will be purged.
 * @param[in] config The rtr_mgr_config struct
 */
void rtr_mgr_stop(rtr_mgr_config *config);

/**
 * @brief Detects if the rtr_mgr_group is fully synchronized with at least one group.
 * @param[in] config The rtr_mgr_config.
 * @return true If the pfx_table stores non-outdated pfx_records from at least one socket group.
 * @return false If the pfx_table isn't fully synchronized with at least one group.
 */
bool rtr_mgr_conf_in_sync(rtr_mgr_config *config);

/**
 * @brief Validates the origin of a BGP-Route.
 * @param[in] config
 * @param[in] asn Autonomous system number of the Origin-AS of the prefix.
 * @param[in] prefix Announced network prefix
 * @param[in] mask_len Length of the network mask of the announced prefix
 * @param[out] result Outcome of the validation.
 * @return PFX_SUCCESS On success.
 * @return PFX_ERROR If an error occurred.
 */
int rtr_mgr_validate(rtr_mgr_config *config, const uint32_t asn, const ip_addr *prefix, const uint8_t mask_len, pfxv_state *result);

#endif
/* @} */
