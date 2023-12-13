/*
* This file is part of RTRlib.
*
* This file is subject to the terms and conditions of the MIT license.
* See the file LICENSE in the top level directory for more details.
*
* Website: http://rtrlib.realmv6.org/
*/

#ifndef RTR_ASPA_PDU_ARRAY_H
#define RTR_ASPA_PDU_ARRAY_H

#include "../aspa.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct aspa_record_with_pdu {
	uint32_t customer_asn;
	size_t provider_count;
	uint32_t *provider_asns;
	struct pdu_aspa *pdu;
};

/**
 * @brief Struct which is similar in function to std::vector from C++.
 * If the vector is running full a larger chunk of memory is allocated and the data is copied over.
 */
struct aspa_pdu_array {
	uint32_t size;
	size_t capacity;
	struct aspa_record_with_pdu *data;
};

/**
 * @brief Creates an vector object
 * @param[vector_pointer] the pointer to the newly created pointer will be written to *vector_pointer
 * @result Valid pointer to an aspa_pdu_array struct
 * @result Null On error.
 */
int aspa_pdu_array_create(struct aspa_pdu_array **vector_pointer);

/**
 * @brief Deletes the given vector except its aspa_record_with_pdu's provider lists
 * @param[vector] aspa_vector which will be deleted
 * @result 0 On success.
 * @result -1 On error.
 */
int aspa_pdu_array_free(struct aspa_pdu_array *vector);

/**
 * @brief Reallocates the vector to increase its size
 * @param[vector] aspa_vector which should get an increase in size
 * @result 0 On success.
 * @result -1 On error.
 */
int aspa_pdu_array_reallocate(struct aspa_pdu_array *vector);

/**
 * @brief adds a free slot for an aspa record with pdu to the list
 * @param[vector] aspa_vector at whose end the slot is to be made
 * @return pointer to slot 
 */
struct aspa_record_with_pdu *aspa_pdu_array_append_slot(struct aspa_pdu_array *vector);

#endif
