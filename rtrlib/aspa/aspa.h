/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

/**
 * @defgroup mod_aspa_h ASPA validation table
 *
 * @brief The aspa_table is an abstract data structure to organize the validated
 * Autonomous System Provider Authorization data received from an RPKI-RTR
 * cache server.
 *
 * @{
 */

#ifndef RTR_ASPA_H
#define RTR_ASPA_H

#include "rtrlib/aspa/aspa_array/aspa_array.h"
#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/rtr/rtr.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief ASPA Record
 * Customer (Customer Autonomous Systen, CAS) authorizes a set of provider AS numbers.
 *
 * @param customer_asn Customer ASN
 * @param provider_count The number of providers this customer declares.
 * @param provider_asns An array of provider ASNs.
 */
struct aspa_record {
	uint32_t customer_asn;
	size_t provider_count;
	uint32_t *provider_asns;
};

// MARK: - ASPA Table

struct aspa_table;

/**
 * @brief An enum describing the type of operation the ASPA table should perform using any given ASPA record.
 */
enum __attribute__((__packed__)) aspa_operation_type {
	/** The existing record, identified by its customer ASN, shall be withdrawn from the ASPA table. */
	ASPA_REMOVE = 0,

	/** The new record, identified by its customer ASN, shall be added to the ASPA table. */
	ASPA_ADD = 1
};

/**
 * @brief A function pointer that is called if an record was added to the @p aspa_table
 * or was removed from the @p aspa_table.
 *
 * @param aspa_table ASPA table which was updated.
 * @param record ASPA record that was modified.
 * @param rtr_socket The socket the record originated from
 * @param operation_type The type of this operation.
 */
typedef void (*aspa_update_fp)(struct aspa_table *aspa_table, const struct aspa_record record,
			       const struct rtr_socket *rtr_socket, const enum aspa_operation_type operation_type);

/**
 * @brief ASPA Table

 * @param lock Read-Write lock to prevent data races.
 * @param update_lock Read-Write lock to prevent changes made to the table while an update is in progress.
 * @param update function, called when the dynamic ordered array changes.
 * @param sockets sockets Sockets, each storing a dynamic ordered array
 *
 * An ASPA table consists of a linked list of a sockets  and ASPA arrays, simplifying removing or replacing records
 * originating from any given socket.
 */
struct aspa_table {
	pthread_rwlock_t lock;
	pthread_rwlock_t update_lock;
	aspa_update_fp update_fp;
	struct aspa_store_node *store;
};

/**
 * @brief Possible return values for `aspa_*` functions.
 */
enum aspa_status {
	/** Operation was successful. */
	ASPA_SUCCESS = 0,

	/** Error occurred. */
	ASPA_ERROR = -1,

	/** The supplied aspa_record already exists in the aspa_table. */
	ASPA_DUPLICATE_RECORD = -2,

	/** aspa_record wasn't found in the aspa_table. */
	ASPA_RECORD_NOT_FOUND = -3,
};

/**
 * @brief Initializes the @p aspa_table struct.
 *
 * @param[in] aspa_table aspa_table that will be initialized.
 * @param[in] update_fp Pointer to update function
 */
void aspa_table_init(struct aspa_table *aspa_table, aspa_update_fp update_fp);

/**
 * @brief Frees the memory associated with the @p aspa_table
 *
 * @param[in] aspa_table aspa_table that will be initialized.
 * @param notify A boolean value determining whether to notify clients about records being removed from the table.
 */
void aspa_table_free(struct aspa_table *aspa_table, bool notify);

/**
 * @brief Removes all records in the @p aspa_table that originated from the socket.
 *
 * @param aspa_table ASPA table to use.
 * @param rtr_socket Record's origin socket.
 * @param notify A boolean value determining whether clients should be notified about the records' removal.
 * @return @c SPKI_SUCCESS On success.
 * @return @c SPKI_ERROR On error.
 */
enum aspa_status aspa_table_src_remove(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket, bool notify);

// MARK: - AS_PATH Verification

enum aspa_direction { ASPA_UPSTREAM, ASPA_DOWNSTREAM };

/**
 * @brief AS_PATH verification result.
 */
enum aspa_verification_result {
	ASPA_AS_PATH_UNKNOWN,
	ASPA_AS_PATH_INVALID,
	ASPA_AS_PATH_VALID,
};

/**
 * @brief Verifies an AS_PATH .
 *
 * Implements an optimized version of the ASPA verification algorithm described in section 6 of
 * https://datatracker.ietf.org/doc/draft-ietf-sidrops-aspa-verification/16/ .
 *
 * @param[in] aspa_table ASPA table to use.
 * @param[in] direction `AS_PATH` direction, as explained in the draft
 * @param[in] as_path `AS_PATH` array to be validated: concatenated of BGP UPDATEs' `AS_PATH`s
 * @param[in] len the length of @p as_path array
 * @return @c ASPA_AS_PATH_UNKNOWN if the `AS_PATH` cannot be fully verified
 * @return @c ASPA_AS_PATH_INVALID if `AS_PATH` is invalid
 * @return @c ASPA_AS_PATH_VALID if `AS_PATH` is valid
 */
enum aspa_verification_result aspa_verify_as_path(struct aspa_table *aspa_table, uint32_t as_path[], size_t len,
						  enum aspa_direction direction);

/**
 * @brief Collapses an `AS_PATH` in-place, replacing in-series repetitions with single occurences
 *
 * @return Length of the given array.
 */
size_t aspa_collapse_as_path(uint32_t as_path[], size_t len);

#endif /* RTR_ASPA_H */
/** @} */
