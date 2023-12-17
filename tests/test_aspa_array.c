/*
* This file is part of RTRlib.
*
* This file is subject to the terms and conditions of the MIT license.
* See the file LICENSE in the top level directory for more details.
*
* Website; http://rtrlib.realmv6.org/
*/

#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/aspa/aspa_private.h"
#include "rtrlib/aspa/aspa_array/aspa_array.h"

#include <assert.h>

static void test_create_array()
{
	struct aspa_array *vector;
	assert(aspa_array_create(&vector) == ASPA_SUCCESS);
	assert(vector->data != NULL);
	assert(vector->size == 0);
	assert(vector->capacity >= 128);
};

static void generate_fake_aspa_record(uint32_t cas, uint32_t random_number, struct aspa_record **record)
{
	struct aspa_record *new_record = (struct aspa_record *)lrtr_malloc(sizeof(struct aspa_record));
	new_record->customer_asn = cas;
	uint32_t *provider_as_numbers = (uint32_t *)lrtr_malloc(sizeof(*new_record->provider_asns) * 3);

	for (size_t i = 0; i < 3; i++) {
		provider_as_numbers[i] = random_number + i;
	}

	new_record->provider_count = 3;
	new_record->provider_asns = provider_as_numbers;

	*record = new_record;
}

static void test_add_element()
{
	struct aspa_array *vector;
	assert(aspa_array_create(&vector) == ASPA_SUCCESS);

	struct aspa_record *record;
	generate_fake_aspa_record(42, 300, &record);
	assert(aspa_array_insert(vector, 0, record) == 0);

	assert(vector->data[0].customer_asn == 42);
	assert(vector->data[0].provider_count == 3);
	assert(vector->data[0].provider_asns[0] == 300);
	assert(vector->data[0].provider_asns[1] == 300 + 1);
	assert(vector->data[0].provider_asns[2] == 300 + 2);

	assert(aspa_array_free(vector, true) == ASPA_SUCCESS);
}

static void test_relocate()
{
	struct aspa_array *vector;
	assert(aspa_array_create(&vector) == ASPA_SUCCESS);
	vector->capacity = 2;
	struct aspa_record *old_pointer = vector->data;

	struct aspa_record *record_4;
	generate_fake_aspa_record(4, 600, &record_4);
	assert(aspa_array_insert(vector, 0, record_4) == ASPA_SUCCESS);

	struct aspa_record *record_2;
	generate_fake_aspa_record(2, 400, &record_2);
	assert(aspa_array_insert(vector, 1, record_2) == ASPA_SUCCESS);

	struct aspa_record *record_1;
	generate_fake_aspa_record(1, 300, &record_1);
	assert(aspa_array_insert(vector, 2, record_1) == ASPA_SUCCESS);

	struct aspa_record *record_3;
	generate_fake_aspa_record(3, 500, &record_3);
	assert(aspa_array_insert(vector, 3, record_3) == ASPA_SUCCESS);

	assert(old_pointer != vector->data); // new pointer because relocated
	assert(vector->capacity >= 4);
	assert(vector->size == 4);

	// checks for the proper ordering
	assert(vector->data[0].customer_asn == 4);
	assert(vector->data[1].customer_asn == 2);
	assert(vector->data[2].customer_asn == 1);
	assert(vector->data[3].customer_asn == 3);

	assert(aspa_array_free(vector, true) == ASPA_SUCCESS);
}

static void test_remove_element()
{
	struct aspa_array *vector;
	assert(aspa_array_create(&vector) == 0);

	struct aspa_record *record_1;
	generate_fake_aspa_record(1, 300, &record_1);
	assert(aspa_array_insert(vector, 0, record_1) == ASPA_SUCCESS);

	struct aspa_record *record_2;
	generate_fake_aspa_record(2, 400, &record_2);
	assert(aspa_array_insert(vector, 1, record_2) == ASPA_SUCCESS);

	struct aspa_record *record_3;
	generate_fake_aspa_record(3, 500, &record_3);
	assert(aspa_array_insert(vector, 2, record_3) == ASPA_SUCCESS);

	struct aspa_record *record_4;
	generate_fake_aspa_record(4, 600, &record_4);
	assert(aspa_array_insert(vector, 3, record_4) == ASPA_SUCCESS);

	assert(vector->data[2].customer_asn == 3);

	assert(aspa_array_free_entry(vector, &vector->data[2]) == 0);
	assert(aspa_array_free_entry(vector, &vector->data[100]) == -1);

	assert(vector->size == 3);
	assert(vector->data[0].customer_asn == 1);
	assert(vector->data[1].customer_asn == 2);
	assert(vector->data[2].customer_asn == 4);

	assert(aspa_array_free(vector, true) == 0);
}

static void test_find_element()
{
	struct aspa_array *vector;
	assert(aspa_array_create(&vector) == 0);

	struct aspa_record *record_1;
	generate_fake_aspa_record(1, 300, &record_1);
	assert(aspa_array_insert(vector, 0, record_1) == 0);

	struct aspa_record *record_2;
	generate_fake_aspa_record(2, 400, &record_2);
	assert(aspa_array_insert(vector, 1, record_2) == 0);

	struct aspa_record *record_3;
	generate_fake_aspa_record(3, 500, &record_3);
	assert(aspa_array_insert(vector, 2, record_3) == 0);

	struct aspa_record *record_4;
	generate_fake_aspa_record(4, 600, &record_4);
	assert(aspa_array_insert(vector, 3, record_4) == 0);

	struct aspa_record *record_5;
	generate_fake_aspa_record(5, 700, &record_5);
	assert(aspa_array_insert(vector, 4, record_5) == 0);

	assert(aspa_array_search(vector, 1) == &vector->data[0]);
	assert(aspa_array_search(vector, 2) == &vector->data[1]);
	assert(aspa_array_search(vector, 3) == &vector->data[2]);
	assert(aspa_array_search(vector, 4) == &vector->data[3]);
	assert(aspa_array_search(vector, 5) == &vector->data[4]);

	assert(aspa_array_free(vector, true) == ASPA_SUCCESS);
}

int main()
{
	test_create_array();
	test_add_element();
	test_relocate();
	test_remove_element();
	test_find_element();
}
