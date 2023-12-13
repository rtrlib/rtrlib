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

int aspa_array_create(struct aspa_array **vector_pointer)
{
	const size_t default_initial_size = 128;

	// allocation the chunk of memory of the provider as numbers
	struct aspa_record *data_field =
		(struct aspa_record *)lrtr_malloc(sizeof(struct aspa_record) * default_initial_size);

	// malloc failed so returning an error
	if (!data_field) {
		return -1;
	}

	// allocating the aspa_record itself
	struct aspa_array *vector = (struct aspa_array *)lrtr_malloc(sizeof(struct aspa_array));

	// malloc for aspa_record failed hence we return an error
	if (!vector) {
		return -1;
	}

	// initializing member variables of the aspa record
	vector->capacity = default_initial_size;
	vector->size = 0;
	vector->data = data_field;

	// returning the vector
	*vector_pointer = vector;

	return 0;
}

int aspa_array_free(struct aspa_array *vector)
{
	// if the vector is null just return
	if (vector == NULL) {
		return -1;
	}

	if (vector->data != NULL) {
		// freeing the data
		lrtr_free(vector->data);
	}

	// freeing the object itself
	lrtr_free(vector);

	return 0;
}

int aspa_array_reallocate(struct aspa_array *vector)
{
	// the factor by how much the capacity will increase: new_capacity = old_capacity * SIZE_INCREASE_EXPONENTIAL
	const size_t SIZE_INCREASE_EXPONENTIAL = 2;

	// allocation the new chunk of memory
	struct aspa_record *new_data_field = (struct aspa_record *)lrtr_malloc(
		sizeof(struct aspa_record) * vector->capacity * SIZE_INCREASE_EXPONENTIAL);

	// malloc failed so returning an error
	if (new_data_field == NULL) {
		return -1;
	}

	// copying the data from the old location to the new one
	memcpy(new_data_field, vector->data, vector->capacity * sizeof(struct aspa_record));

	// deleting the old vector
	lrtr_free(vector->data);

	// assigning the new array to the vector and incrementing the capacity
	vector->data = new_data_field;
	vector->capacity *= SIZE_INCREASE_EXPONENTIAL;

	return 0;
}

void aspa_array_private_insert(struct aspa_array *vector, struct aspa_record *record)
{
	// TODO: Handle replacements
	// iterator running from the back of the array to the front
	size_t j = vector->size;

	// we are moving from the back of the array to the front while looking
	// for the place where to insert our new record
	while (j > 0 && vector->data[j - 1].customer_asn > record->customer_asn) {
		// moving data to the right
		vector->data[j] = vector->data[j - 1];
		j -= 1;
	}

	vector->data[j] = *record;
}

int aspa_array_insert(struct aspa_array *vector, struct aspa_record *record)
{
	// check if this element will fit into the vector
	if (vector->size + 1 > vector->capacity) {
		// increasing the vectors size so the new element fits
		if (aspa_array_reallocate(vector) < 0) {
			return -1;
		}
	}

	// insert the element at the correct place
	aspa_array_private_insert(vector, record);
	vector->size += 1;

	return 0;
}

struct aspa_record *aspa_array_search(struct aspa_array *vector, uint32_t customer_asn)
{
	// if the vector is empty we return an error
	if (vector->size == 0 || vector->capacity == 0) {
		return -1;
	}

	// left and right bound of our search space
	register size_t left = 0;
	register size_t right = vector->size;

	// we stop if right and left crossed
	while (left <= right) {
		// current center
		size_t center = (left + right) >> 1;
		uint32_t center_value = vector->data[center].customer_asn;

		if (center_value == customer_asn) {
			// success found the value
			return &vector->data[center];
		} else if (center_value > customer_asn) {
			// value should be on the left side
			right = center - 1;
		} else {
			// value should be on the right side
			left = center + 1;
		}
	}

	// element not found
	return NULL;
}

int aspa_array_free_entry(struct aspa_array *vector, struct aspa_record *entry)
{
	if (vector->size == 0 || entry < vector->data ||
			entry >= vector->data + vector->size) {
		return -1;
	}

	// number of elements that need to be moved left
	size_t index = (size_t)(entry - vector->data);
	size_t number_of_elements = vector->size - index - 1;

	lrtr_free(entry->provider_asns);

	// if 1 or more elements needs to be copied
	if (number_of_elements > 0) {
		memmove(entry, entry + 1,
			number_of_elements * sizeof(struct aspa_record));
	}

	// decrementing the size by one
	vector->size -= 1;

	// we dont need to sort here
	return 0;
}
