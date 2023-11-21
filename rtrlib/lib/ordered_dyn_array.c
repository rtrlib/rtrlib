/*
* This file is part of RTRlib.
*
* This file is subject to the terms and conditions of the MIT license.
* See the file LICENSE in the top level directory for more details.
*
* Website: http://rtrlib.realmv6.org/
*/

#include "alloc_utils_private.h"
#include "ordered_dyn_array.h"

int ordered_dyn_array_create(struct ordered_dyn_array **vector_pointer)
{
	const size_t DEFAULT_INITIAL_SIZE = 128;

	// allocation the chunk of memory of the provider as numbers
	struct aspas_record *data_field =
		(struct aspas_record *)lrtr_malloc(sizeof(struct aspas_record) * DEFAULT_INITIAL_SIZE);

	// malloc failed so returning an error
	if (data_field == NULL) {
		return -1;
	}

	// allocating the aspa_record itself
	struct ordered_dyn_array *vector = (struct ordered_dyn_array *)lrtr_malloc(sizeof(struct ordered_dyn_array));

	// malloc for aspa_record failed hence we return an error
	if (vector == NULL) {
		return -1;
	}

	// initializing member variables of the aspa record
	vector->capacity = DEFAULT_INITIAL_SIZE;
	vector->size = 0;
	vector->data = data_field;

	// returning the vector
	*vector_pointer = vector;

	return 0;
}

int ordered_dyn_array_delete(struct ordered_dyn_array *vector)
{
	// if the vector is null just return
	if (vector == NULL) {
		return -1;
	}

	if (vector->data != NULL) {
		// freeing the data
		free(vector->data);
	}

	// freeing the object itself
	free(vector);

	return 0;
}

int ordered_dyn_array_reallocate(struct ordered_dyn_array *vector)
{
	// the factor by how much the capacity will increase: new_capacity = old_capacity * SIZE_INCREASE_EXPONENTIAL
	const size_t SIZE_INCREASE_EXPONENTIAL = 2;

	// allocation the new chunk of memory
	struct aspas_record *new_data_field = (struct aspas_record *)lrtr_malloc(
		sizeof(struct aspas_record) * vector->capacity * SIZE_INCREASE_EXPONENTIAL);

	// malloc failed so returning an error
	if (new_data_field == NULL) {
		return -1;
	}

	// copying the data from the old location to the new one
	memcpy(new_data_field, vector->data, vector->capacity * sizeof(struct aspas_record));

	// deleting the old vector
	free(vector->data);

	// assigning the new array to the vector and incrementing the capacity
	vector->data = new_data_field;
	vector->capacity *= SIZE_INCREASE_EXPONENTIAL;

	return 0;
}

void ordered_dyn_array_private_insert(struct ordered_dyn_array *vector, struct aspas_record record)
{
	// iterator running from the back of the array to the front
	size_t j = vector->size;

	// we are moving from the back of the array to the front while looking
	// for the place where to insert our new record
	while (j > 0 && vector->data[j - 1].customer_asn > record.customer_asn) {
		// moving data to the right
		vector->data[j] = vector->data[j - 1];
		j -= 1;
	}

	// inserting the record
	vector->data[j] = record;
}

int ordered_dyn_array_insert(struct ordered_dyn_array *vector, struct aspas_record record)
{
	// check if this element will fit into the vector
	if (vector->size + 1 > vector->capacity) {
		// increasing the vectors size so the new element fits
		if (ordered_dyn_array_reallocate(vector) == -1) {
			return -1;
		}
	}

	// insert the element at the correct place
	ordered_dyn_array_private_insert(vector, record);
	vector->size += 1;

	return 0;
}

size_t ordered_dyn_array_search(struct ordered_dyn_array *vector, uint32_t customer_as)
{
	// if the vector is empty we return an error
	if (vector->size == 0 || vector->capacity == 0) {
		return -1;
	}

	// left and right bound of our search space
	size_t left = 0;
	size_t right = vector->size;

	// we stop if right and left crossed
	while (left <= right) {
		// current center
		size_t center = left + ((right - left) / 2);
		uint32_t center_value = vector->data[center].customer_asn;

		if (center_value == customer_as) {
			// success found the value
			return center;
		} else if (center_value > customer_as) {
			// value should be on the left side
			right = center - 1;
		} else {
			// value should be on the right side
			left = center + 1;
		}
	}

	// element not found
	return -1;
}

int ordered_dyn_array_delete_at(struct ordered_dyn_array *vector, size_t index)
{
	if (vector->size <= index || vector->size == 0) {
		return -1;
	}

	// number of element that need to be moved left
	size_t number_of_elements = vector->size - index - 1;

	// if 1 or more elements needs to be copied
	if (number_of_elements > 0) {
		memcpy(vector->data + index, vector->data + index + 1,
		       number_of_elements * sizeof(struct aspas_record));
	}

	// decrementing the size by one
	vector->size -= 1;

	// we dont need to sort here
	return 0;
}
