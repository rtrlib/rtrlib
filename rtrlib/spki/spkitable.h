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
 */
#ifndef RTR_SPKI_H
#define RTR_SPKI_H


#include <stdint.h>
#include <sys/types.h>
#include "rtrlib/lib/ip.h"
#include "rtrlib/rtr/rtr.h"

#define SKI_SIZE 20
#define SPKI_SIZE 91

/**
 * @brief Possible return values for some spki_table_ functions.
 */
enum spki_rtvals {
    /** Operation was successfull. */
    SPKI_SUCCESS = 0,

    /** Error occured. */
    SPKI_ERROR = -1,

    /** The supplied spki_record already exists in the spki_table. */
    SPKI_DUPLICATE_RECORD = -2,

    // /** spki_record wasn't found in the spki_table. */
    SPKI_RECORD_NOT_FOUND = -3
};

struct spki_table;

/**
 * @brief spki_record.
 * @param router_key The router key struct with the {ASN, SKI, Public Key} triple
 * @param asn Origin AS number, used as the hash index (I use index to not confuse it with the crypto keys).
 * @param next In case this spki_record returns from spki_table_get(..), this points to the next found spki_record.
 * @param hash_node Used to store the spki_record in spki_table->hashtable, for fast insert/remove/validate operations
 * @param list_node Used to store the spki_record in spki_table->list, to make them iterable (used for purging)
 */
struct spki_record {
    uint8_t ski[SKI_SIZE];
    uint32_t asn;
    uint8_t spki[SPKI_SIZE];
    const struct rtr_socket *socket;
};

/**
 * @brief A function pointer that is called if an record was added to the spki_table or was removed from the spki_table.
 * @param spki_table which was updated.
 * @param record spki_record that was modified.
 * @param added True if the record was added, false if the record was removed.
 */
typedef void (*spki_update_fp)(struct spki_table *spki_table, const struct spki_record record, const bool added);

/**
 * @brief Initializes the spki_table struct.
 * @param[in] spki_table spki_table that will be initialized.
 */
void spki_table_init(struct spki_table *spki_table, spki_update_fp update_fp);


/**
 * @brief Frees the memory associcated with the spki_table.
 * @param[in] spki_table spki_table that will be initialized.
 */
void spki_table_free(struct spki_table *spki_table);


/**
 * @brief Adds a spki_record to a spki_table.
 * @param[in] spki_table spki_table to use.
 * @param[in] spki_record spki_record that will be added.
 * @return KEY_SUCCESS On success.
 * @return KEY_ERROR On error.
 * @return KEY_DUPLICATE_RECORD // TODO
 */
int spki_table_add_entry(struct spki_table *spki_table, struct spki_record *spki_record);


/**
 * @brief Returns all spki_record whose AS number matches asn and SKI matches ski. Multiple spki_record are linked
 * with their "next" field
 * @param[in] spki_table spki_table to use
 * @param[in] asn Hash index to look for
 * @param[in] ski the 20 byte field which contains the SKI to search for
 * @param[out] result the result array. NULL if no records could be found
 * @param[out] result_size elment count of the result array
 */
int spki_table_get_all(struct spki_table *spki_table, uint32_t asn, uint8_t *ski, struct spki_record **result, unsigned int *result_size);


/**
 * @brief Removes spki_record from spki_table
 * @param spki_table spki_table to use
 * @param spki_record spki_record to remove;
 */
int spki_table_remove_entry(struct spki_table *spki_table, struct spki_record *spki_record);


/**
 * @brief Removes all entries in the spki_table that match the passed socket_id.
 * @param[in] spki_table spki_table to use.
 * @param[in] socket origin socket of the record
 * @return KEY_SUCCESS On success.
 * @return KEY_ERROR On error.
 */
int spki_table_src_remove(struct spki_table *spki_table, const struct rtr_socket *socket);

#endif
