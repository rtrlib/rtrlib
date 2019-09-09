/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef RTR_PRIVATE_H
#define RTR_PRIVATE_H
#include "rtrlib/rtr/rtr.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

#define RTR_DBG(fmt, ...) lrtr_dbg("RTR Socket: " fmt, ##__VA_ARGS__)
#define RTR_DBG1(a) lrtr_dbg("RTR Socket: " a)

static const uint32_t RTR_EXPIRATION_MIN = 600; // ten minutes
static const uint32_t RTR_EXPIRATION_MAX = 172800; // two days
static const uint32_t RTR_EXPIRATION_DEFAULT = 7200; // two hours

static const uint32_t RTR_REFRESH_MIN = 1; // one second
static const uint32_t RTR_REFRESH_MAX = 86400; // one day
static const uint32_t RTR_REFRESH_DEFAULT = 3600; // one hour

static const uint32_t RTR_RETRY_MIN = 1; // one second
static const uint32_t RTR_RETRY_MAX = 7200; // two hours
static const uint32_t RTR_RETRY_DEFAULT = 600; // ten minutes

static const uint8_t RTR_PROTOCOL_VERSION_0; // = 0
static const uint8_t RTR_PROTOCOL_VERSION_1 = 1;

static const uint8_t RTR_PROTOCOL_MIN_SUPPORTED_VERSION; // = 0
static const uint8_t RTR_PROTOCOL_MAX_SUPPORTED_VERSION = 1;

enum rtr_interval_range { RTR_BELOW_INTERVAL_RANGE = -1, RTR_INSIDE_INTERVAL_RANGE = 0, RTR_ABOVE_INTERVAL_RANGE = 1 };

enum rtr_interval_type { RTR_INTERVAL_TYPE_EXPIRATION, RTR_INTERVAL_TYPE_REFRESH, RTR_INTERVAL_TYPE_RETRY };

/**
 * @brief Initializes a rtr_socket.
 * @param[out] rtr_socket Pointer to the allocated rtr_socket that will be initialized.
 * @param[in] tr_socket Pointer to a tr_socket that will be used for the transport connection.
 * If NULL the tr_socket element of the rtr_socket won't be changed.
 * @param[in] pfx_table pfx_table that stores the validation records obtained from the connected rtr server.
 * @param[in] spki_table spki_table that stores the router keys obtained from the connected rtr server.
 * @param[in] refresh_interval Interval in seconds between serial queries that are sent to the server.
 * Must be >= 1 and <= 86400 (one day), recommended default is 3600s (one hour).
 * @param[in] expire_interval Stored validation records will be deleted
 * if cache was unable to refresh data for this period.
 * The value should be twice the refresh_interval. The value must be >= 600 (ten minutes) and <= 172800 (two days).
 * The recommended default is 7200s (two hours).
 * @param[in] retry_interval This parameter tells the router how long to wait (in seconds) before retrying
 * a failed Serial Query or Reset Query. The value must be >= 1s and <= 7200s (two hours).
 * The recommended default is 600 seconds (ten minutes).
 * @param[in] iv_mode The interval mode that controls how new interval values are applied.
 * @param[in] fp A callback function that is executed when the state of the socket changes.
 * @param[in] fp_data_config Parameter that is passed to the connection_state_fp callback.
 * Expects rtr_mgr_config.
 * @param[in] fp_data_group Parameter that is passed to the connection_state_fp callback.
 * Expects rtr_mgr_group.
 * @return RTR_INVALID_PARAM If the refresh_interval or the expire_interval is not valid.
 * @return RTR_SUCCESS On success.
 */
int rtr_init(struct rtr_socket *rtr_socket, struct tr_socket *tr_socket, struct pfx_table *pfx_table,
	     struct spki_table *spki_table, const unsigned int refresh_interval, const unsigned int expire_interval,
	     const unsigned int retry_interval, enum rtr_interval_mode iv_mode, rtr_connection_state_fp fp,
	     void *fp_data_config, void *fp_data_group);

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

#endif
