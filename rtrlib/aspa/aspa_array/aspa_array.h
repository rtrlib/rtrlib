/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef RTR_ASPA_ARRAY_H
#define RTR_ASPA_ARRAY_H

#include "../aspa.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Struct which is similar in function to std::vector from C++.
 * If the vector is running full, a larger chunk of memory is reallocated.
 *
 * This structure stores ASPA records in a contiguous chunk of memory,
 * sorted in ascending order by their customer ASN.
 */
struct aspa_array {
	uint32_t size;
	size_t capacity;
	struct aspa_record *data;
};

// MARK: - Initialization & Deinitialization

/**
 * @brief Creates an vector object
 * @param[in,out] array_ptr Pointer to a variable that will hold a reference to the newly created array.
 * @return @c ASPA_SUCCESS if the operation succeeds, @c ASPA_ERROR if it fails.
 */
enum aspa_status aspa_array_create(struct aspa_array **array_ptr);

/**
 * @brief Deletes the given ASPA array.
 * @param array ASPA array which will be deleted
 * @param free_provider_arrays A boolean value determining whether each record's provider array should be deallocated.
 */
void aspa_array_free(struct aspa_array *array, bool free_provider_arrays);

// MARK: - Manipulation

/**
 * @brief Inserts a given ASPA record into the array, preserving its order.
 *
 * @param array The ASPA array that will hold the new record.
 * @param index The index at which the new record will be stored.
 * @param record The new record.
 * @param copy_providers A boolean value indicating whether the array should copy the record's
 * providers before inserting the record.
 * @return @c ASPA_SUCCESS if the operation succeeds, @c ASPA_ERROR if it fails.
 */
enum aspa_status aspa_array_insert(struct aspa_array *array, size_t index, struct aspa_record *record,
				   bool copy_providers);

/**
 * @brief Appends a given ASPA record to the array.
 *
 * @param array The ASPA array that will hold the new record.
 * @param record The record that will be appended to the array.
 * @param copy_providers A boolean value indicating whether the array should copy the record's
 * providers before appending the record.
 * @return @c ASPA_SUCCESS if the operation succeeds, @c ASPA_ERROR if it fails.
 */
enum aspa_status aspa_array_append(struct aspa_array *array, struct aspa_record *record, bool copy_providers);

/**
 * @brief Removes the record at the given index from the array.
 *
 * @param array The array to remove the record from.
 * @param index The record's index.
 * @param free_providers A boolean value determining whether to free the existing record's provider array.
 * @return @c ASPA_SUCCESS if the operation succeeds, @c ASPA_RECORD_NOT_FOUND if the record's index doesn't exist,
 * @c ASPA_ERROR otherwise.
 */
enum aspa_status aspa_array_remove(struct aspa_array *array, size_t index, bool free_providers);

// MARK: - Retrieval

/**
 * @brief Returns a reference to the record at the given index.
 *
 * @param array ASPA array
 * @param index The index in the ASPA array.
 * @return A reference to the `aspa_record` if found, @c NULL otherwise.
 */
struct aspa_record *aspa_array_get_record(struct aspa_array *array, size_t index);

/**
 * @brief Searches the given ASPA array for a record matching its customer ASN.
 *
 * @param array The array to search.
 * @param customer_asn Customer ASN
 * @return A reference to the `aspa_record` if found, @c NULL otherwise.
 */
struct aspa_record *aspa_array_search(struct aspa_array *array, uint32_t customer_asn);

/**
 * @brief Reserves some space in the array
 *
 * @param array The array to remove the record from.
 * @param size the number of object that should definetly fit into the array
 * @return @c ASPA_SUCCESS if the operation succeeds @c ASPA_ERROR otherwise.
 */
enum aspa_status aspa_array_reserve(struct aspa_array *array, size_t size);

#endif // RTR_ASPA_ARRAY_H