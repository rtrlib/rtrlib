/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

/**
 * @defgroup mod_pfx_h Prefix validation table
 * @brief The pfx_table is an abstract data structure to organize the validated prefix origin data received from an RPKI-RTR cache server.
 *
 * @{
 */

#ifndef RTR_PFX_H
#define RTR_PFX_H

#include "rtrlib/lib/ip.h"
#include "rtrlib/pfx/trie/trie-pfx.h"

#include <inttypes.h>

/**
 * @brief Possible return values for pfx_ functions.
 */
enum pfx_rtvals {
	/** Operation was successful. */
	PFX_SUCCESS = 0,

	/** Error occured. */
	PFX_ERROR = -1,

	/** The supplied pfx_record already exists in the pfx_table. */
	PFX_DUPLICATE_RECORD = -2,

	/** pfx_record wasn't found in the pfx_table. */
	PFX_RECORD_NOT_FOUND = -3
};

/**
 * @brief Validation states returned from  pfx_validate_origin.
 */
enum pfxv_state {
	/** A valid certificate for the pfx_record exists. */
	BGP_PFXV_STATE_VALID,

	/** @brief No certificate for the route exists. */
	BGP_PFXV_STATE_NOT_FOUND,

	/** @brief One or more records that match the input prefix exists in the pfx_table but the prefix max_len or ASN does'nt match. */
	BGP_PFXV_STATE_INVALID
};

/**
 * @brief A function pointer that is called for each record in the pfx_table.
 * @param pfx_record
 * @param data forwarded data which the user has passed to pfx_table_for_each_ipv4_record() or
 * pfx_table_for_each_ipv6_record()
 */
typedef void (*pfx_for_each_fp)(const struct pfx_record *pfx_record, void *data);

/**
 * @brief Initializes the pfx_table struct.
 * @param[in] pfx_table pfx_table that will be initialized.
 * @param[in] update_fp Afunction pointers that will be called if a record was added or removed.
 */
void pfx_table_init(struct pfx_table *pfx_table, pfx_update_fp update_fp);

/**
 * @brief Frees all memory associcated with the pfx_table.
 * @param[in] pfx_table pfx_table that will be freed.
 */
void pfx_table_free(struct pfx_table *pfx_table);

/**
 * @brief Adds a pfx_record to a pfx_table.
 * @param[in] pfx_table pfx_table to use.
 * @param[in] pfx_record pfx_record that will be added.
 * @return PFX_SUCCESS On success.
 * @return PFX_ERROR On error.
 * @return PFX_DUPLICATE_RECORD If the pfx_record already exists.
 */
int pfx_table_add(struct pfx_table *pfx_table, const struct pfx_record *pfx_record);

/**
 * @brief Removes a pfx_record from a pfx_table.
 * @param[in] pfx_table pfx_table to use.
 * @param[in] pfx_record Record that will be removed.
 * @return PFX_SUCCESS On success.
 * @return PFX_ERROR On error.
 * @return PFX_RECORD_NOT_FOUND If pfx_records could'nt be found.
 */
int pfx_table_remove(struct pfx_table *pfx_table, const struct pfx_record *pfx_record);

/**
 * @brief Removes all entries in the pfx_table that match the passed socket_id value from a pfx_table.
 * @param[in] pfx_table pfx_table to use.
 * @param[in] socket origin socket of the record
 * @return PFX_SUCCESS On success.
 * @return PFX_ERROR On error.
 */
int pfx_table_src_remove(struct pfx_table *pfx_table, const struct rtr_socket *socket);

/**
 * @brief Validates the origin of a BGP-Route.
 * @param[in] pfx_table pfx_table to use.
 * @param[in] asn Autonomous system number of the Origin-AS of the route.
 * @param[in] prefix Announcend network Prefix.
 * @param[in] mask_len Length of the network mask of the announced prefix.
 * @param[out] result Result of the validation.
 * @return PFX_SUCCESS On success.
 * @return PFX_ERROR On error.
 */
int pfx_table_validate(struct pfx_table *pfx_table, const uint32_t asn, const struct lrtr_ip_addr *prefix,
		       const uint8_t mask_len, enum pfxv_state *result);

/**
 * @brief Validates the origin of a BGP-Route and returns a list of pfx_record that decided the result.
 * @param[in] pfx_table pfx_table to use.
 * @param[out] reason Pointer to a memory area that will be used as array of pfx_records. The memory area will be overwritten. Reason must point to NULL or an allocated memory area.
 * @param[out] reason_len Size of the array reason.
 * @param[in] asn Autonomous system number of the Origin-AS of the route.
 * @param[in] prefix Announcend network Prefix
 * @param[in] mask_len Length of the network mask of the announced prefix
 * @param[out] result Result of the validation.
 * @return PFX_SUCCESS On success.
 * @return PFX_ERROR On error.
 */
int pfx_table_validate_r(struct pfx_table *pfx_table, struct pfx_record **reason, unsigned int *reason_len,
			 const uint32_t asn, const struct lrtr_ip_addr *prefix, const uint8_t mask_len,
			 enum pfxv_state *result);

/**
 * @brief Iterates over all IPv4 records in the pfx_table.
 * @details For every pfx_record the function fp is called. The pfx_record and
 * the data pointer is passed to the fp.
 * @param[in] pfx_table
 * @param[in] fp A pointer to a callback function with the signature \c pfx_for_each_fp.
 * @param[in] data This parameter is forwarded to the callback function.
 */
void pfx_table_for_each_ipv4_record(struct pfx_table *pfx_table, pfx_for_each_fp fp, void *data);

/**
 * @brief Iterates over all IPv6 records in the pfx_table.
 * @details For every pfx_record the function fp is called. The pfx_record and
 * the data pointer is passed to the fp.
 * @param[in] pfx_table
 * @param[in] fp A pointer to a callback function with the signature \c pfx_for_each_fp.
 * @param[in] data This parameter is forwarded to the callback function.
 */
void pfx_table_for_each_ipv6_record(struct pfx_table *pfx_table, pfx_for_each_fp fp, void *data);

#endif
/** @} */
