/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

/**
 * @defgroup mod_spki_h Subject Public Key Info table
 * @brief The spki_table is an abstract data structure to organize the received Router Key PDUs
 * from a RPKI-RTR cache server.
 *
 * @{
 */

#ifndef RTR_SPKI_PRIVATE_H
#define RTR_SPKI_PRIVATE_H

#include "rtrlib/spki/spkitable.h"

#include <stdint.h>

/**
 * @brief Possible return values for some spki_table_ functions.
 */
enum spki_rtvals {
	/** Operation was successful. */
	SPKI_SUCCESS = 0,

	/** Error occurred. */
	SPKI_ERROR = -1,

	/** The supplied spki_record already exists in the spki_table. */
	SPKI_DUPLICATE_RECORD = -2,

	/** spki_record wasn't found in the spki_table. */
	SPKI_RECORD_NOT_FOUND = -3
};

/**
 * @brief Initializes the spki_table struct.
 * @param[in] spki_table spki_table that will be initialized.
 * @param[in] update_fp Pointer to update function
 */
void spki_table_init(struct spki_table *spki_table, spki_update_fp update_fp);

/**
 * @brief Frees the memory associated with the spki_table.
 * @param[in] spki_table spki_table that will be initialized.
 */
void spki_table_free(struct spki_table *spki_table);

/**
 * @brief Frees the memory associated with the spki_table without calling the update callback.
 * @param[in] spki_table spki_table that will be initialized.
 */
void spki_table_free_without_notify(struct spki_table *spki_table);

/**
 * @brief Adds a spki_record to a spki_table.
 * @param[in] spki_table spki_table to use.
 * @param[in] spki_record spki_record that will be added.
 * @return SPKI_SUCCESS On success.
 * @return SPKI_ERROR On error.
 * @return SPKI_DUPLICATE_RECORD If an identical spki_record already exists
 */
int spki_table_add_entry(struct spki_table *spki_table, struct spki_record *spki_record);

/**
 * @brief Returns all spki_record whose ASN and SKI matches.
 * @param[in] spki_table spki_table to use
 * @param[in] asn the AS number
 * @param[in] ski the 20 byte field which contains the SKI to search for
 * @param[out] result the result array. NULL if no records could be found
 * @param[out] result_size elment count of the result array
 * @return SPKI_SUCCESS On success
 * @return SPKI_ERROR On error
 */
int spki_table_get_all(struct spki_table *spki_table, uint32_t asn, uint8_t *ski, struct spki_record **result,
		       unsigned int *result_size);

/**
 * @brief Returns all spki_record whose SKI number matches the given one.
 * @param[in] spki_table spki_table to use
 * @param[in] ski the 20 byte field which contains the SKI to search for
 * @param[out] result the result array. NULL if no records could be found
 * @param[out] result_size elment count of the result array
 * @return SPKI_SUCCESS On success
 * @return SPKI_ERROR On error
 */
int spki_table_search_by_ski(struct spki_table *spki_table, uint8_t *ski, struct spki_record **result,
			     unsigned int *result_size);

/**
 * @brief Removes spki_record from spki_table
 * @param spki_table spki_table to use
 * @param spki_record spki_record to remove;
 * @return SPKI_SUCCESS On success
 * @return SPKI_ERROR On error
 * @return SPKI_RECORD_NOT_FOUND On record not found
 */
int spki_table_remove_entry(struct spki_table *spki_table, struct spki_record *spki_record);

/**
 * @brief Removes all entries in the spki_table that match the passed socket_id.
 * @param[in] spki_table spki_table to use.
 * @param[in] socket origin socket of the record
 * @return SPKI_SUCCESS On success.
 * @return SPKI_ERROR On error.
 */
int spki_table_src_remove(struct spki_table *spki_table, const struct rtr_socket *socket);

/**
 * @brief Copy spki table except entries from the given socket
 * @param[in] src source table
 * @param[in] dest target table
 * @param[in] socket socket which entries should not be copied
 * @return SPKI_SUCCESS On success.
 * @return SPKI_ERROR On error.
 */
int spki_table_copy_except_socket(struct spki_table *src, struct spki_table *dest, struct rtr_socket *socket);

/**
 * @brief Notify client about changes between two spki tables regarding one specific socket
 * @details old_table will be modified and should probebly be freed after calling this function
 * @param[in] new_table
 * @param[in] old_table
 * @param[in] socket socket which entries should be diffed
 */
void spki_table_notify_diff(struct spki_table *new_table, struct spki_table *old_table,
			    const struct rtr_socket *socket);

/**
 * @brief tommy_hashlin and tommy_list of the argument tables
 * @param[in] a
 * @param[in] b
 */
void spki_table_swap(struct spki_table *a, struct spki_table *b);

#endif
/** @} */
