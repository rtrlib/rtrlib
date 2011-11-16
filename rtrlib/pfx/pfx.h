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
#include <sys/types.h>
#include <stdint.h>
#include "rtrlib/lib/ip.h"

/**
 * @brief Possible return values for pfx_ functions.
 */
enum pfx_rtvals{
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
typedef enum pfxv_state{
    /** A valid certificate for the pfx_record exists. */
    BGP_PFXV_STATE_VALID,

    /** @brief No entry which matches the prefix and prefix length exists. */
    BGP_PFXV_STATE_NOT_FOUND,

    /** Validation failed, one or more prefixes that match the input prefix exists in the pfx_table, but prefix
     * max_len or the ASN does'nt match. */
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
typedef struct pfx_record{
  uint32_t asn;
  ip_addr prefix;
  uint8_t min_len;
  uint8_t max_len;
  uintptr_t socket_id;
} pfx_record;

/**
 * @brief A function pointer that is called if an record was added or removed to the pfx_table.
 * @param pfx_table which was updated.
 * @param record pfx_record that was modified.
 * @param added True if the record was added, false if the record was removed
 * servers 
 */
typedef void (*pfx_update_fp)(struct pfx_table* pfx_table, const pfx_record record, const bool added);

/**
 * @brief Initialize the pfx_table struct.
 * @param[in] pfx_table pfx_table that will be initialized.
 * @param[in] update_fp Afunction pointers that will be called if a record was added or removed.
 */
void pfx_table_init(struct pfx_table* pfx_table, pfx_update_fp update_fp);

/**
 * @brief Free all memory associcated with pfx_table.
 * @param[in] pfx_table pfx_table what will be freed.
 */
void pfx_table_free(struct pfx_table* pfx_table);

/**
 * @brief Append pfx_record to pfx_table.
 * @param[in] pfx_table pfx_table to use.
 * @param[in] pfx_record pfx_record that will be added.
 * @return PFX_SUCCESS
 * @return PFX_ERROR
 * @return PFX_DUPLICATE_RECORD
 */
int pfx_table_add(struct pfx_table* pfx_table, const pfx_record* pfx_record);

/**
 * @brief Remove pfx_record from the pfx_table.
 * @param[in] pfx_table pfx_table to use.
 * @param[in] pfx_record Record that will be removed.
 * @return PFX_SUCCESS
 * @return PFX_ERROR
 * @return PFX_RECORD_NOT_FOUND
 */
int pfx_table_remove(struct pfx_table* pfx_table, const pfx_record* pfx_record);

/**
 * @brief Remove all entries from the source socket_id in pfx_table
 * @param[in] pfx_table pfx_table to use.
 * @param[in] socket_id ID of the rtr_socket
 * @return PFX_SUCCESS
 * @return PFX_ERROR
 */
int pfx_table_src_remove(struct pfx_table* pfx_table, const uintptr_t socket_id);

/**
 * @brief Validates the origin of a BGP-Route
 * @param[in] pfx_table pfx_table to use.
 * @param[in] asn Autonomous system number of the Origin-AS of the route.
 * @param[in] prefix Announcend network Prefix
 * @param[in] mask_len Length of the network mask of the announced prefix
 * @param[out] result Result of the validation.
 * @return PFX_SUCCESS On success.
 * @return PFX_ERROR If an error occured.
 */
int pfx_table_validate(struct pfx_table* pfx_table, const uint32_t asn, const ip_addr* prefix, const uint8_t mask_len, pfxv_state* result);

#endif
/* @} */
