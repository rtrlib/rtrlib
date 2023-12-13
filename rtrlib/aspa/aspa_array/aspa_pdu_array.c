/*
* This file is part of RTRlib.
*
* This file is subject to the terms and conditions of the MIT license.
* See the file LICENSE in the top level directory for more details.
*
* Website: http://rtrlib.realmv6.org/
*/

#include "aspa_pdu_array.h"

#include "rtrlib/aspa/aspa_private.h"
#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/rtr/rtr.h"

int aspa_pdu_array_create(struct aspa_pdu_array **vector_pointer)
{
	const size_t default_initial_size = 128;

	// allocation the chunk of memory of the provider as numbers
	struct aspa_record_with_pdu *data_field =
		(struct aspa_record_with_pdu *)lrtr_malloc(sizeof(struct aspa_record_with_pdu) * default_initial_size);

	// malloc failed so returning an error
	if (!data_field) {
		return -1;
	}

	// allocating the aspa_record_with_pdu itself
	struct aspa_pdu_array *vector = (struct aspa_pdu_array *)lrtr_malloc(sizeof(struct aspa_pdu_array));

	// malloc for aspa_record_with_pdu failed hence we return an error
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

int aspa_pdu_array_free(struct aspa_pdu_array *vector)
{
	// if the vector is null just return
	if (vector == NULL) {
		return -1;
	}

	for (size_t i = 0; i < vector->size; i++) {
		if (vector->data[i].pdu != NULL)
			// we're not deleting provider_asns here because they
			// may still be in use in an aspa_store's aspa_array
			lrtr_free(vector->data[i].pdu);
	}

	if (vector->data != NULL) {
		// freeing the data
		lrtr_free(vector->data);
	}

	// freeing the object itself
	lrtr_free(vector);

	return 0;
}

int aspa_pdu_array_reallocate(struct aspa_pdu_array *vector)
{
	// the factor by how much the capacity will increase: new_capacity = old_capacity * SIZE_INCREASE_EXPONENTIAL
	const size_t SIZE_INCREASE_EXPONENTIAL = 2;

	// allocation the new chunk of memory
	struct aspa_record_with_pdu *new_data_field = (struct aspa_record_with_pdu *)lrtr_malloc(
		sizeof(struct aspa_record_with_pdu) * vector->capacity * SIZE_INCREASE_EXPONENTIAL);

	// malloc failed so returning an error
	if (new_data_field == NULL) {
		return -1;
	}

	// copying the data from the old location to the new one
	memcpy(new_data_field, vector->data, vector->capacity * sizeof(struct aspa_record_with_pdu));

	// deleting the old vector
	lrtr_free(vector->data);

	// assigning the new array to the vector and incrementing the capacity
	vector->data = new_data_field;
	vector->capacity *= SIZE_INCREASE_EXPONENTIAL;

	return 0;
}

struct aspa_record_with_pdu *aspa_pdu_array_append_slot(struct aspa_pdu_array *vector)
{
	// check if this element will fit into the vector
	if (vector->size + 1 > vector->capacity) {
		// increasing the vectors size so the new element fits
		if (aspa_pdu_array_reallocate(vector) < 0) {
			return NULL;
		}
	}

	return &vector->data[vector->size++];
}
