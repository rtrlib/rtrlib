/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

/**
 * @defgroup mod_rtr_mgr_h RTR connection manager
 * @brief The RTR connection manager maintains multiple groups of @ref
 * rtr_socket "RTR sockets".
 * @details The RTR connection manager is initialized with one or multiple
 * groups of rtr_sockets. Each group is configured with a preference
 * value and contains a set of rtr_socket RTR sockets. It connects to all
 * sockets of the group with the lowest preference value.\n
 * In case of failures, the connection manager establishes connections to
 * RTR servers of another group with the next lowest preference value (see
 * <a href="https://tools.ietf.org/html/rfc6810">IETF
 * RFC 6810</a> for details about error handling).\n
 * RTRlib also supports a Retry Interval (see
 * <a href="https://tools.ietf.org/html/draft-ietf-sidr-rpki-rtr-rfc6810-bis">
 * draft-ietf-sidr-rpki-rtr-rfc6810-bis</a>).
 * If a more preferred group is online again, the RTR connection manager
 * will switch back and close connections to the caches of the less
 * preferred group.
 *
 * @{
 * @example rtr_mgr.c
 * Usage example of the RTR connection manager.
 */
#ifndef RTR_MGR
#define RTR_MGR

#include "rtrlib/pfx/pfx.h"
#include "rtrlib/spki/spkitable.h"

#include <pthread.h>
#include <stdint.h>

/**
 * @brief Status of a rtr_mgr_group.
 */
enum rtr_mgr_status {
	/** RTR sockets are disconnected */
	RTR_MGR_CLOSED,
	/** RTR sockets trying to establish a connection. */
	RTR_MGR_CONNECTING,
	/** All RTR sockets of the group are synchronized with rtr servers. */
	RTR_MGR_ESTABLISHED,
	/** Error occurred on at least one RTR socket. */
	RTR_MGR_ERROR,
};

/**
 * @brief A set of RTR sockets.
 * @param sockets Array of rtr_socket pointer. The tr_socket element of
 *		the rtr_socket must be associated with an initialized #
 *		transport socket.
 * @param sockets_len Number of elements in the sockets array.
 * @param preference The preference value of this group.
 *		   Groups with lower preference values are preferred.
 * @param status Status of the group.
 */
struct rtr_mgr_group {
	struct rtr_socket **sockets;
	unsigned int sockets_len;
	uint8_t preference;
	enum rtr_mgr_status status;
};

typedef void (*rtr_mgr_status_fp)(const struct rtr_mgr_group *, enum rtr_mgr_status, const struct rtr_socket *, void *);

struct tommy_list_wrapper;

// TODO Add refresh, expire, and retry intervals to config for easier access.
struct rtr_mgr_config {
	struct tommy_list_wrapper *groups;
	unsigned int len;
	pthread_mutex_t mutex;
	rtr_mgr_status_fp status_fp;
	void *status_fp_data;
	struct pfx_table *pfx_table;
	struct spki_table *spki_table;
};

/**
 * @brief Initializes a rtr_mgr_config.
 * @param[out] config_out The rtr_mgr_config that will be initialized by this
 *			function. On error, *config_out will be NULL!
 * @param[in] groups Linked list of rtr_mgr_group. Every RTR socket in an
 *		     rtr_mgr_group must be assoziated with an initialized
 *		     transport socket. A Transport socket is only allowed to be
 *		     associated with one rtr socket. The preference values must
 *		     be unique in the linked list. More than one rtr_mgr_group
 *		     with the same preference value isn't allowed.
 * @param[in] groups_len Number of elements in the groups array. Must be >= 1.
 * @param[in] refresh_interval Interval in seconds between serial queries that
 *			     are sent to the server. Must be >= 1 and <=
 *			     86400s (1d), recommended default is 3600s (1h).
 * @param[in] expire_interval Stored validation records will be deleted if
 *			    cache was unable to refresh data for this period.
 *			    The value should be twice the refresh_interval
 *			    and must be >= 600s (10min) and <= 172800s (2d).
 *			    The recommended default is 7200s (2h).
 * @param[in] retry_interval This parameter tells the router how long to wait
 *			   (in seconds) before retrying a failed Serial Query
 *			   or Reset Query.
 *			   The value must be >= 1s and <= 7200s (2h).
 *			   The recommended default is 600s (10min).
 * @param[in] update_fp Pointer to pfx_update_fp callback, that is executed for
			every added and removed pfx_record.
 * @param[in] spki_update_fp Pointer to spki_update_fp callback, that is
			     executed for every added and removed spki_record.
 * @param[in] status_fp Pointer to a function that is called if the connection
 *			status from one of the socket groups is changed.
 * @param[in] status_fp_data Pointer to a memory area that is passed to the
 *			     status_fp function. Memory area can be freely used
 *			     to pass user-defined data to the status_fp
 *			     callback.
 * @return RTR_ERROR If an error occurred
 * @return RTR_INVALID_PARAM If refresh_interval or expire_interval is invalid.
 * @return RTR_SUCCESS On success.
 */
int rtr_mgr_init(struct rtr_mgr_config **config_out, struct rtr_mgr_group groups[], const unsigned int groups_len,
		 const unsigned int refresh_interval, const unsigned int expire_interval,
		 const unsigned int retry_interval, const pfx_update_fp update_fp, const spki_update_fp spki_update_fp,
		 const rtr_mgr_status_fp status_fp, void *status_fp_data);

/**
 * @brief Adds a new rtr_mgr_group to the linked list of a initialized config.
 * @details A new group must have at least one rtr_socket associated
 *          with it. This socket must have at least one initialized
 *          transport socket associated with it. The new group must
 *          have a preference value that is none of the already present
 *          groups have. More than one rtr_mgr_group with the same
 *          preference is not allowed.
 * @param config A rtr_mgr_config struct that has been initialized
 *           previously with rtr_mgr_init
 * @param group A rtr_mgr_group with at least one rtr_socket and a
 *           preference value that no existing group has.
 * @return RTR_INVALID_PARAM If a group with the same preference value already
 *           exists.
 * @return RTR_ERROR If an error occurred while adding the group.
 * @return RTR_SUCCESS If the group was successfully added.
 *
 */
int rtr_mgr_add_group(struct rtr_mgr_config *config, const struct rtr_mgr_group *group);
/**
 * @brief Removes an existing rtr_mgr_group from the linked list of config.
 * @details The group to be removed is identified by its preference value.
 *          Should the group to be removed be currently active, it will be
 *          shut down and the next best group will be spun up.
 * @param config A rtr_mgr_config struct that has been initialized previously
 *          with rtr_mgr_init
 * @param preference The preference value of the group to be removed.
 * @return RTR_ERROR If no group with this preference value exists.
 * @return RTR_SUCCESS If group was successfully removed.
 *
 */
int rtr_mgr_remove_group(struct rtr_mgr_config *config, unsigned int preference);
/**
 * @brief Frees all resources that were allocated from the rtr_mgr.
 * @details rtr_mgr_stop must be called before, to shutdown all rtr_sockets.
 * @param[in] config rtr_mgr_config.
 */
void rtr_mgr_free(struct rtr_mgr_config *config);

/**
 * @brief Establishes rtr_socket connections
 * @details Establishes the connection with the rtr_sockets of the group
 * with the lowest preference value and handles errors as defined in the
 * RPKI-RTR protocol.
 * @param[in] config Pointer to an initialized rtr_mgr_config.
 * @return RTR_SUCCESS On success
 * @return RTR_ERROR On error
 */
int rtr_mgr_start(struct rtr_mgr_config *config);

/**
 * @brief Terminates rtr_socket connections
 * @details Terminates all rtr_socket connections defined in the config.
 * All pfx_records received from these sockets will be purged.
 * @param[in] config The rtr_mgr_config struct
 */
void rtr_mgr_stop(struct rtr_mgr_config *config);

/**
 * @brief Check if rtr_mgr_group is fully synchronized with at least one group.
 * @param[in] config The rtr_mgr_config.
 * @return true If pfx_table stores non-outdated pfx_records
 * @return false If pfx_table isn't fully synchronized with at least one group.
 */
bool rtr_mgr_conf_in_sync(struct rtr_mgr_config *config);

/**
 * @brief Validates the origin of a BGP-Route.
 * @param[in] config The rtr_mgr_config
 * @param[in] asn Autonomous system number of the Origin-AS of the prefix
 * @param[in] prefix Announced network prefix
 * @param[in] mask_len Length of the network mask of the announced prefix
 * @param[out] result Outcome of the validation
 * @return PFX_SUCCESS On success.
 * @return PFX_ERROR If an error occurred.
 */
int rtr_mgr_validate(struct rtr_mgr_config *config, const uint32_t asn, const struct lrtr_ip_addr *prefix,
		     const uint8_t mask_len, enum pfxv_state *result);

/**
 * @brief Returns all SPKI records which match the given ASN and SKI.
 * @param[in] config
 * @param[in] asn Autonomous system number of the Origin-AS
 * @param[in] ski the SKI to search for
 * @param[out] result a array of all matching spki_records
 * @param[out] result_count number of returned spki_records
 * @return SPKI_SUCCESS On success
 * @return SPKI_ERROR If an error occurred
 */
int rtr_mgr_get_spki(struct rtr_mgr_config *config, const uint32_t asn, uint8_t *ski, struct spki_record **result,
		     unsigned int *result_count);

/**
 * @brief Converts a rtr_mgr_status to a String.
 * @param[in] status state to convert to a string.
 * @return NULL If status isn't a valid rtr_mgr_status.
 * @return !=NULL The rtr_rtr_mgr_status as String.
 */
const char *rtr_mgr_status_to_str(enum rtr_mgr_status status);

/**
 * @brief Iterates over all IPv4 records in the pfx_table.
 * @details For every pfx_record the function fp is called. The pfx_record and
 * the data pointer is passed to the fp.
 * @param[in] config rtr_mgr_config
 * @param[in] fp Pointer to callback function with signature \c pfx_for_each_fp.
 * @param[in] data This parameter is forwarded to the callback function.
 */
void rtr_mgr_for_each_ipv4_record(struct rtr_mgr_config *config, pfx_for_each_fp fp, void *data);

/**
 * @brief Iterates over all IPv6 records in the pfx_table.
 * @details For every pfx_record the function fp is called. The pfx_record and
 * the data pointer is passed to the fp.
 * @param[in] config rtr_mgr_config
 * @param[in] fp Pointer to callback function with signature \c pfx_for_each_fp.
 * @param[in] data This parameter is forwarded to the callback function.
 */
void rtr_mgr_for_each_ipv6_record(struct rtr_mgr_config *config, pfx_for_each_fp fp, void *data);

/**
 * @brief Returns the first, thus active group.
 * @param[in] config The rtr_mgr_config
 * @return rtr_mgr_group The head of the linked list.
 */
struct rtr_mgr_group *rtr_mgr_get_first_group(struct rtr_mgr_config *config);

int rtr_mgr_for_each_group(struct rtr_mgr_config *config, void (*fp)(const struct rtr_mgr_group *group, void *data),
			   void *data);
#endif
/** @} */
