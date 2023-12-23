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
 * @brief The aspa_table is an abstract data structure to organize the validated Autonomous System Provider Authorization  data
 * received from an RPKI-RTR cache server.
 *
 * @{
 */

#ifndef RTR_ASPA_H
#define RTR_ASPA_H

#include "aspa_array/aspa_array.h"

#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/rtr/rtr.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

/**
 * @brief ASPA Record
 * Customer (Customer Autonomous Systen, CAS) authorizes a set of provider AS numbers
 */
struct aspa_record {
	uint32_t customer_asn;
	size_t provider_count;
	uint32_t *provider_asns;
};

/**
 * @brief Computes size of a given ASPA record.
 * @param record The record whose size to compute.
 */
size_t aspa_size_of_aspa_record(const struct aspa_record *record);

struct aspa_table;

/**
 * @brief A function pointer that is called if an record was added to the aspa_table or was removed from the aspa_table.
 *
 * @param aspa_table which was updated.
 * @param record aspa_record that was modified.
 * @param added True if the record was added, false if the record was removed.
 */
typedef void (*aspa_update_fp)(struct aspa_table *aspa_table, const struct aspa_record record,
			       const struct rtr_socket *rtr_socket, const bool added);

/**
 * @brief ASPA Table

 * @property lock Read-Write lock to prevent data races.
 * @property Update function, called when the dynamic ordered array changes.
 * @property sockets sockets Sockets, each storing a dynamic ordered array
 */
struct aspa_table {
	pthread_rwlock_t lock;
	aspa_update_fp update_fp;
	struct aspa_store_node *store;
};

/**
 * @brief Possible return values for aspa_ functions.
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
 * @brief Validation states returned from  aspa_validate_as_path.
 */
enum as_path_state {
	// TODO: docs/naming
	/** <#...#> */
	BGP_AS_PATH_STATE_VALID,

	// TODO: docs/naming
	/** @brief <#...#> */
	BGP_AS_PATH_STATE_NOT_FOUND,

	// TODO: docs/naming
	/** @brief <#...#> */
	BGP_AS_PATH_STATE_INVALID
};

/**
 * @brief Initializes the aspa_table struct.
 *
 * @param[in] aspa_table aspa_table that will be initialized.
 * @param[in] update_fp Pointer to update function
 */
void aspa_table_init(struct aspa_table *aspa_table, aspa_update_fp update_fp);

/**
 * @brief Frees the memory associated with the aspa_table.
 *
 * @param[in] aspa_table aspa_table that will be initialized.
 * @param notify A boolean value determining whether to notify clients about records being removed from the table.
 */
void aspa_table_free(struct aspa_table *aspa_table, bool notify);

/**
 * @brief Removes all entries in the spki_table that match the passed socket_id.
 *
 * @param[in] aspa_table ASPA table to use.
 * @param[in] socket origin socket of the record
 * @return @c SPKI_SUCCESS On success.
 * @return @c SPKI_ERROR On error.
 */
enum aspa_status aspa_table_src_remove(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket, bool notify);

enum aspa_direction { ASPA_UPSTREAM, ASPA_DOWNSTREAM };

enum aspa_verification_result {
	ASPA_AS_PATH_UNKNOWN,
	ASPA_AS_PATH_INVALID,
	ASPA_AS_PATH_VALID,
};

/**
 * @brief Verifies an AS_PATH .
 *
 * Implements an optimized version of the ASPA verification algorithm described in section 6.1 of
 * https://datatracker.ietf.org/doc/draft-ietf-sidrops-aspa-verification/16/ .
 *
 * @param[in] aspa_table ASPA table to use
 * @param[in] direction @c AS_PATH direction, as explained in the draft
 * @param[in] as_path @c AS_PATH array to be validated: concatenated of BGP UPDATE's @c AS_PATHs
 * @param[in] len the length of @p as_path array
 * @return @c ASPA_AS_PATH_UNKNOWN if the @c AS_PATH cannot be fully verified
 * @return @c ASPA_AS_PATH_INVALID if @c AS_PATH is invalid
 * @return @c ASPA_AS_PATH_VALID if @c AS_PATH is valid
 */
enum aspa_verification_result aspa_verify_as_path(struct aspa_table *aspa_table, uint32_t as_path[], size_t len,
						  enum aspa_direction direction);

/**
 * @brief Verifies an upstream @c AS_PATH .
 *
 * Implements an optimized version of the ASPA verification algorithm described in section 6.1 of
 * https://datatracker.ietf.org/doc/draft-ietf-sidrops-aspa-verification/16/ .
 *
 * @param[in] aspa_table ASPA table to use
 * @param[in] as_path @c AS_PATH array to be validated: concatenated of BGP UPDATE's @c AS_PATHs
 * @param[in] len the length of @p as_path array
 * @return @c ASPA_AS_PATH_UNKNOWN if the @c AS_PATH cannot be fully verified
 * @return @c ASPA_AS_PATH_INVALID if @c AS_PATH is invalid
 * @return @c ASPA_AS_PATH_VALID if @c AS_PATH is valid
 */
enum aspa_verification_result aspa_verify_as_path_upstream(struct aspa_table *aspa_table, uint32_t as_path[],
							   size_t len);

/**
 * @brief Verifies a downstream @c AS_PATH .
 *
 * Implements an optimized version of the ASPA verification algorithm described in section 6.1 of
 * https://datatracker.ietf.org/doc/draft-ietf-sidrops-aspa-verification/16/ .
 *
 * @param[in] aspa_table ASPA table to use
 * @param[in] as_path @c AS_PATH array to be validated: concatenated of BGP UPDATE's @c AS_PATHs
 * @param[in] len the length of @p as_path array
 * @return @c ASPA_AS_PATH_UNKNOWN if the @c AS_PATH cannot be fully verified
 * @return @c ASPA_AS_PATH_INVALID if @c AS_PATH is invalid
 * @return @c ASPA_AS_PATH_VALID if @c AS_PATH is valid
 */
enum aspa_verification_result aspa_verify_as_path_downstream(struct aspa_table *aspa_table, uint32_t as_path[],
							     size_t len);

/**
 * @brief Collapses an @c AS_PATH in-place, replacing in-series repetitions with single occurences
 * @return len of collapsed array
 */
size_t aspa_collapse_as_path(uint32_t as_path[], size_t len);

#endif
/** @} */
