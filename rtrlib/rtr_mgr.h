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
 * RTRlib also supports a Retry Interval (see <a
 * href="https://tools.ietf.org/html/draft-ietf-sidr-rpki-rtr-rfc6810-bis">draft-ietf-sidr-rpki-rtr-rfc6810-bis</a>).
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
#include "rtrlib/rtr/rtr.h"
#include "rtrlib/spki/spkitable.h"

/**
 * @brief Status of a rtr_mgr_group.
 */
enum rtr_mgr_status {
    /** RTR sockets are disconnected */
    RTR_MGR_CLOSED,

    /** RTR sockets trying to establish a connection. */
    RTR_MGR_CONNECTING,

    /** All RTR sockets of the group are synchronized with the rtr servers. */
    RTR_MGR_ESTABLISHED,

    /** Error occured on at least one RTR socket. */
    RTR_MGR_ERROR,
};

/**
 * @brief A set of RTR sockets.
 * @param sockets Array of rtr_socket pointer. The tr_socket element of the rtr_socket must be associated with an initialized transport socket.
 * @param sockets_len Number of elements in the sockets array.
 * @param preference The preference value of this group. Groups with lower preference values are preferred.
 * @param status Status of the group.
 */
struct rtr_mgr_group {
    struct rtr_socket **sockets;
    unsigned int sockets_len;
    uint8_t preference;
    enum rtr_mgr_status status;
};

typedef void (*rtr_mgr_status_fp)(const struct rtr_mgr_group *, enum rtr_mgr_status, const struct rtr_socket *, void *);


/**
 * Type alias \c pfx_for_each_fp for function signature: \code (*)(const struct pfx_record *pfx_record, void *data) \endcode
 */
typedef void (*pfx_for_each_fp)(const struct pfx_record *pfx_record, void *data);

struct rtr_mgr_config {
    struct rtr_mgr_group *groups;
    unsigned int len;
    pthread_mutex_t mutex;
    rtr_mgr_status_fp status_fp;
    void *status_fp_data;
};

/**
 * @brief Initializes a rtr_mgr_config.
 * @param[out] config_out The rtr_mgr_config that will be initialized by this function. On error, *config_out will be NULL!
 * @param[in] groups Array of rtr_mgr_group. Every RTR socket in an
 *		     rtr_mgr_group must be assoziated with an initialized
 *		     transport socket. A Transport socket is only allowed to be
 *		     associated with one rtr socket. The preference values must
 *		     be unique in the group array. More than one rtr_mgr_group
 *		     with the same preference value isn't allowed.
 * @param[in] groups_len Number of elements in the groups array.
 * @param[in] refresh_interval Interval in seconds between serial queries that are sent to the server. Must be >= 1 and <= 86400s (one day),
 * recommended default is 3600s (one hour).
 * @param[in] expire_interval Stored validation records will be deleted if cache was unable to refresh data for this period.
 * The value should be twice the refresh_interval. The value must be >= 600s (ten minutes) and <= 172800s (two days).
 * The recommanded default is 7200s (two hours).
 * @param[in] retry_interval This parameter tells the router how long to wait (in seconds) before retrying
 * a failed Serial Query or Reset Query. The value must be >= 1s and <= 7200s (two hours).
 * The recommanded default is 600 seconds (ten minutes).
 * @param[in] update_fp A Pointer to a pfx_update_fp callback, that is executed for every added and removed pfx_record.
 * @param[in] spki_update_fp A Pointer to a spki_update_fp callback, that is executed for every added and removed spki_record.
 * @param[in] status_fp Pointer to a function that is called if the connection
 *	                status from one of the socket groups is changed.
 * @param[in] status_fp_data Pointer to a memory area that is passed to the
 *			     status_fp function. Memory area can be freely used
 *			     to pass user-defined data to the status_fp
 *			     callback.
 * @return RTR_ERROR If an error occurred
 * @return RTR_INVALID_PARAM If the refresh_interval or the expire_interval is invalid.
 * @return RTR_SUCCESS On success.
 */
int rtr_mgr_init(struct rtr_mgr_config **config_out, struct rtr_mgr_group groups[], const unsigned int groups_len,
                                    const unsigned int refresh_interval, const unsigned int expire_interval, const unsigned int retry_interval,
                                    const pfx_update_fp update_fp,
                                    const spki_update_fp spki_update_fp,
                                    const rtr_mgr_status_fp status_fp,
                                    void *status_fp_data);

/**
 * @brief Frees all resources that were allocated from the rtr_mgr.
 * rtr_mgr_stop(..) must be called before, to shutdown all RTR socket connections.
 * @param[in] config rtr_mgr_config.
 */
void rtr_mgr_free(struct rtr_mgr_config *config);

/**
 * @brief Establishes the connection with the rtr_sockets of the group with the lowest preference value and handles errors as defined in the RPKI-RTR protocol.
 * @param[in] config Pointer to an initialized rtr_mgr_config.
 * @return RTR_SUCCESS On success
 * @return RTR_ERROR On error
 */
int rtr_mgr_start(struct rtr_mgr_config *config);

/**
 * @brief Terminates all rtr_socket connections that are defined in the config. All pfx_records received from these sockets will be purged.
 * @param[in] config The rtr_mgr_config struct
 */
void rtr_mgr_stop(struct rtr_mgr_config *config);

/**
 * @brief Detects if the rtr_mgr_group is fully synchronized with at least one group.
 * @param[in] config The rtr_mgr_config.
 * @return true If the pfx_table stores non-outdated pfx_records from at least one socket group.
 * @return false If the pfx_table isn't fully synchronized with at least one group.
 */
bool rtr_mgr_conf_in_sync(struct rtr_mgr_config *config);

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
int rtr_mgr_validate(struct rtr_mgr_config *config, const uint32_t asn, const struct lrtr_ip_addr *prefix, const uint8_t mask_len, enum pfxv_state *result);


/**
 * @brief Returns all SPKI records which match the given ASN and SKI.
 * @param[in] config
 * @param[in] asn Autonomous system number of the Origin-AS.
 * @param[in] ski the SKI to search for.
 * @param[out] result a array of all matching spki_records
 * @param[out] result_count number of returned spki_records
 * @return SPKI_SUCCESS On success.
 * @return SPKI_ERROR If an error occurred.
 */
int rtr_mgr_get_spki(struct rtr_mgr_config *config, const uint32_t asn, uint8_t *ski, struct spki_record *result, unsigned int *result_count);

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
 * @param[in] fp A pointer to a callback function with the signature \c pfx_for_each_fp.
 * @param[in] data This parameter is forwarded to the callback function.
 */
void rtr_mgr_for_each_ipv4_record(struct rtr_mgr_config *config, pfx_for_each_fp fp, void *data);

/**
 * @brief Iterates over all IPv6 records in the pfx_table.
 * @details For every pfx_record the function fp is called. The pfx_record and
 * the data pointer is passed to the fp.
 * @param[in] config rtr_mgr_config
 * @param[in] fp A pointer to a callback function with the signature \c pfx_for_each_fp.
 * @param[in] data This parameter is forwarded to the callback function.
 */
void rtr_mgr_for_each_ipv6_record(struct rtr_mgr_config *config, pfx_for_each_fp fp, void *data);

#endif
/* @} */
