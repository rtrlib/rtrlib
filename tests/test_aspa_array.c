/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website; http://rtrlib.realmv6.org/
 */

#include "rtrlib/aspa/aspa_array/aspa_array.h"
#include "rtrlib/aspa/aspa_private.h"
#include "rtrlib/lib/alloc_utils_private.h"

#include <assert.h>

#define ASNS(...) ((uint32_t[]){__VA_ARGS__})

#define SEEDED_ASNS(seed) ASNS(seed, seed + 1, seed + 2)

// clang-format off

#define RECORD(cas, providers) \
	((struct aspa_record){.customer_asn = cas, \
				  .provider_count = (size_t)(sizeof(providers) / sizeof(uint32_t)), \
				  .provider_asns = sizeof(providers) == 0 ? NULL : providers})

// clang-format on

static void test_create_array(void)
{
	struct aspa_array *array;

	assert(aspa_array_create(&array) == ASPA_SUCCESS);
	assert(array->data);
	assert(array->size == 0);
	assert(array->capacity >= 128);

	aspa_array_free(array, true);
};

static void test_add_element(void)
{
	struct aspa_array *array;

	assert(aspa_array_create(&array) == ASPA_SUCCESS);

	struct aspa_record record = RECORD(42, SEEDED_ASNS(300));

	assert(aspa_array_insert(array, 0, &record, true) == 0);
	assert(array->data[0].customer_asn == 42);
	assert(array->data[0].provider_count == 3);
	assert(array->data[0].provider_asns[0] == 300);
	assert(array->data[0].provider_asns[1] == 300 + 1);
	assert(array->data[0].provider_asns[2] == 300 + 2);

	aspa_array_free(array, true);
}

static void test_insert(void)
{
	struct aspa_array *array;

	assert(aspa_array_create(&array) == ASPA_SUCCESS);
	array->capacity = 2;
	struct aspa_record *old_pointer = array->data;

	struct aspa_record record_4 = RECORD(4, SEEDED_ASNS(600));

	assert(aspa_array_insert(array, 0, &record_4, true) == ASPA_SUCCESS);

	struct aspa_record record_1 = RECORD(1, SEEDED_ASNS(300));

	assert(aspa_array_insert(array, 1, &record_1, true) == ASPA_SUCCESS);

	struct aspa_record record_2 = RECORD(2, SEEDED_ASNS(400));

	// Verify shifting works
	assert(aspa_array_insert(array, 1, &record_2, true) == ASPA_SUCCESS);

	struct aspa_record record_3 = RECORD(3, SEEDED_ASNS(500));

	assert(aspa_array_insert(array, 3, &record_3, true) == ASPA_SUCCESS);

	// New pointer because reallocated
	assert(old_pointer != array->data);
	assert(array->capacity >= 4);
	assert(array->size == 4);

	assert(array->data[0].customer_asn == 4);
	assert(array->data[1].customer_asn == 2);
	assert(array->data[2].customer_asn == 1);
	assert(array->data[3].customer_asn == 3);

	aspa_array_free(array, true);
}

static void test_append(void)
{
	struct aspa_array *array;

	assert(aspa_array_create(&array) == ASPA_SUCCESS);
	array->capacity = 2;
	struct aspa_record *old_pointer = array->data;

	struct aspa_record record_4 = RECORD(4, SEEDED_ASNS(600));

	assert(aspa_array_append(array, &record_4, true) == ASPA_SUCCESS);

	struct aspa_record record_2 = RECORD(2, SEEDED_ASNS(400));

	assert(aspa_array_append(array, &record_2, true) == ASPA_SUCCESS);

	struct aspa_record record_1 = RECORD(1, SEEDED_ASNS(300));

	assert(aspa_array_append(array, &record_1, true) == ASPA_SUCCESS);

	struct aspa_record record_3 = RECORD(3, SEEDED_ASNS(500));

	assert(aspa_array_append(array, &record_3, true) == ASPA_SUCCESS);

	// new pointer because reallocated
	assert(old_pointer != array->data);
	assert(array->capacity >= 4);
	assert(array->size == 4);

	assert(array->data[0].customer_asn == 4);
	assert(array->data[1].customer_asn == 2);
	assert(array->data[2].customer_asn == 1);
	assert(array->data[3].customer_asn == 3);

	aspa_array_free(array, true);
}

static void test_remove_element(void)
{
	struct aspa_array *array;

	assert(aspa_array_create(&array) == 0);

	struct aspa_record record_1 = RECORD(1, SEEDED_ASNS(300));

	assert(aspa_array_insert(array, 0, &record_1, true) == ASPA_SUCCESS);

	struct aspa_record record_2 = RECORD(2, SEEDED_ASNS(400));

	assert(aspa_array_insert(array, 1, &record_2, true) == ASPA_SUCCESS);

	struct aspa_record record_3 = RECORD(3, SEEDED_ASNS(500));

	assert(aspa_array_insert(array, 2, &record_3, true) == ASPA_SUCCESS);

	struct aspa_record record_4 = RECORD(4, SEEDED_ASNS(600));

	assert(aspa_array_insert(array, 3, &record_4, true) == ASPA_SUCCESS);

	assert(array->data[2].customer_asn == 3);

	assert(aspa_array_remove(array, 2, true) == ASPA_SUCCESS);
	assert(aspa_array_remove(array, 100, true) == ASPA_RECORD_NOT_FOUND);

	assert(array->size == 3);
	assert(array->data[0].customer_asn == 1);
	assert(array->data[1].customer_asn == 2);
	assert(array->data[2].customer_asn == 4);

	aspa_array_free(array, true);
}

static void test_find_element(void)
{
	struct aspa_array *array;

	assert(aspa_array_create(&array) == 0);

	struct aspa_record record_1 = RECORD(1, SEEDED_ASNS(300));

	assert(aspa_array_insert(array, 0, &record_1, true) == 0);

	struct aspa_record record_2 = RECORD(2, SEEDED_ASNS(400));

	assert(aspa_array_insert(array, 1, &record_2, true) == 0);

	struct aspa_record record_3 = RECORD(3, SEEDED_ASNS(500));

	assert(aspa_array_insert(array, 2, &record_3, true) == 0);

	struct aspa_record record_4 = RECORD(4, SEEDED_ASNS(600));

	assert(aspa_array_insert(array, 3, &record_4, true) == 0);

	struct aspa_record record_5 = RECORD(5, SEEDED_ASNS(700));

	assert(aspa_array_insert(array, 4, &record_5, true) == 0);

	assert(aspa_array_search(array, 1) == &array->data[0]);
	assert(aspa_array_search(array, 2) == &array->data[1]);
	assert(aspa_array_search(array, 3) == &array->data[2]);
	assert(aspa_array_search(array, 4) == &array->data[3]);
	assert(aspa_array_search(array, 5) == &array->data[4]);

	aspa_array_free(array, true);
}

int main(void)
{
	test_create_array();
	test_add_element();
	test_insert();
	test_append();
	test_remove_element();
	test_find_element();
}
