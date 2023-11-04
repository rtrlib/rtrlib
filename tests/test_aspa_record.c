/*
* This file is part of RTRlib.
*
* This file is subject to the terms and conditions of the MIT license.
* See the file LICENSE in the top level directory for more details.
*
* Website; http://rtrlib.realmv6.org/
*/

#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/aspa/aspa.h"
#include "rtrlib/aspa/aspa_array/aspa_array.h"

#include <assert.h>

static void test_aspa_record_merge() {
	struct aspa_record record_1;
	struct aspa_record record_2;

	const uint32_t provider_asns_1[] = {1, 4, 5, 6, 7, 10, 12};
	const uint32_t provider_asns_2[] = {4, 5, 6, 16};
	const uint32_t expected_result[] = {1, 4, 5, 6,7, 10, 12, 16};

	record_1.customer_asn = 3;
	record_2.customer_asn = 3;

	record_1.provider_asns = lrtr_malloc(7 * sizeof(uint32_t));
	record_1.provider_count = 7;
	memcpy(record_1.provider_asns, provider_asns_1, 7 * sizeof(uint32_t));


	record_2.provider_asns = lrtr_malloc(4 * sizeof(uint32_t));
	record_2.provider_count = 4;
	memcpy(record_2.provider_asns, provider_asns_2, 4 * sizeof(uint32_t));

	assert(merge_aspa_records(&record_1, &record_2) == 0);

	assert(record_2.customer_asn == 3);
	assert(record_2.provider_count == 8);
	assert(memcmp(record_2.provider_asns, expected_result, record_2.provider_count) == 0);
};

int main() {
	test_aspa_record_merge();
}


