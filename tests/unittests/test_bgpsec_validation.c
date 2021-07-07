/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include <assert.h>

#include "rtrlib_unittests.h"
#include "rtrlib/bgpsec/bgpsec_utils.c"
#include "rtrlib/bgpsec/bgpsec_private.h"
#include "test_bgpsec_validation.h"

struct rtr_bgpsec *setup_bgpsec(void)
{
	struct rtr_bgpsec *bgpsec = NULL;
	uint8_t alg = 1;
	uint8_t safi = 1;
	uint16_t afi = 1;
	uint32_t my_as = 65537;
	uint32_t target_as = 65538;
	struct rtr_bgpsec_nlri pfx;
	uint32_t ip = 0xC0000200;

	pfx.prefix_len = 24;
	pfx.prefix.ver = LRTR_IPV4;
	pfx.prefix.u.addr4.addr = ntohl(ip);

	bgpsec = rtr_bgpsec_new(alg, safi, afi, my_as, target_as, pfx);
	bgpsec->path = lrtr_malloc(sizeof(struct rtr_secure_path_seg));
	bgpsec->sigs = lrtr_malloc(sizeof(struct rtr_signature_seg));
	return bgpsec;
}

int __wrap_check_router_keys(const struct rtr_signature_seg *sig_segs,
			     struct spki_table *table)
{
	UNUSED(sig_segs);
	UNUSED(table);
	return (int)mock();
}

int __wrap_align_byte_sequence(
		const struct rtr_bgpsec *data,
		struct stream *s,
		enum align_type type)
{
	UNUSED(data);
	UNUSED(s);
	UNUSED(type);
	return (int)mock();
}

unsigned int __wrap_req_stream_size(const struct rtr_bgpsec *data,
				    enum align_type type)
{
	UNUSED(data);
	UNUSED(type);
	return (int)mock();
}

int __wrap_hash_byte_sequence(
		uint8_t *bytes,
		unsigned int bytes_len,
		uint8_t alg_suite_id,
		unsigned char **hash_result)
{
	UNUSED(bytes);
	UNUSED(bytes_len);
	UNUSED(alg_suite_id);
	UNUSED(hash_result);
	return (int)mock();
}

int __wrap_validate_signature(
		const unsigned char *hash,
		const struct rtr_signature_seg *sig,
		struct spki_record *record)
{
	UNUSED(hash);
	UNUSED(sig);
	UNUSED(record);
	return (int)mock();
}

int __wrap_spki_table_search_by_ski(struct spki_table *spki_table,
				    uint8_t *ski,
				    struct spki_record **result,
				    unsigned int *result_size)
{
	UNUSED(spki_table);
	UNUSED(ski);
	UNUSED(result);
	*result_size = 1;
	return (int)mock();
}

static void test_sanity_checks(void **state)
{
	struct rtr_bgpsec *bgpsec = setup_bgpsec();
	struct spki_table *table = lrtr_malloc(16);
	enum rtr_bgpsec_rtvals result = RTR_BGPSEC_SUCCESS;

	UNUSED(state);

	result = rtr_bgpsec_validate_as_path(NULL, table);
	assert_int_equal(RTR_BGPSEC_MISSING_DATA, result);

	result = rtr_bgpsec_validate_as_path(bgpsec, NULL);
	assert_int_equal(RTR_BGPSEC_MISSING_DATA, result);

	result = rtr_bgpsec_validate_as_path(NULL, NULL);
	assert_int_equal(RTR_BGPSEC_MISSING_DATA, result);

	bgpsec->path_len = 1;
	bgpsec->sigs_len = 2;

	result = rtr_bgpsec_validate_as_path(bgpsec, table);
	assert_int_equal(RTR_BGPSEC_WRONG_SEGMENT_AMOUNT, result);

	bgpsec->path_len = 0;
	bgpsec->sigs_len = 0;
	bgpsec->alg = 3;

	result = rtr_bgpsec_validate_as_path(bgpsec, table);
	assert_int_equal(RTR_BGPSEC_UNSUPPORTED_ALGORITHM_SUITE, result);

	bgpsec->alg = 1;
	bgpsec->nlri.prefix.ver = 8;

	result = rtr_bgpsec_validate_as_path(bgpsec, table);
	assert_int_equal(RTR_BGPSEC_UNSUPPORTED_AFI, result);

	lrtr_free(bgpsec->path);
	lrtr_free(bgpsec->sigs);

	bgpsec->path = NULL;
	bgpsec->sigs = NULL;

	result = rtr_bgpsec_validate_as_path(bgpsec, table);
	assert_int_equal(RTR_BGPSEC_MISSING_DATA, result);

	lrtr_free(table);
	lrtr_free(bgpsec);
}

static void test_check_router_keys(void **state)
{
	struct rtr_bgpsec *bgpsec = setup_bgpsec();
	struct spki_table *table = lrtr_malloc(16);
	enum rtr_bgpsec_rtvals result = RTR_BGPSEC_SUCCESS;

	UNUSED(state);

	will_return(__wrap_check_router_keys, RTR_BGPSEC_ROUTER_KEY_NOT_FOUND);
	result = rtr_bgpsec_validate_as_path(bgpsec, table);

	assert_int_equal(RTR_BGPSEC_ROUTER_KEY_NOT_FOUND, result);

	lrtr_free(table);
	lrtr_free(bgpsec->path);
	lrtr_free(bgpsec->sigs);
	lrtr_free(bgpsec);
}

static void test_align_byte_sequence(void **state)
{
	struct rtr_bgpsec *bgpsec = setup_bgpsec();
	struct spki_table *table = lrtr_malloc(16);
	enum rtr_bgpsec_rtvals result = RTR_BGPSEC_SUCCESS;

	UNUSED(state);

	will_return(__wrap_align_byte_sequence, RTR_BGPSEC_ERROR);
	will_return(__wrap_req_stream_size, 12);
	will_return(__wrap_check_router_keys, RTR_BGPSEC_SUCCESS);
	result = rtr_bgpsec_validate_as_path(bgpsec, table);

	assert_int_equal(RTR_BGPSEC_ERROR, result);

	lrtr_free(table);
	lrtr_free(bgpsec->path);
	lrtr_free(bgpsec->sigs);
	lrtr_free(bgpsec);
}

static void test_hash_byte_sequence(void **state)
{
	struct rtr_bgpsec *bgpsec = setup_bgpsec();
	struct spki_table *table = lrtr_malloc(16);
	enum rtr_bgpsec_rtvals result = RTR_BGPSEC_SUCCESS;

	UNUSED(state);

	will_return(__wrap_hash_byte_sequence, RTR_BGPSEC_ERROR);
	will_return(__wrap_align_byte_sequence, RTR_BGPSEC_SUCCESS);
	will_return(__wrap_req_stream_size, 12);
	will_return(__wrap_check_router_keys, RTR_BGPSEC_SUCCESS);
	result = rtr_bgpsec_validate_as_path(bgpsec, table);

	assert_int_equal(RTR_BGPSEC_ERROR, result);

	lrtr_free(table);
	lrtr_free(bgpsec->path);
	lrtr_free(bgpsec->sigs);
	lrtr_free(bgpsec);
}

static void test_validate_signature(void **state)
{
	struct rtr_bgpsec *bgpsec = setup_bgpsec();
	struct spki_table *table = lrtr_malloc(16);
	enum rtr_bgpsec_rtvals result = RTR_BGPSEC_SUCCESS;

	UNUSED(state);

	will_return(__wrap_validate_signature, RTR_BGPSEC_ERROR);
	will_return(__wrap_spki_table_search_by_ski, SPKI_SUCCESS);
	will_return(__wrap_hash_byte_sequence, RTR_BGPSEC_SUCCESS);
	will_return(__wrap_align_byte_sequence, RTR_BGPSEC_SUCCESS);
	will_return(__wrap_req_stream_size, 12);
	will_return(__wrap_check_router_keys, RTR_BGPSEC_SUCCESS);
	result = rtr_bgpsec_validate_as_path(bgpsec, table);

	assert_int_equal(RTR_BGPSEC_ERROR, result);

	lrtr_free(table);
	lrtr_free(bgpsec->path);
	lrtr_free(bgpsec->sigs);
	lrtr_free(bgpsec);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_sanity_checks),
		cmocka_unit_test(test_check_router_keys),
		cmocka_unit_test(test_align_byte_sequence),
		cmocka_unit_test(test_hash_byte_sequence),
		cmocka_unit_test(test_validate_signature),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
