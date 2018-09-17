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

#ifndef RTR_PFX_PRIVATE_H
#define RTR_PFX_PRIVATE_H
#include <stdint.h>

#include "rtrlib/pfx/pfx.h"
#include "rtrlib/lib/ip_private.h"

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
 * @brief Frees all memory associcated with the pfx_table without calling the update callback.
 * @param[in] pfx_table pfx_table that will be freed.
 */
void pfx_table_free_without_notify(struct pfx_table *pfx_table);

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
int pfx_table_validate(struct pfx_table *pfx_table, const uint32_t asn, const struct lrtr_ip_addr *prefix, const uint8_t mask_len, enum pfxv_state *result);

/**
 * @brief Copy content of @p src_table into @p dst_table
 * @details dst must be empty and initialized
 * @param[in] src_table Source table
 * @param[out] dst_table Destination table
 * @param[in] socket socket which prefixes should not be copied
 */
int pfx_table_copy_except_socket(struct pfx_table *src_table, struct pfx_table *dst_table, const struct rtr_socket *socket);

/**
 * @brief Swap root nodes of the argument tables
 * @param[in,out] a First table
 * @param[in,out] b second table
 */
void pfx_table_swap(struct pfx_table *a, struct pfx_table *b);

/**
 * @brief Notify client about changes between to pfx tables regarding one specific socket
 * @details old_table will be modified it should be freed after calling this function
 * @param[in] new_table
 * @param[in] old_table
 * @param[in] socket socket which prefixes should be diffed
 */
void pfx_table_notify_diff(struct pfx_table *new_table, struct pfx_table *old_table, const struct rtr_socket *socket);

#endif
/* @} */
