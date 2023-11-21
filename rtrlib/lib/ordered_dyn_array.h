/*
* This file is part of RTRlib.
*
* This file is subject to the terms and conditions of the MIT license.
* See the file LICENSE in the top level directory for more details.
*
* Website: http://rtrlib.realmv6.org/
*/

#ifndef LRTR_VECTOR_UTILS_H
#define LRTR_VECTOR_UTILS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief ASPA Object
 * here the customer as number (CAS) authorizes a set of provider AS numbers
 */
struct aspas_record {
	uint32_t customer_asn;
	size_t provider_count;
	uint32_t *provider_asns;
};

/**
 * @brief Struct which is similar in function to std::vector from C++.
 * If the vector is running full a larger chunk of memory is allocated and the data is copied over.
 */
struct ordered_dyn_array {
	uint32_t size;
	size_t capacity;
	struct aspas_record *data;
};
/**
 * creates an vector object
 * @param[vector_pointer] the pointer to the newly created pointer will be written to *vector_pointer
 * @result Valid pointer to an ordered_dyn_array struct
 * @result Null On error.
 */
int ordered_dyn_array_create(struct ordered_dyn_array **vector_pointer);

/**
 * deletes the given vector
 * @param[vector] aspa_vector which will be deleted
 * @result 0 On success.
 * @result -1 On error.
 */
int ordered_dyn_array_delete(struct ordered_dyn_array *vector);

/**
 * reallocates the vector to increase its size
 * @param[vector] aspa_vector which should get an increase in size
 * @result 0 On success.
 * @result -1 On error.
 */
int ordered_dyn_array_reallocate(struct ordered_dyn_array *vector);

/**
 * will insert the element at the correct place in the list so the ascending order is preserved
 * This function assumes that the data field is large enough to fit one more element
 * This method is intended for internal use please use ordered_dyn_array_insert instead.
 * @param[vector] list of aspas records
 * @param[size] size of the aspas records list
 */
void ordered_dyn_array_private_insert(struct ordered_dyn_array *vector, struct aspas_record record);

/**
 * adds a new aspa record to the list
 * @param[vector] aspa_vector into which the value will be inserted
 * @param[value] uin32_t value which will be inserted
 * @result 0 On success.
 * @result -1 On error.
 */
int ordered_dyn_array_insert(struct ordered_dyn_array *vector, struct aspas_record record);

/**
 * deletes the element at the index
 * @param[vector] aspa_vector from where the element should be removed
 * @param[index] index of the element which should be removed
 * @result 0 On success.
 * @result -1 On error.
 */
int ordered_dyn_array_delete_at(struct ordered_dyn_array *vector, size_t index);

/**
 * returns the index in the vector for a given customer as number (CAS)
 * @param[vector] aspa_vector in which the algorithm will search
 * @param[custom_as] value for which will be searched
 * @result index of the element on success
 * @result -1 On error or not if the element coulnd't be located
 */
size_t ordered_dyn_array_search(struct ordered_dyn_array *vector, uint32_t customer_as);

#endif //LRTR_VECTOR_UTILS_H
