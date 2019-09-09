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
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

enum rtr_rtvals { RTR_SUCCESS = 0, RTR_ERROR = -1, RTR_INVALID_PARAM = -2 };

/**
 * @brief These modes let the user configure how received intervals should be handled.
 */
enum rtr_interval_mode {
	/** Ignore appliance of interval values at all. */
	RTR_INTERVAL_MODE_IGNORE_ANY,

	/** Accept any interval values, even if outside of range. */
	RTR_INTERVAL_MODE_ACCEPT_ANY,

	/** If interval value is outside of range, apply min (if below range) or max (if above range). */
	RTR_INTERVAL_MODE_DEFAULT_MIN_MAX,

	/** Ignore any interval values that are outside of range. */
	RTR_INTERVAL_MODE_IGNORE_ON_FAILURE
};

/**
 * @brief States of the RTR socket.
 */
enum rtr_socket_state {
	/** Socket is establishing the transport connection. */
	RTR_CONNECTING,

	/** Connection is established,
	 * socket is waiting for a Serial Notify or expiration of the refresh_interval timer
	 */
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
typedef void (*rtr_connection_state_fp)(const struct rtr_socket *rtr_socket, const enum rtr_socket_state state,
					void *connection_state_fp_param_config, void *connection_state_fp_param_group);

/**
 * @brief A RTR socket.
 * @param tr_socket Pointer to an initialized tr_socket that will be used to communicate with the RTR server.
 * @param refresh_interval Time period in seconds. Tells the router how long to wait before next attempting
 * to poll the cache, using a Serial Query or Reset Query PDU.
 * @param last_update Timestamp of the last validation record update. Is 0 if the pfx_table doesn't stores any
 * validation reords from this rtr_socket.
 * @param expire_interval Time period in seconds. Received records are deleted if the client was unable to refresh data
 * for this time period.  If 0 is specified, the expire_interval is twice the refresh_interval.
 * @param retry_interval Time period in seconds between a failed query and the next attempt.
 * @param iv_mode Defines handling of incoming intervals.
 * @param state Current state of the socket.
 * @param session_id session_id of the RTR session.
 * @param request_session_id True, if the rtr_client have to request a new none from the server.
 * @param serial_number Last serial number of the obtained validation records.
 * @param pfx_table pfx_table that stores the validation records obtained from the connected rtr server.
 * @param thread_id Handle of the thread this socket is running in.
 * @param connection_state_fp A callback function that is executed when the state of the socket changes.
 * @param connection_state_fp_param_config Parameter that is passed to the connection_state_fp callback.
 * Expects a pointer to a rtr_mgr_config struct.
 * @param connection_state_fp_param_group Parameter that is passed to the connection_state_fp callback.
 * Expects a pointer to the rtr_mgr_group this socket belongs to.
 * @param version Protocol version used by this socket
 * @param has_received_pdus True, if this socket has already received PDUs
 * @param spki_table spki_table that stores the router keys obtained from the connected rtr server
 */
struct rtr_socket {
	struct tr_socket *tr_socket;
	unsigned int refresh_interval;
	time_t last_update;
	unsigned int expire_interval;
	unsigned int retry_interval;
	enum rtr_interval_mode iv_mode;
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
	bool is_resetting;
};

/**
 * @brief Converts a rtr_socket_state to a String.
 * @param[in] state state to convert to a string
 * @return NULL If state isn't a valid rtr_socket_state
 * @return !=NULL The rtr_socket_state as String.
 */
const char *rtr_state_to_str(enum rtr_socket_state state);

/**
 * @brief Set the interval option to the desired one. It's either RTR_INTERVAL_MODE_IGNORE_ANY,
 * RTR_INTERVAL_MODE_APPLY_ANY, RTR_INTERVAL_MODE_DEFAULT_MIN_MAX or RTR_INTERVAL_MODE_IGNORE_ON_FAILURE.
 * @param[in] rtr_socket The target socket.
 * @param[in] option The new interval option that should be applied.
 */
void rtr_set_interval_mode(struct rtr_socket *rtr_socket, enum rtr_interval_mode option);

/**
 * @brief Get the current interval mode.
 * @param[in] rtr_socket The target socket.
 * @return The value of the interval_option variable.
 */
enum rtr_interval_mode rtr_get_interval_mode(struct rtr_socket *rtr_socket);
#endif
/** @} */
