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


#include "rtrlib/keys/tommyds-1.8/tommy.h"
#include <pthread.h>
/**
 * @brief A function pointer that is called by the key_table to find elements.
 * @param arg Pointer at the value to search, in our case a uint32_t asn.
 * @param obj Pointer at the object to compare to, in our case struct key_entry.
 * @return 0 if the value matches the element, != 0 if different
 */
typedef int (*hash_cmp_fp)(const void *arg, const void *obj);

/**
 * @brief key_table.
 * @param hashtable Linear hashtable
 * @param lock
 */

struct key_table {
	tommy_hashlin *hashtable;
	hash_cmp_fp cmp_fp;
    pthread_rwlock_t lock;
};


/**
 * @brief key_entry.
 * @param key_pdu The complete pdu_router_key struct, see packets.c
 * @param asn Origin AS number, used as the hash index (I use index to not confuse it with the crypto keys).
 * @param next In case this key_entry returns from key_table_get(..), this points to the next found key_entry.
 * @param node Required by the tommyDS hashtable implementation we are using. Stores info about the hashed object.
 */

struct key_entry {
	struct pdu_router_key *key_pdu;
    uint32_t asn;
    struct key_entry *next;
    tommy_node node;
};


/**
 * @brief Possible return values for some key_table_ functions.
 */
enum key_rtvals {
    /** Operation was successfull. */
    KEY_SUCCESS = 0,

    /** Error occured. */
    KEY_ERROR = -1,

    //TODO:
    // /** The supplied key_entry already exists in the key_table. */
    // KEY_DUPLICATE_RECORD = -2,

    // /** key_entry wasn't found in the key_table. */
    // KEY_RECORD_NOT_FOUND = -3
};

/**
 * @brief Function that compares arg to the ASN value of obj
 */
int asn_cmp(const void *arg, const void *obj);


/**
 * @brief Initializes the key_table struct.
 * @param[in] key_table key_table that will be initialized.
 */
void key_table_init(struct key_table *key_table);





/**
 * @brief Adds a key_entry to a key_table.
 * @param[in] key_table key_table to use.
 * @param[in] key_entry key_entry that will be added.
 * @return KEY_SUCCESS On success.
 * @return KEY_ERROR On error.
 * @return KEY_DUPLICATE_RECORD // TODO
 */
int key_table_add(struct key_table *key_table, struct key_entry *key_entry);


/**
 * @brief Returns all key_entry whose AS number matches asn. Multiple key_entry are linked
 * with their "next" field
 * @param key_table key_table to use
 * @param asn Hash index to look for
 */
struct key_entry* key_table_get(struct key_table *key_table, uint32_t asn);
