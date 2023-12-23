/*
* This file is part of RTRlib.
*
* This file is subject to the terms and conditions of the MIT license.
* See the file LICENSE in the top level directory for more details.
*
* Website: http://rtrlib.realmv6.org/
*/

#ifndef RTR_ASPA_DYN_ARRAY_H
#define RTR_ASPA_DYN_ARRAY_H

#include "../aspa.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Struct which is similar in function to std::vector from C++.
 * If the vector is running full a larger chunk of memory is allocated and the data is copied over.
 */
struct aspa_array {
	uint32_t size;
	size_t capacity;
	struct aspa_record *data;
};

/**
 * @brief Creates an vector object
 * @param[vector_pointer] the pointer to the newly created pointer will be written to *vector_pointer
 * @result Valid pointer to an aspa_array struct
 * @result Null On error.
 */
enum aspa_status aspa_array_create(struct aspa_array **array_ptr);

/**
 * @brief Deletes the given ASPA array
 * @param array ASPA array which will be deleted
 * @param free_provider_sets A boolean value determining whether each record's provider set should be released.
 * @result @c ASPA_SUCCESS On success.
 * @result @c ASPA_ERROR On error.
 */
enum aspa_status aspa_array_free(struct aspa_array *array, bool free_provider_sets);

/**
 * @brief Reallocates the vector to increase its size
 * @param[vector] aspa_vector which should get an increase in size
 * @result 0 On success.
 * @result -1 On error.
 */
enum aspa_status aspa_array_reallocate(struct aspa_array *array);

/**
 * @brief Will insert the element at the correct place in the list so the ascending order is preserved
 * This function assumes that the data field is large enough to fit one more element
 * This method is intended for internal use please use aspa_array_insert instead.
 */
enum aspa_status aspa_array_insert(struct aspa_array *array, size_t index, struct aspa_record *record);

enum aspa_status aspa_array_remove(struct aspa_array *array, size_t index);

enum aspa_status aspa_array_append(struct aspa_array *vector, struct aspa_record *record);

struct aspa_record *aspa_array_get_record(struct aspa_array *array, size_t index);
/**
 * @brief deletes the element from the vector
 * @param[vector] aspa_vector from which the element is to be removed
 * @param[entry] pointer to the element which is to be removed
 * @result 0 On success.
 * @result -1 On error.
 */
enum aspa_status aspa_array_free_entry(struct aspa_array *array, struct aspa_record *entry);

/**
 * @brief returns the index in the vector for a given customer as number (CAS)
 * @param[vector] aspa_vector in which the algorithm will search
 * @param[custom_as] value for which will be searched
 * @result pointer to the record on success
 * @result NULL On error or if the element coulnd't be located
 */
struct aspa_record *aspa_array_search(struct aspa_array *array, uint32_t customer_asn);

#endif
