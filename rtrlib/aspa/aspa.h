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
 * The ASPA table stores records containg a customer ASN (CAS) and a number of provider ASNs. These records are grouped by
 * the socket they originated from so they can be purged easily right after any given socket dies, so filtering is not needed. The ASPA
 * records themselves are stored in an @c aspa_array dynamically ordering the records inside in order to make searching faster. The table
 * stores each of these @c aspa_array references alongside a reference to the beforementioned @c rtr_socket inside a linked
 * list. The linked list is needed for validation and in cases where the table is copied, like in @c rtr_sync_receive_and_store_apdus .
 * Adding or removing records, or purging all records for any given socket would involve performing a lookup on the linked list. This
 * issue is addressed by storing a reference to the corresponding @c aspa_array in each @c rtr_socket . All possible operations
 * on the ASPA table therefore check if the given @c aspa_table matches the one in the @c rtr_socket and use the
 * @c aspa_array from the socket directly instead of performing a lookup in the linked list. Most of the time, this "fast lookup"
 * mechanism is used since there's only a single "global" @c aspa_table anyway, namely the one stored in every @c rtr_socket .
 *
 *
 * @{
 */

#ifndef RTR_ASPA_H
#define RTR_ASPA_H

#include "aspa_array/aspa_array.h"

#include "rtrlib/rtr/rtr.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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

struct aspa_store_node {
	struct aspa_array *aspa_array;
	struct rtr_socket *rtr_socket;
	struct aspa_store_node *next;
};

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
enum aspa_rtvals {
	/** Operation was successful. */
	ASPA_SUCCESS = 0,

	/** Error occurred. */
	ASPA_ERROR = -1,

	/** The supplied aspa_record already exists in the aspa_table. */
	ASPA_DUPLICATE_RECORD = -2,

	/** aspa_record wasn't found in the aspa_table. */
	ASPA_RECORD_NOT_FOUND = -3
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
 * @brief Adds a aspa_record to a aspa_table.
 *
 * @param[in] aspa_table aspa_table to use.
 * @param[in] aspa_record aspa_record that will be added.
 * @return aspa_SUCCESS On success.
 
 * @return aspa_ERROR On error.
 * @return aspa_DUPLICATE_RECORD If an identical aspa_record already exists
 */
int aspa_table_add(struct aspa_table *aspa_table, struct aspa_record *aspa_record, struct rtr_socket *rtr_socket,
		   bool replace);

/**
 * @brief Removes aspa_record from aspa_table
 *
 * @param aspa_table aspa_table to use
 * @param aspa_record aspa_record to remove;
 * @return aspa_SUCCESS On success
 *
 * @return aspa_ERROR On error
 * @return aspa_RECORD_NOT_FOUND On record not found
 */
int aspa_table_remove(struct aspa_table *aspa_table, struct aspa_record *aspa_record, struct rtr_socket *rtr_socket);

/**
 * @brief Registers a new socket as source.
 *
 * @param[in] aspa_table aspa_table to use.
 * @param[in] rtr_socket origin socket of the record
 * @return aspa_SUCCESS On success.
 * @return aspa_ERROR On error.
 */
int aspa_table_src_remove(struct aspa_table *aspa_table, struct rtr_socket *rtr_socket);

/**
 * @brief Returns all aspa_record whose SKI number matches the given one.
 *
 * @param[in] aspa_table aspa_table to use
 * @param[in] customer_asn Customer ASN to search for
 * @param[out] result the result array. NULL if no records could be found
 * @param[out] result_size elment count of the result array
 * @return aspa_SUCCESS On success
 * @return aspa_ERROR On error
 */
int aspa_table_search_by_customer_asn(struct aspa_table *aspa_table, uint32_t *customer_asn,
				      struct aspa_record **result, unsigned int *result_size);

#endif
/** @} */
