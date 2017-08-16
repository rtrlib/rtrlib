/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
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

#define RTR_DBG(fmt, ...) lrtr_dbg("RTR Socket: " fmt, ## __VA_ARGS__)
#define RTR_DBG1(a) lrtr_dbg("RTR Socket: " a)

static const uint8_t RTR_PROTOCOL_VERSION_0 = 0;
static const uint8_t RTR_PROTOCOL_VERSION_1 = 1;

static const uint8_t RTR_PROTOCOL_MIN_SUPPORTED_VERSION = 0;
static const uint8_t RTR_PROTOCOL_MAX_SUPPORTED_VERSION = 1;

enum rtr_rtvals {
    RTR_SUCCESS = 0,
    RTR_ERROR = -1,
    RTR_INVALID_PARAM = -2
};

/**
 * @brief States of the RTR socket.
 */
enum rtr_socket_state {
    /** Socket is establishing the transport connection. */
    RTR_CONNECTING,

    /** Connection is established, socket is waiting for a Serial Notify or expiration of the refresh_interval timer */
    RTR_ESTABLISHED,

    /** Resetting RTR connection. */
    RTR_RESET,

    /** Receiving validation records from the RTR server.  */
    RTR_SYNC,

    /** Reconnect without any waiting period */
    RTR_FAST_RECONNECT,

    /** No validation records are available on the RTR server. */
    RTR_ERROR_NO_DATA_AVAIL,

    /** Server was unable to answer the last serial or reset query. */
    RTR_ERROR_NO_INCR_UPDATE_AVAIL,

    /** Fatal protocol error occurred. */
    RTR_ERROR_FATAL,

    /** Error on the transport socket occurred. */
    RTR_ERROR_TRANSPORT,

    /** RTR Socket was started, but now has shut down. */
    RTR_SHUTDOWN,

    /** RTR Socket has not been started yet. Initial state after rtr_init */
    RTR_CLOSED,
};

struct rtr_socket;

/**
 * @brief A function pointer that is called if the state of the rtr socket has changed.
 */
typedef void (*rtr_connection_state_fp)(const struct rtr_socket *rtr_socket, const enum rtr_socket_state state, void *connection_state_fp_param_config, void *connection_state_fp_param_group);

/**
 * @brief A RTR socket.
 * @param tr_socket Pointer to an initialized tr_socket that will be used to communicate with the RTR server.
 * @param refresh_interval Time period in seconds. Tells the router how long to wait before next attempting to poll the cache, using a Serial Query or
 * Reset Query PDU.
 * @param last_update Timestamp of the last validation record update. Is 0 if the pfx_table doesn't stores any
 * validation reords from this rtr_socket.
 * @param expire_interval Time period in seconds. Received records are deleted if the client was unable to refresh data for this time period.
 * If 0 is specified, the expire_interval is twice the refresh_interval.
 * @param retry_interval Time period in seconds between a faild quary and the next attempt.
 * @param state Current state of the socket.
 * @param session_id session_id of the RTR session.
 * @param request_session_id True, if the rtr_client have to request a new none from the server.
 * @param serial_number Last serial number of the obtained validation records.
 * @param pfx_table pfx_table that stores the validation records obtained from the connected rtr server.
 * @param thread_id Handle of the thread this socket is running in.
 * @param connection_state_fp A callback function that is executed when the state of the socket changes.
 * @param connection_state_fp_param_config Parameter that is passed to the connection_state_fp callback. Expects a pointer to a rtr_mgr_config struct.
 * @param connection_state_fp_param_group Parameter that is passed to the connection_state_fp callback. Expects a pointer to the rtr_mgr_group this socket belongs to.
 * @param version Protocol version used by this socket
 * @param has_received_pdus True, if this socket has already recieved PDUs
 * @param spki_table spki_table that stores the router keys obtaiend from the connected rtr server
 */
struct rtr_socket {
    struct tr_socket *tr_socket;
    unsigned int refresh_interval;
    time_t last_update;
    unsigned int expire_interval;
    unsigned int retry_interval;
    enum rtr_socket_state state;
    uint32_t session_id;
    bool request_session_id;
    uint32_t serial_number;
    struct pfx_table *pfx_table;
    pthread_t thread_id;
    rtr_connection_state_fp connection_state_fp;
    void *connection_state_fp_param_config;
    void *connection_state_fp_param_group;
    unsigned int version;
    bool has_received_pdus;
    struct spki_table *spki_table;
};

/**
 * @brief Initializes a rtr_socket.
 * @param[out] rtr_socket Pointer to the allocated rtr_socket that will be initialized.
 * @param[in] tr_socket Pointer to a tr_socket that will be used for the transport connection. If NULL the tr_socket element of
 * the rtr_socket won't be changed.
 * @param[in] pfx_table pfx_table that stores the validation records obtained from the connected rtr server.
 * @param[in] spki_table spki_table that stores the router keys obtained from the connected rtr server.
 * @param[in] refresh_interval Interval in seconds between serial queries that are sent to the server. Must be >= 1 and <= 86400 (one day),
 * recommended default is 3600s (one hour).
 * @param[in] expire_interval Stored validation records will be deleted if cache was unable to refresh data for this period.
 * The value should be twice the refresh_interval. The value must be >= 600 (ten minutes) and <= 172800 (two days).
 * The recommanded default is 7200s (two hours).
 * @param[in] retry_interval This parameter tells the router how long to wait (in seconds) before retrying
 * a failed Serial Query or Reset Query. The value must be >= 1s and <= 7200s (two hours).
 * The recommanded default is 600 seconds (ten minutes).
 * @param[in] fp A callback function that is executed when the state of the socket changes.
 * @param[in] fp_data_config Parameter that is passed to the connection_state_fp callback.
 * Expects rtr_mgr_config.
 * @param[in] fp_data_group Parameter that is passed to the connection_state_fp callback.
 * Expects rtr_mgr_group.
 * @return RTR_INVALID_PARAM If the refresh_interval or the expire_interval is not valid.
 * @return RTR_SUCCESS On success.
 */
int rtr_init(struct rtr_socket *rtr_socket, struct tr_socket *tr_socket, struct pfx_table *pfx_table,
             struct spki_table *spki_table, const unsigned int refresh_interval,
             const unsigned int expire_interval, const unsigned int retry_interval,
             rtr_connection_state_fp fp, void *fp_data_config, void *my_group);

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
const char *rtr_state_to_str(enum rtr_socket_state state);

#endif
/* @} */
