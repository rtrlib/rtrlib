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
#include "rtrlib/bgpsec/bgpsec.c"
#include "test_bgpsec_signing.h"

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

int __wrap_load_private_key(EC_KEY **priv_key, uint8_t *bytes_key)
{
	UNUSED(priv_key);
	UNUSED(bytes_key);
	return (int)mock();
}

int __wrap_ECDSA_size(const EC_KEY *key)
{
	UNUSED(key);
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

int __wrap_sign_byte_sequence(uint8_t *hash_result,
			      EC_KEY *priv_key,
			      uint8_t alg,
			      struct rtr_signature_seg *new_signature)
{
	UNUSED(hash_result);
	UNUSED(priv_key);
	UNUSED(alg);
	UNUSED(new_signature);
	return (int)mock();
}

static void test_sanity_checks(void **state)
{
	struct rtr_bgpsec *bgpsec = setup_bgpsec();
	uint8_t *private_key = lrtr_malloc(20);
	struct rtr_signature_seg *new_signature = NULL;
	enum rtr_bgpsec_rtvals result = RTR_BGPSEC_SUCCESS;
	struct rtr_signature_seg *not_empty =
				lrtr_malloc(sizeof(struct rtr_signature_seg));

	UNUSED(state);

	bgpsec->path_len = 2;
	bgpsec->sigs_len = 1;

	result = rtr_bgpsec_generate_signature(NULL, private_key,
					       &new_signature);
	assert_int_equal(RTR_BGPSEC_ERROR, result);

	result = rtr_bgpsec_generate_signature(bgpsec, NULL,
					       &new_signature);
	assert_int_equal(RTR_BGPSEC_ERROR, result);

	result = rtr_bgpsec_generate_signature(NULL, NULL,
					       &new_signature);
	assert_int_equal(RTR_BGPSEC_ERROR, result);

	bgpsec->path_len = 1;
	bgpsec->sigs_len = 1;

	result = rtr_bgpsec_generate_signature(bgpsec, private_key,
					       &new_signature);
	assert_int_equal(RTR_BGPSEC_WRONG_SEGMENT_AMOUNT, result);

	bgpsec->path_len = 2;
	bgpsec->sigs_len = 1;

	result = rtr_bgpsec_generate_signature(bgpsec, private_key,
					       &not_empty);
	assert_int_equal(RTR_BGPSEC_ERROR, result);

	lrtr_free(bgpsec->path);
	lrtr_free(bgpsec->sigs);
	bgpsec->path = NULL;

	result = rtr_bgpsec_generate_signature(bgpsec, private_key,
					       &new_signature);
	assert_int_equal(RTR_BGPSEC_ERROR, result);

	lrtr_free(private_key);
	lrtr_free(bgpsec);
	lrtr_free(not_empty);
	rtr_bgpsec_free_signatures(new_signature);
}

static void test_load_private_key(void **state)
{
	struct rtr_bgpsec *bgpsec = setup_bgpsec();
	uint8_t *private_key = lrtr_malloc(20);
	struct rtr_signature_seg *new_signature = NULL;
	enum rtr_bgpsec_rtvals result = RTR_BGPSEC_SUCCESS;

	UNUSED(state);

	bgpsec->path_len = 2;
	bgpsec->sigs_len = 1;

	will_return(__wrap_load_private_key, RTR_BGPSEC_LOAD_PRIV_KEY_ERROR);
	result = rtr_bgpsec_generate_signature(bgpsec, private_key,
					       &new_signature);
	assert_int_equal(RTR_BGPSEC_LOAD_PRIV_KEY_ERROR, result);

	lrtr_free(private_key);
	lrtr_free(bgpsec->path);
	lrtr_free(bgpsec->sigs);
	lrtr_free(bgpsec);
	rtr_bgpsec_free_signatures(new_signature);
}

static void test_ecdsa_size(void **state)
{
	struct rtr_bgpsec *bgpsec = setup_bgpsec();
	uint8_t *private_key = lrtr_malloc(20);
	struct rtr_signature_seg *new_signature = NULL;
	enum rtr_bgpsec_rtvals result = RTR_BGPSEC_SUCCESS;

	UNUSED(state);

	bgpsec->path_len = 2;
	bgpsec->sigs_len = 1;

	will_return(__wrap_ECDSA_size, 0);
	will_return(__wrap_load_private_key, RTR_BGPSEC_SUCCESS);
	result = rtr_bgpsec_generate_signature(bgpsec, private_key,
					       &new_signature);
	assert_int_equal(RTR_BGPSEC_LOAD_PRIV_KEY_ERROR, result);

	lrtr_free(private_key);
	lrtr_free(bgpsec->path);
	lrtr_free(bgpsec->sigs);
	lrtr_free(bgpsec);
	rtr_bgpsec_free_signatures(new_signature);
}

static void test_align_byte_sequence(void **state)
{
	struct rtr_bgpsec *bgpsec = setup_bgpsec();
	uint8_t *private_key = lrtr_malloc(20);
	struct rtr_signature_seg *new_signature = NULL;
	enum rtr_bgpsec_rtvals result = RTR_BGPSEC_SUCCESS;

	UNUSED(state);

	bgpsec->path_len = 2;
	bgpsec->sigs_len = 1;

	will_return(__wrap_align_byte_sequence, RTR_BGPSEC_ERROR);
	will_return(__wrap_req_stream_size, 20);
	will_return(__wrap_ECDSA_size, 10);
	will_return(__wrap_load_private_key, RTR_BGPSEC_SUCCESS);
	result = rtr_bgpsec_generate_signature(bgpsec, private_key,
					       &new_signature);
	assert_int_equal(RTR_BGPSEC_ERROR, result);

	lrtr_free(private_key);
	lrtr_free(bgpsec->path);
	lrtr_free(bgpsec->sigs);
	lrtr_free(bgpsec);
	rtr_bgpsec_free_signatures(new_signature);
}

static void test_hash_byte_sequence(void **state)
{
	struct rtr_bgpsec *bgpsec = setup_bgpsec();
	uint8_t *private_key = lrtr_malloc(20);
	struct rtr_signature_seg *new_signature = NULL;
	enum rtr_bgpsec_rtvals result = RTR_BGPSEC_SUCCESS;

	UNUSED(state);

	bgpsec->path_len = 2;
	bgpsec->sigs_len = 1;

	will_return(__wrap_hash_byte_sequence, RTR_BGPSEC_ERROR);
	will_return(__wrap_align_byte_sequence, RTR_BGPSEC_SUCCESS);
	will_return(__wrap_req_stream_size, 20);
	will_return(__wrap_ECDSA_size, 10);
	will_return(__wrap_load_private_key, RTR_BGPSEC_SUCCESS);
	result = rtr_bgpsec_generate_signature(bgpsec, private_key,
					       &new_signature);
	assert_int_equal(RTR_BGPSEC_ERROR, result);

	lrtr_free(private_key);
	lrtr_free(bgpsec->path);
	lrtr_free(bgpsec->sigs);
	lrtr_free(bgpsec);
	rtr_bgpsec_free_signatures(new_signature);
}

static void test_sign_byte_sequence(void **state)
{
	struct rtr_bgpsec *bgpsec = setup_bgpsec();
	uint8_t *private_key = lrtr_malloc(20);
	struct rtr_signature_seg *new_signature = NULL;
	enum rtr_bgpsec_rtvals result = RTR_BGPSEC_SUCCESS;

	UNUSED(state);

	bgpsec->path_len = 2;
	bgpsec->sigs_len = 1;

	will_return(__wrap_sign_byte_sequence, RTR_BGPSEC_SIGNING_ERROR);
	will_return(__wrap_hash_byte_sequence, RTR_BGPSEC_SUCCESS);
	will_return(__wrap_align_byte_sequence, RTR_BGPSEC_SUCCESS);
	will_return(__wrap_req_stream_size, 20);
	will_return(__wrap_ECDSA_size, 10);
	will_return(__wrap_load_private_key, RTR_BGPSEC_SUCCESS);
	result = rtr_bgpsec_generate_signature(bgpsec, private_key,
					       &new_signature);
	assert_int_equal(RTR_BGPSEC_SIGNING_ERROR, result);

	lrtr_free(private_key);
	lrtr_free(bgpsec->path);
	lrtr_free(bgpsec->sigs);
	lrtr_free(bgpsec);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_sanity_checks),
		cmocka_unit_test(test_load_private_key),
		cmocka_unit_test(test_ecdsa_size),
		cmocka_unit_test(test_align_byte_sequence),
		cmocka_unit_test(test_hash_byte_sequence),
		cmocka_unit_test(test_sign_byte_sequence),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
