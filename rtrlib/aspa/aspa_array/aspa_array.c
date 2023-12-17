/*
* This file is part of RTRlib.
*
* This file is subject to the terms and conditions of the MIT license.
* See the file LICENSE in the top level directory for more details.
*
* Website: http://rtrlib.realmv6.org/
*/

#include "aspa_array.h"

#include "rtrlib/aspa/aspa_private.h"
#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/rtr/rtr.h"

enum aspa_status aspa_array_create(struct aspa_array **array_ptr)
{
	const size_t default_initial_size = 128;

	// allocation the chunk of memory of the provider as numbers
	struct aspa_record *data_field =
		(struct aspa_record *)lrtr_malloc(sizeof(struct aspa_record) * default_initial_size);

	// malloc failed so returning an error
	if (!data_field) {
		return ASPA_ERROR;
	}

	// allocating the aspa_record itself
	struct aspa_array *array = (struct aspa_array *)lrtr_malloc(sizeof(struct aspa_array));

	// malloc for aspa_record failed hence we return an error
	if (!array) {
		return ASPA_ERROR;
	}

	// initializing member variables of the aspa record
	array->capacity = default_initial_size;
	array->size = 0;
	array->data = data_field;

	// returning the vector
	*array_ptr = array;

	return ASPA_SUCCESS;
}

enum aspa_status aspa_array_free(struct aspa_array *array, bool free_provider_sets)
{
	// if the vector is null just return
	if (!array) {
		return ASPA_ERROR;
	}

	if (array->data) {
		// freeing the data
		lrtr_free(array->data);

		if (free_provider_sets) {
			for (size_t i = 0; i < array->size; i++) {
				if (array->data[i].provider_asns) {
					lrtr_free(array->data[i].provider_asns);
					array->data[i].provider_asns = NULL;
				}
			}
		}
	}

	// freeing the array itself
	lrtr_free(array);

	return ASPA_SUCCESS;
}

enum aspa_status aspa_array_reallocate(struct aspa_array *array)
{
	// the factor by how much the capacity will increase: new_capacity = old_capacity * SIZE_INCREASE_EXPONENTIAL
	const size_t SIZE_INCREASE_EXPONENTIAL = 2;

	// allocation the new chunk of memory
	struct aspa_record *new_data_field = (struct aspa_record *)lrtr_malloc(
		sizeof(struct aspa_record) * array->capacity * SIZE_INCREASE_EXPONENTIAL);

	// malloc failed so returning an error
	if (new_data_field == NULL) {
		return ASPA_ERROR;
	}

	// copying the data from the old location to the new one
	memcpy(new_data_field, array->data, array->capacity * sizeof(struct aspa_record));

	// deleting the old vector
	lrtr_free(array->data);

	// assigning the new array to the vector and incrementing the capacity
	array->data = new_data_field;
	array->capacity *= SIZE_INCREASE_EXPONENTIAL;

	return ASPA_SUCCESS;
}

enum aspa_status aspa_array_insert(struct aspa_array *array, size_t index, struct aspa_record *record)
{
	if (index > array->size)
		return ASPA_ERROR;

	// check if this element will fit into the vector
	if (array->size >= array->capacity) {
		// increasing the vectors size so the new element fits
		if (aspa_array_reallocate(array) != ASPA_SUCCESS) {
			return ASPA_ERROR;
		}
	}

	// No need to move if last element
	if (index < array->size) {
		size_t trailing = (array->size - index) * sizeof(struct aspa_record);

		/*             trailing
				   /-------------\
		 #3 #8 #11 #24 #30 #36 #37
		 #3 #8 #11  *  #24 #30 #36 #37
		            ^   ^
				index   index + 1
		 */
		memmove(&array->data[index + 1], &array->data[index], trailing);
	}

	array->size += 1;
	array->data[index] = *record;
	return ASPA_SUCCESS;
}

enum aspa_status aspa_array_remove(struct aspa_array *array, size_t index)
{
	if (index >= array->size || array->size == 0)
		return ASPA_ERROR;

	// No need to move if last element
	if (index < array->size - 1) {
		size_t trailing = (array->size - index - 1) * sizeof(struct aspa_record);

		/*                   trailing
						 /-------------\
		   #3 #8 #11  *  #24 #30 #36 #37
		   #3 #8 #11 #24 #30 #36 #37
					  ^   ^
				  index   index + 1
		 */
		memmove(&array->data[index], &array->data[index + 1], trailing);
	}

	array->size -= 1;
	return ASPA_SUCCESS;
}

inline struct aspa_record *aspa_array_get_record(struct aspa_array *array, size_t index)
{
	if (!array || index >= array->size || array->size == 0 || !array->data)
		return NULL;

	return &array->data[index];
}

enum aspa_status aspa_array_append(struct aspa_array *array, struct aspa_record *record)
{
	// check if this element will fit into the vector
	if (array->size >= array->capacity) {
		// increasing the vectors size so the new element fits
		if (aspa_array_reallocate(array) != ASPA_SUCCESS) {
			return ASPA_ERROR;
		}
	}

	// append the record at the end
	array->data[array->size] = *record;
	array->size += 1;

	return ASPA_SUCCESS;
}

struct aspa_record *aspa_array_search(struct aspa_array *array, uint32_t customer_asn)
{
	// if the vector is empty we return an error
	if (array->size == 0 || array->capacity == 0) {
		return NULL;
	}

	// left and right bound of our search space
	register size_t left = 0;
	register size_t right = array->size;

	// we stop if right and left crossed
	while (left <= right) {
		// current center
		size_t center = (left + right) >> 1;
		uint32_t center_value = array->data[center].customer_asn;

		// success found the value
		if (center_value == customer_asn) {
			return &array->data[center];

		// value should be on the right side
		} else if (center_value < customer_asn) {
			left = center + 1;

		// value should be on the left side
		} else if (center == 0) {
			// value cannot be left of index 0
			return NULL;
		} else {
			right = center - 1;
		}
	}

	// element not found
	return NULL;
}

enum aspa_status aspa_array_free_entry(struct aspa_array *array, struct aspa_record *entry)
{
	if (array->size == 0 || entry < array->data || entry >= array->data + array->size) {
		return -1;
	}

	// number of elements that need to be moved left
	size_t index = (size_t)(entry - array->data);
	size_t number_of_elements = array->size - index - 1;

	lrtr_free(entry->provider_asns);

	// if 1 or more elements needs to be copied
	if (number_of_elements > 0) {
		memmove(entry, entry + 1, number_of_elements * sizeof(struct aspa_record));
	}

	// decrementing the size by one
	array->size -= 1;

	// we dont need to sort here
	return ASPA_SUCCESS;
}
