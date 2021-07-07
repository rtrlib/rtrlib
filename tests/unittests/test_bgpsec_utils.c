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
#include "test_bgpsec_utils.h"

/* Setup and return a bgpsec struct */
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
	return bgpsec;
}

/* Setup and return a signature segment */
struct rtr_signature_seg *setup_sig_seg(void)
{
	struct rtr_signature_seg *sig_seg = NULL;

	const char *ski = "aaaaaaaaaaaaaaaaaaaa"; // 20 times 'a'
	uint16_t sig_len = 10;
	const char *sig = "bbbbbbbbbb"; // 10 times 'b'

	sig_seg = rtr_bgpsec_new_signature_seg((uint8_t *)ski,
					       sig_len,
					       (uint8_t *)sig);
	return sig_seg;
}

/* Setup and return a secure path segment */
struct rtr_secure_path_seg *setup_sec_seg(void)
{
	struct rtr_secure_path_seg *sec_path = NULL;
	uint8_t pcount = 1;
	uint8_t flags = 0;
	uint32_t my_as = 65537;

	sec_path = rtr_bgpsec_new_secure_path_seg(pcount,
						  flags,
						  my_as);
	return sec_path;
}

/* Test all stream functions */
static void test_bgpsec_streams(void **state)
{
	UNUSED(state);

	uint16_t size = 8;
	struct stream *s = init_stream(size);

	uint16_t exp_size = get_stream_size(s);
	uint8_t *stream_start = get_stream_start(s);

	/* Test, if stream is initialized correctly */
	assert_int_equal(size, exp_size);
	assert_int_equal(0, s->w_head);
	assert_int_equal(0, s->r_head);
	assert(stream_start == s->stream);

	/* Write 8 bytes to the stream */
	const char *data = "abcdefgh";

	write_stream(s, (uint8_t *)data, 8);

	/* Check, if the write head moved to position 8 */
	assert_int_equal(s->w_head, 8);

	/* Read 8 bytes from stream. Must be identical to data. */
	for (int i = 0; i < s->size; i++)
		assert_int_equal(read_stream(s), data[i]);

	/* Check, if the read head moved to position 8 */
	assert_int_equal(s->r_head, 8);

	struct stream *s_cpy = copy_stream(s);

	assert_int_equal(s->size, s_cpy->size);
	assert_int_equal(s->r_head, s_cpy->r_head);
	assert_int_equal(s->w_head, s_cpy->w_head);

	// Reset read/write heads.
	s_cpy->w_head = 0;
	s_cpy->r_head = 0;

	/* Repeat the read test, this time on the stream copy */
	for (int i = 0; i < s_cpy->size; i++)
		assert_int_equal(read_stream(s_cpy), data[i]);

	s->w_head = 0; // Reset write heads.

	/* Read 3 bytes from the stream at current read position 2 */
	s->r_head = 2;
	uint8_t buffer_a[3] = {'\0'};

	read_n_bytes_stream(buffer_a, s, 3);

	for (int i = 0; i < 3; i++)
		assert_int_equal(buffer_a[i], data[i + 2]);

	/* Read 6 bytes from the stream starting at position 3.
	 * If you want to read more bytes from the stream than it holds,
	 * bytes are only read till the end of the stream.
	 */
	uint8_t buffer_b[6] = {'\0'};

	read_stream_at(buffer_b, s, 3, 6);

	for (int i = 0; i < 6; i++)
		assert_int_equal(buffer_b[i], data[i + 3]);

	/* Free the stream and its copy */
	free_stream(s_cpy);
	free_stream(s);
}

/* Test the size calculator functions */
static void test_bgpsec_constructors(void **state)
{
	UNUSED(state);

	enum rtr_bgpsec_rtvals retval = RTR_BGPSEC_SUCCESS;

	struct rtr_bgpsec *bgpsec = NULL;
	struct rtr_secure_path_seg *sec_path = NULL;
	struct rtr_signature_seg *sig_seg = NULL;

	struct rtr_signature_seg *mal_sig_seg = NULL;

	uint32_t ip = 0xC0000200;

	/* The signature is not valid, but this is not relevant for the
	 * test. We only check if the information are copied correctly.
	 */
	const char *ski = "aaaaaaaaaaaaaaaaaaaa"; // 20 times 'a'
	uint16_t sig_len = 10;
	const char *sig = "bbbbbbbbbb"; // 10 times 'b'
	const char *mal_ski = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

	bgpsec = setup_bgpsec();

	/* Check every single field in bgpsec that contains a numeric value.
	 * Do the same for the secure path and signature segment.
	 */
	assert_int_equal(1, bgpsec->alg);
	assert_int_equal(1, bgpsec->safi);
	assert_int_equal(1, bgpsec->afi);
	assert_int_equal(65537, bgpsec->my_as);
	assert_int_equal(65538, bgpsec->target_as);
	assert_int_equal(0, bgpsec->path_len);
	assert_int_equal(0, bgpsec->sigs_len);
	assert_int_equal(24, bgpsec->nlri.prefix_len);
	assert_int_equal(LRTR_IPV4, bgpsec->nlri.prefix.ver);
	assert_int_equal(ntohl(ip), bgpsec->nlri.prefix.u.addr4.addr);

	sec_path = setup_sec_seg();

	assert_int_equal(1, sec_path->pcount);
	assert_int_equal(0, sec_path->flags);
	assert_int_equal(65537, sec_path->asn);

	rtr_bgpsec_prepend_sec_path_seg(bgpsec, sec_path);

	/* Check, if the path was appeded to the bgpsec struct. If so,
	 * the information of the first bgpsec path element must be
	 * identical to the information of sec_path.
	 */
	assert_int_equal(1, bgpsec->path_len);
	assert_int_equal(sec_path->pcount, bgpsec->path->pcount);
	assert_int_equal(sec_path->flags, bgpsec->path->flags);
	assert_int_equal(sec_path->asn, bgpsec->path->asn);

	sig_seg = setup_sig_seg();

	assert_int_equal(sig_len, sig_seg->sig_len);

	/* Check, if each byte of ski and signature was copied correctly */
	for (int i = 0; i < SKI_SIZE; i++)
		assert_int_equal('a', sig_seg->ski[i]);

	for (int i = 0; i < sig_len; i++)
		assert_int_equal('b', sig_seg->signature[i]);

	retval = rtr_bgpsec_prepend_sig_seg(bgpsec, sig_seg);
	assert_int_equal(RTR_BGPSEC_SUCCESS, retval);

	/* Same append check as for the secure path segment, but this time
	 * for the signature.
	 */
	assert_int_equal(1, bgpsec->sigs_len);
	assert_int_equal(sig_seg->sig_len, bgpsec->sigs->sig_len);
	for (int i = 0; i < SKI_SIZE; i++)
		assert_int_equal(ski[i], bgpsec->sigs->ski[i]);
	for (int i = 0; i < sig_len; i++)
		assert_int_equal(sig[i], bgpsec->sigs->signature[i]);

	mal_sig_seg = setup_sig_seg();
	memcpy(mal_sig_seg->ski, mal_ski, SKI_SIZE);

	assert_int_equal(1, ski_is_empty((uint8_t *)mal_ski));

	retval = rtr_bgpsec_prepend_sig_seg(bgpsec, mal_sig_seg);
	assert_int_equal(RTR_BGPSEC_ERROR, retval);

	/* Append a segment to the last position of the path. */
	sec_path = setup_sec_seg();
	sec_path->asn = 12345;
	rtr_bgpsec_append_sec_path_seg(bgpsec, sec_path);
	assert_int_equal(2, bgpsec->path_len);

	/* Check, if the element was appended at the last position */
	assert_int_equal(12345, bgpsec->path->next->asn);

	/* Do the same appending process with a signature segment */
	sig_seg = setup_sig_seg();
	sig_seg->sig_len = 42;
	retval = rtr_bgpsec_append_sig_seg(bgpsec, sig_seg);
	assert_int_equal(RTR_BGPSEC_SUCCESS, retval);

	assert_int_equal(42, bgpsec->sigs->next->sig_len);

	/* Do the same appending process with an invalid signature segment */
	retval = rtr_bgpsec_append_sig_seg(bgpsec, mal_sig_seg);
	assert_int_equal(RTR_BGPSEC_ERROR, retval);

	/* Free the bgpsec struct and all signatures and secure paths */
	rtr_bgpsec_free(bgpsec);
	rtr_bgpsec_free_signatures(mal_sig_seg);
}

/* Test size calculator functions */
static void test_bgpsec_sizes(void **state)
{
	UNUSED(state);

	struct rtr_bgpsec *bgpsec = NULL;
	struct rtr_secure_path_seg *sec_path = NULL;
	struct rtr_signature_seg *sig_seg = NULL;

	bgpsec = setup_bgpsec();

	/* Generate five secure path and signature segments and append
	 * them to bgpsec.
	 */
	for (int i = 0; i < 5; i++) {
		sec_path = setup_sec_seg();
		rtr_bgpsec_prepend_sec_path_seg(bgpsec, sec_path);

		sig_seg = setup_sig_seg();
		rtr_bgpsec_prepend_sig_seg(bgpsec, sig_seg);
	}

	/* When calculating the required stream size for validation, the
	 * last appended signature segment is skipped. The calculation goes:
	 * five secure path segments +
	 * four signature segments +
	 * the size of rest (afi, nlri, ...)
	 */
	unsigned int exp_stream_size = (5 * 6) + (4 * 32) + 12;
	unsigned int result = req_stream_size(bgpsec, VALIDATION);

	assert_int_equal(exp_stream_size, result);

	/* For singing, the length of all secure path and signature segments
	 * matters. The calculation goes:
	 * five secure path segments +
	 * five signature segments +
	 * the size of rest (afi, nlri, ...)
	 */
	exp_stream_size = (5 * 6) + (5 * 32) + 12;
	result = req_stream_size(bgpsec, SIGNING);
	assert_int_equal(exp_stream_size, result);

	/* Calculate the signature size only (validation). Again, the first
	 * segment is skipped.
	 */
	int size = get_sig_seg_size(bgpsec->sigs, VALIDATION);

	assert_int_equal((4 * 32), size);

	/* Calculate the signature size only (signing) */
	size = get_sig_seg_size(bgpsec->sigs, SIGNING);
	assert_int_equal((5 * 32), size);

	/* Free the bgpsec struct and all signatures and secure paths */
	rtr_bgpsec_free(bgpsec);
}

/* Test size calculator functions */
static void test_bgpsec_prepend_pop(void **state)
{
	UNUSED(state);

	enum rtr_bgpsec_rtvals retval;

	struct rtr_bgpsec *bgpsec = NULL;
	struct rtr_secure_path_seg *sec_path = NULL;
	struct rtr_signature_seg *sig_seg = NULL;

	bgpsec = setup_bgpsec();
	sec_path = setup_sec_seg();
	sig_seg = setup_sig_seg();

	/* Path length = signature length = 1 */
	rtr_bgpsec_prepend_sec_path_seg(bgpsec, sec_path);
	retval = rtr_bgpsec_prepend_sig_seg(bgpsec, sig_seg);

	assert_int_equal(RTR_BGPSEC_SUCCESS, retval);
	assert_int_equal(1, bgpsec->path_len);
	assert_int_equal(1, bgpsec->sigs_len);

	sec_path = setup_sec_seg();
	sig_seg = setup_sig_seg();

	/* Change some values so we can later validate that these
	 * specific segments were returned.
	 */
	sec_path->asn = 12345;
	sig_seg->sig_len = 80;

	/* Path length = signature length = 2 */
	rtr_bgpsec_prepend_sec_path_seg(bgpsec, sec_path);
	retval = rtr_bgpsec_prepend_sig_seg(bgpsec, sig_seg);

	assert_int_equal(RTR_BGPSEC_SUCCESS, retval);
	assert_int_equal(2, bgpsec->path_len);
	assert_int_equal(2, bgpsec->sigs_len);

	sec_path = NULL;
	sig_seg = NULL;

	/* Path length = signature length = 1 */
	sec_path = rtr_bgpsec_pop_secure_path_seg(bgpsec);
	sig_seg = rtr_bgpsec_pop_signature_seg(bgpsec);

	assert_int_equal(1, bgpsec->path_len);
	assert_int_equal(1, bgpsec->sigs_len);
	assert_int_equal(12345, sec_path->asn);
	assert_int_equal(80, sig_seg->sig_len);
	assert(sec_path);
	assert(sig_seg);
	assert(!sec_path->next);
	assert(!sig_seg->next);

	rtr_bgpsec_free_secure_path(sec_path);
	rtr_bgpsec_free_signatures(sig_seg);

	sec_path = NULL;
	sig_seg = NULL;

	/* Path length = signature length = 0 */
	sec_path = rtr_bgpsec_pop_secure_path_seg(bgpsec);
	sig_seg = rtr_bgpsec_pop_signature_seg(bgpsec);

	assert_int_equal(0, bgpsec->path_len);
	assert_int_equal(0, bgpsec->sigs_len);
	assert_int_equal(65537, sec_path->asn);
	assert_int_equal(10, sig_seg->sig_len);
	assert(sec_path);
	assert(sig_seg);
	assert(!sec_path->next);
	assert(!sig_seg->next);

	rtr_bgpsec_free_secure_path(sec_path);
	rtr_bgpsec_free_signatures(sig_seg);

	sec_path = NULL;
	sig_seg = NULL;

	/* Path length = signature length = 0 */
	sec_path = rtr_bgpsec_pop_secure_path_seg(bgpsec);
	sig_seg = rtr_bgpsec_pop_signature_seg(bgpsec);
	assert(!sec_path);
	assert(!sig_seg);

	rtr_bgpsec_free(bgpsec);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_bgpsec_streams),
		cmocka_unit_test(test_bgpsec_constructors),
		cmocka_unit_test(test_bgpsec_sizes),
		cmocka_unit_test(test_bgpsec_prepend_pop),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
