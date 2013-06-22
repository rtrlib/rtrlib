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
 * @defgroup mod_pfx_h Prefix validation table
 * @brief The pfx_table is an abstract data structure to organize the validated prefix origin data received from an RPKI-RTR cache server.
 *
 * @{
 */

#ifndef RTR_PFX_H
#define RTR_PFX_H
#include <stdint.h>
#include <sys/types.h>
#include "rtrlib/lib/ip.h"

/**
 * @brief Possible return values for pfx_ functions.
 */
enum pfx_rtvals {
    /** Operation was successfull. */
    PFX_SUCCESS = 0,

    /** Error occured. */
    PFX_ERROR = -1,

    /** The supplied pfx_record already exists in the pfx_table. */
    PFX_DUPLICATE_RECORD = -2,

    /** pfx_record wasn't found in the pfx_table. */
    PFX_RECORD_NOT_FOUND = -3
};

struct pfx_table;

/**
 * @brief Validation states returned from  pfx_validate_origin.
 */
typedef enum pfxv_state {
    /** A valid certificate for the pfx_record exists. */
    BGP_PFXV_STATE_VALID,

    /** @brief No certificate for the route exists. */
    BGP_PFXV_STATE_NOT_FOUND,

    /** @brief One or more records that match the input prefix exists in the pfx_table but the prefix max_len or ASN does'nt match. */
    BGP_PFXV_STATE_INVALID
} pfxv_state;


/**
 * @brief pfx_record.
 * @param asn Origin AS number.
 * @param prefix IP prefix.
 * @param min_len Minimum prefix length.
 * @param max_len Maximum prefix length.
 * @param socket_id unique id of the rtr_socket that received this record.
 */
typedef struct pfx_record {
    uint32_t asn;
    ip_addr prefix;
    uint8_t min_len;
    uint8_t max_len;
    uintptr_t socket_id;
} pfx_record;

/**
 * @brief A function pointer that is called if an record was added to the pfx_table or was removed from the pfx_table.
 * @param pfx_table which was updated.
 * @param record pfx_record that was modified.
 * @param added True if the record was added, false if the record was removed.
 */
typedef void (*pfx_update_fp)(struct pfx_table *pfx_table, const pfx_record record, const bool added);

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
int pfx_table_add(struct pfx_table *pfx_table, const pfx_record *pfx_record);

/**
 * @brief Removes a pfx_record from a pfx_table.
 * @param[in] pfx_table pfx_table to use.
 * @param[in] pfx_record Record that will be removed.
 * @return PFX_SUCCESS On success.
 * @return PFX_ERROR On error.
 * @return PFX_RECORD_NOT_FOUND If pfx_records could'nt be found.
 */
int pfx_table_remove(struct pfx_table *pfx_table, const pfx_record *pfx_record);

/**
 * @brief Removes all entries in the pfx_table that match the passed socket_id value from a pfx_table.
 * @param[in] pfx_table pfx_table to use.
 * @param[in] socket_id ID of the rtr_socket.
 * @return PFX_SUCCESS On success.
 * @return PFX_ERROR On error.
 */
int pfx_table_src_remove(struct pfx_table *pfx_table, const uintptr_t socket_id);

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
int pfx_table_validate(struct pfx_table *pfx_table, const uint32_t asn, const ip_addr *prefix, const uint8_t mask_len, pfxv_state *result);

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
int pfx_table_validate_r(struct pfx_table *pfx_table, pfx_record **reason, unsigned int *reason_len,  const uint32_t asn, const ip_addr *prefix, const uint8_t mask_len, pfxv_state *result);

#endif
/* @} */
