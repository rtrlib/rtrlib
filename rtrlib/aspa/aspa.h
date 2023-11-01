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

#include "rtrlib/rtr/rtr.h"
#include "ordered_dyn_array/ordered_dyn_array.h"

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

struct aspa_table;

/**
 * @brief A function pointer that is called if an record was added to the aspa_table or was removed from the aspa_table.
 *
 * @param aspa_table which was updated.
 * @param record aspa_record that was modified.
 * @param added True if the record was added, false if the record was removed.
 */
typedef void (*aspa_update_fp)(struct aspa_table *aspa_table,
							   const struct aspa_record record,
							   const struct rtr_socket *rtr_socket,
							   const bool added);


/**
 * @brief A function pointer that is called for each record in the ASPA table.
 * @param aspa_record The current record.
 * @param data forwarded data which the user has passed to aspa_table_for_each_record().
 */
typedef void (*aspa_for_each_fp)(const struct aspa_record *aspa_record, void *data);

struct aspa_cache {
	struct rtr_socket *rtr_socket;
	struct ordered_dyn_array *cached_array;
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
	struct aspa_cache *cache;
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
 */
void aspa_table_free(struct aspa_table *aspa_table);

/**
 * @brief Adds a aspa_record to a aspa_table.
 *
 * @param[in] aspa_table aspa_table to use.
 * @param[in] aspa_record aspa_record that will be added.
 * @return aspa_SUCCESS On success.
 
 * @return aspa_ERROR On error.
 * @return aspa_DUPLICATE_RECORD If an identical aspa_record already exists
 */
int aspa_table_add(struct aspa_table *aspa_table, struct aspa_record *aspa_record, struct rtr_socket *rtr_socket, bool replace);

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
 * @param[in] socket origin socket of the record
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
int aspa_table_search_by_customer_asn(struct aspa_table *aspa_table, uint32_t *customer_asn, struct aspa_record **result,
				 unsigned int *result_size);

/**
 * @brief Iterates over all records originating from the given sockets in the ASPA table.
 * @details For every record the function fp is called. The record and
 * the data pointer is passed to the fp.
 * @param[in] aspa_table The aspa
 * @param[in] fp A pointer to a callback function with the signature \c aspa_for_each_fp.
 * @param[in] sockets The sockets the records originate from.
 * @param[in] sockets_len The number of sockets in the sockets parameter.
 * @param[in] data This parameter is forwarded to the callback function.
 */
void aspa_table_for_each_record(struct aspa_table *aspa_table, aspa_for_each_fp fp, const struct rtr_socket **sockets, size_t sockets_len, void *data);

#endif
/** @} */
