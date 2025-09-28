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

// MARK: - Initialization & Deinitialization

enum rtr_aspa_status aspa_array_create(struct aspa_array **array_ptr)
{
	const size_t default_initial_size = 128;

	// allocation the chunk of memory of the provider as numbers
	struct rtr_aspa_record *data_field = rtr_malloc(sizeof(struct rtr_aspa_record) * default_initial_size);

	if (!data_field)
		return RTR_ASPA_ERROR;

	struct aspa_array *array = rtr_malloc(sizeof(struct aspa_array));

	if (!array) {
		rtr_free(data_field);
		return RTR_ASPA_ERROR;
	}

	// initializing member variables of the aspa record
	array->capacity = default_initial_size;
	array->size = 0;
	array->data = data_field;

	// returning the array
	*array_ptr = array;

	return RTR_ASPA_SUCCESS;
}

void aspa_array_free(struct aspa_array *array, bool free_provider_arrays)
{
	if (!array)
		return;

	if (array->data) {
		if (free_provider_arrays) {
			for (size_t i = 0; i < array->size; i++) {
				if (array->data[i].provider_asns) {
					rtr_free(array->data[i].provider_asns);
					array->data[i].provider_asns = NULL;
				}
			}
		}

		rtr_free(array->data);
	}

	rtr_free(array);
}

// MARK: - Manipulation

static enum rtr_aspa_status aspa_array_reallocate(struct aspa_array *array)
{
	// the factor by how much the capacity will increase: new_capacity = old_capacity + SIZE_INCREASE_OFFSET
	const size_t SIZE_INCREASE_OFFSET = 1000;

	// allocation the new chunk of memory
	struct rtr_aspa_record *tmp =
		rtr_realloc(array->data, sizeof(struct rtr_aspa_record) * array->capacity + SIZE_INCREASE_OFFSET);

	// malloc failed so returning an error
	if (!tmp)
		return RTR_ASPA_ERROR;

	array->data = tmp;
	array->capacity += SIZE_INCREASE_OFFSET;
	return RTR_ASPA_SUCCESS;
}

static enum rtr_aspa_status aspa_array_insert_internal(struct aspa_array *array, size_t index, struct rtr_aspa_record *record,
						   bool copy_providers)
{
	// check if this element will fit into the array
	if (array->size >= array->capacity) {
		// increasing the array's size so the new element fits
		if (aspa_array_reallocate(array) != RTR_ASPA_SUCCESS)
			return RTR_ASPA_ERROR;
	}

	uint32_t *provider_asns = NULL;

	if (record->provider_count > 0) {
		if (copy_providers) {
			size_t provider_size = record->provider_count * sizeof(uint32_t);

			provider_asns = rtr_malloc(provider_size);
			if (!provider_asns)
				return RTR_ASPA_ERROR;

			memcpy(provider_asns, record->provider_asns, provider_size);
		} else {
			provider_asns = record->provider_asns;
		}
	}

	// No need to move if last element
	if (index < array->size) {
		size_t trailing = (array->size - index) * sizeof(struct rtr_aspa_record);

		/*               trailing
		 *		     /-------------\
		 * #3 #8 #11 #24 #30 #36 #37
		 * #3 #8 #11  *  #24 #30 #36 #37
		 *            ^   ^
		 *		index   index + 1
		 */
		memmove(&array->data[index + 1], &array->data[index], trailing);
	}

	// append the record at the end
	array->data[index] = *record;
	array->data[index].provider_asns = provider_asns;
	array->size += 1;

	return RTR_ASPA_SUCCESS;
}

enum rtr_aspa_status aspa_array_insert(struct aspa_array *array, size_t index, struct rtr_aspa_record *record,
				   bool copy_providers)
{
	if (!array || index > array->size || !array->data || !record ||
	    ((record->provider_count > 0) && !record->provider_asns))
		return RTR_ASPA_ERROR;

	return aspa_array_insert_internal(array, index, record, copy_providers);
}

enum rtr_aspa_status aspa_array_append(struct aspa_array *array, struct rtr_aspa_record *record, bool copy_providers)
{
	if (!array || !array->data || !record || ((record->provider_count > 0) && !record->provider_asns))
		return RTR_ASPA_ERROR;

	return aspa_array_insert_internal(array, array->size, record, copy_providers);
}

enum rtr_aspa_status aspa_array_remove(struct aspa_array *array, size_t index, bool free_providers)
{
	if (!array || index >= array->size || array->size == 0)
		return RTR_ASPA_RECORD_NOT_FOUND;

	if (free_providers && array->data[index].provider_asns)
		rtr_free(array->data[index].provider_asns);

	// No need to move if last element
	if (index < array->size - 1) {
		size_t trailing = (array->size - index - 1) * sizeof(struct rtr_aspa_record);

		/*                     trailing
		 *				   /-------------\
		 *   #3 #8 #11  *  #24 #30 #36 #37
		 *   #3 #8 #11 #24 #30 #36 #37
		 *			    ^   ^
		 *		    index   index + 1
		 */
		memmove(&array->data[index], &array->data[index + 1], trailing);
	}

	array->size -= 1;

	// decreasing capacity if possible
	const size_t SIZE_DECREASE_OFFSET = 1000;
	if (array->size + SIZE_DECREASE_OFFSET < array->capacity) {
		struct rtr_aspa_record *tmp_data = rtr_realloc(array->data, array->size + SIZE_DECREASE_OFFSET);

		if (tmp_data == NULL) {
			/*
			 * Although reducing the array's capacity failed at this point,
			 * the element at the given index has been removed and thus the
			 * function's outcome can be considered successful.
			 */
			return RTR_ASPA_SUCCESS;
		}

		array->data = tmp_data;
		array->capacity = array->size + SIZE_DECREASE_OFFSET;
	}

	return RTR_ASPA_SUCCESS;
}

inline struct rtr_aspa_record *aspa_array_get_record(struct aspa_array *array, size_t index)
{
	if (!array || index >= array->size || array->size == 0 || !array->data)
		return NULL;

	return &array->data[index];
}

// MARK: - Retrieval

struct rtr_aspa_record *aspa_array_search(struct aspa_array *array, uint32_t customer_asn)
{
	// if the array is empty we return an error
	if (array->size == 0 || array->capacity == 0)
		return NULL;

	// left and right bound of our search space
	register size_t left = 0;
	register size_t right = array->size - 1;

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

enum rtr_aspa_status aspa_array_reserve(struct aspa_array *array, size_t size)
{
	// the given array is null
	if (array == NULL) {
		return RTR_ASPA_ERROR;
	}

	// we already have enough space allocated
	if (array->capacity >= size) {
		return RTR_ASPA_SUCCESS;
	}

	struct rtr_aspa_record *data = rtr_realloc(array->data, sizeof(struct rtr_aspa_record) * size);

	// realloc failed
	if (data == NULL) {
		return RTR_ASPA_ERROR;
	}

	// updating the array
	array->capacity = size;
	array->data = data;

	return RTR_ASPA_SUCCESS;
}
