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

enum aspa_status aspa_array_create(struct aspa_array **array_ptr)
{
	const size_t default_initial_size = 128;

	// allocation the chunk of memory of the provider as numbers
	struct aspa_record *data_field = lrtr_malloc(sizeof(struct aspa_record) * default_initial_size);

	if (!data_field)
		return ASPA_ERROR;

	struct aspa_array *array = lrtr_malloc(sizeof(struct aspa_array));

	if (!array) {
		lrtr_free(data_field);
		return ASPA_ERROR;
	}

	// initializing member variables of the aspa record
	array->capacity = default_initial_size;
	array->size = 0;
	array->data = data_field;

	// returning the array
	*array_ptr = array;

	return ASPA_SUCCESS;
}

void aspa_array_free(struct aspa_array *array, bool free_provider_arrays)
{
	if (!array)
		return;

	if (array->data) {
		if (free_provider_arrays) {
			for (size_t i = 0; i < array->size; i++) {
				if (array->data[i].provider_asns) {
					lrtr_free(array->data[i].provider_asns);
					array->data[i].provider_asns = NULL;
				}
			}
		}

		lrtr_free(array->data);
	}

	lrtr_free(array);
}

// MARK: - Manipulation

static enum aspa_status aspa_array_reallocate(struct aspa_array *array)
{
	// the factor by how much the capacity will increase: new_capacity = old_capacity * SIZE_INCREASE_EXPONENTIAL
	const size_t SIZE_INCREASE_EXPONENTIAL = 2;

	// allocation the new chunk of memory
	struct aspa_record *tmp =
		lrtr_realloc(array->data, sizeof(struct aspa_record) * array->capacity * SIZE_INCREASE_EXPONENTIAL);

	// malloc failed so returning an error
	if (!tmp)
		return ASPA_ERROR;

	array->data = tmp;
	array->capacity *= SIZE_INCREASE_EXPONENTIAL;
	return ASPA_SUCCESS;
}

static enum aspa_status aspa_array_insert_internal(struct aspa_array *array, size_t index, struct aspa_record *record,
						   bool copy_providers)
{
	// check if this element will fit into the array
	if (array->size >= array->capacity) {
		// increasing the array's size so the new element fits
		if (aspa_array_reallocate(array) != ASPA_SUCCESS)
			return ASPA_ERROR;
	}

	uint32_t *provider_asns = NULL;

	if (record->provider_count > 0) {
		if (copy_providers) {
			size_t provider_size = record->provider_count * sizeof(uint32_t);

			provider_asns = lrtr_malloc(provider_size);
			if (!provider_asns)
				return ASPA_ERROR;

			memcpy(provider_asns, record->provider_asns, provider_size);
		} else {
			provider_asns = record->provider_asns;
		}
	}

	// No need to move if last element
	if (index < array->size) {
		size_t trailing = (array->size - index) * sizeof(struct aspa_record);

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

	return ASPA_SUCCESS;
}

enum aspa_status aspa_array_insert(struct aspa_array *array, size_t index, struct aspa_record *record,
				   bool copy_providers)
{
	if (!array || index > array->size || !array->data || !record ||
	    ((record->provider_count > 0) && !record->provider_asns))
		return ASPA_ERROR;

	return aspa_array_insert_internal(array, index, record, copy_providers);
}

enum aspa_status aspa_array_append(struct aspa_array *array, struct aspa_record *record, bool copy_providers)
{
	if (!array || !array->data || !record || ((record->provider_count > 0) && !record->provider_asns))
		return ASPA_ERROR;

	return aspa_array_insert_internal(array, array->size, record, copy_providers);
}

enum aspa_status aspa_array_remove(struct aspa_array *array, size_t index, bool free_providers)
{
	if (!array || index >= array->size || array->size == 0)
		return ASPA_RECORD_NOT_FOUND;

	if (free_providers && array->data[index].provider_asns)
		lrtr_free(array->data[index].provider_asns);

	// No need to move if last element
	if (index < array->size - 1) {
		size_t trailing = (array->size - index - 1) * sizeof(struct aspa_record);

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
	return ASPA_SUCCESS;
}

inline struct aspa_record *aspa_array_get_record(struct aspa_array *array, size_t index)
{
	if (!array || index >= array->size || array->size == 0 || !array->data)
		return NULL;

	return &array->data[index];
}

// MARK: - Retrieval

struct aspa_record *aspa_array_search(struct aspa_array *array, uint32_t customer_asn)
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


enum aspa_status aspa_array_reserve(struct aspa_array *array, size_t size) {
	// the given array is null
	if (array == NULL) {
		return ASPA_ERROR;
	}

	// we already have enough space allocated
	if (array->capacity >= size) {
		return ASPA_SUCCESS;
	}

	struct aspa_record* data = malloc(sizeof(struct aspa_record) * size);

	// malloc failed
	if (data == NULL) {
		return ASPA_ERROR;
	}

	// coping the data from the old array into the new one
	if (memcpy(data, array->data, array->size * sizeof(struct aspa_record)) == NULL) {
		return ASPA_ERROR;
	}

	// updating the array
	free(array->data);
	array->capacity = size;
	array->data = data;

	return ASPA_SUCCESS;
}
