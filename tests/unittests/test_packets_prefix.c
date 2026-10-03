/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 */

#include "rtrlib_unittests.h"
#include "test_packets_prefix.h"

#include "rtrlib/pfx/pfx.h"
#include "rtrlib/rtr/packets_private.h"
#include "rtrlib/rtr/rtr_pdus.h"

#include <arpa/inet.h>
#include <string.h>

static const uint8_t *recv_data;
static size_t recv_data_len;
static size_t recv_data_offset;
static int expect_error_pdu;

int __wrap_tr_recv_all(const struct rtr_tr_socket *socket, const void *buf, const size_t len, const time_t timeout)
{
	UNUSED(socket);
	UNUSED(timeout);

	assert_non_null(recv_data);
	assert_true(recv_data_offset + len <= recv_data_len);

	memcpy((void *)buf, recv_data + recv_data_offset, len);
	recv_data_offset += len;

	return (int)len;
}

int __wrap_tr_send_all(const struct rtr_tr_socket *socket, const void *pdu, const size_t len, const time_t timeout)
{
	struct pdu_error *error_pdu = (struct pdu_error *)pdu;

	UNUSED(socket);
	UNUSED(timeout);

	assert_true(expect_error_pdu);
	assert_int_equal(error_pdu->type, ERROR);
	assert_int_equal(ntohs(error_pdu->error_code), CORRUPT_DATA);
	assert_int_equal(ntohl(error_pdu->len), len);

	expect_error_pdu = 0;

	return (int)mock();
}

static struct pdu_cache_response cache_response(uint16_t session_id)
{
	struct pdu_cache_response pdu = {0};

	pdu.ver = RTR_PROTOCOL_VERSION_1;
	pdu.type = CACHE_RESPONSE;
	pdu.session_id = htons(session_id);
	pdu.len = htonl(sizeof(pdu));

	return pdu;
}

static struct pdu_end_of_data_v1_v2 end_of_data(uint16_t session_id)
{
	struct pdu_end_of_data_v1_v2 pdu = {0};

	pdu.ver = RTR_PROTOCOL_VERSION_1;
	pdu.type = EOD;
	pdu.session_id = htons(session_id);
	pdu.len = htonl(sizeof(pdu));
	pdu.sn = htonl(1);
	pdu.refresh_interval = htonl(RTR_REFRESH_MIN);
	pdu.retry_interval = htonl(RTR_RETRY_MIN);
	pdu.expire_interval = htonl(RTR_EXPIRATION_MIN);

	return pdu;
}

static struct pdu_ipv4 ipv4_prefix_pdu(uint8_t prefix_len, uint8_t max_prefix_len)
{
	struct pdu_ipv4 pdu = {0};

	pdu.ver = RTR_PROTOCOL_VERSION_1;
	pdu.type = IPV4_PREFIX;
	pdu.len = htonl(sizeof(pdu));
	pdu.flags = 1;
	pdu.prefix_len = prefix_len;
	pdu.max_prefix_len = max_prefix_len;
	assert_int_equal(inet_pton(AF_INET, "10.0.113.0", &pdu.prefix), 1);
	pdu.asn = htonl(64512);

	return pdu;
}

static struct pdu_ipv6 ipv6_prefix_pdu(uint8_t prefix_len, uint8_t max_prefix_len)
{
	struct pdu_ipv6 pdu = {0};

	pdu.ver = RTR_PROTOCOL_VERSION_1;
	pdu.type = IPV6_PREFIX;
	pdu.len = htonl(sizeof(pdu));
	pdu.flags = 1;
	pdu.prefix_len = prefix_len;
	pdu.max_prefix_len = max_prefix_len;
	assert_int_equal(inet_pton(AF_INET6, "2001:db8::", pdu.prefix), 1);
	pdu.asn = htonl(64512);

	return pdu;
}

static void init_socket(struct rtr_socket *socket, struct rtr_pfx_table *pfx_table)
{
	memset(socket, 0, sizeof(*socket));
	socket->state = RTR_SYNC;
	socket->version = RTR_PROTOCOL_VERSION_1;
	socket->session_id = 0x1234;
	socket->request_session_id = false;
	socket->iv_mode = RTR_INTERVAL_MODE_IGNORE_ANY;
	socket->pfx_table = pfx_table;
}

static int run_sync_with_stream(struct rtr_socket *socket, const uint8_t *stream, size_t stream_len,
				int expect_error)
{
	recv_data = stream;
	recv_data_len = stream_len;
	recv_data_offset = 0;
	expect_error_pdu = expect_error;

	if (expect_error)
		will_return(__wrap_tr_send_all, 1);

	return rtr_sync(socket);
}

static void test_ipv4_prefix_max_length_255_is_rejected(void **state)
{
	struct rtr_socket socket;
	struct rtr_pfx_table pfx_table;
	struct pdu_cache_response cr = cache_response(0x1234);
	struct pdu_ipv4 prefix = ipv4_prefix_pdu(24, 255);
	uint8_t stream[sizeof(cr) + sizeof(prefix)];

	UNUSED(state);

	rtr_pfx_table_init(&pfx_table, NULL);
	init_socket(&socket, &pfx_table);

	memcpy(stream, &cr, sizeof(cr));
	memcpy(stream + sizeof(cr), &prefix, sizeof(prefix));

	assert_int_equal(run_sync_with_stream(&socket, stream, sizeof(stream), 1), RTR_ERROR);
	assert_int_equal(recv_data_offset, sizeof(stream));
	assert_false(expect_error_pdu);

	rtr_pfx_table_free(&pfx_table);
}

static void test_ipv6_prefix_max_length_129_is_rejected(void **state)
{
	struct rtr_socket socket;
	struct rtr_pfx_table pfx_table;
	struct pdu_cache_response cr = cache_response(0x1234);
	struct pdu_ipv6 prefix = ipv6_prefix_pdu(32, 129);
	uint8_t stream[sizeof(cr) + sizeof(prefix)];

	UNUSED(state);

	rtr_pfx_table_init(&pfx_table, NULL);
	init_socket(&socket, &pfx_table);

	memcpy(stream, &cr, sizeof(cr));
	memcpy(stream + sizeof(cr), &prefix, sizeof(prefix));

	assert_int_equal(run_sync_with_stream(&socket, stream, sizeof(stream), 1), RTR_ERROR);
	assert_int_equal(recv_data_offset, sizeof(stream));
	assert_false(expect_error_pdu);

	rtr_pfx_table_free(&pfx_table);
}

static void test_valid_ipv4_prefix_is_accepted(void **state)
{
	struct rtr_socket socket;
	struct rtr_pfx_table pfx_table;
	struct pdu_cache_response cr = cache_response(0x1234);
	struct pdu_ipv4 prefix = ipv4_prefix_pdu(24, 24);
	struct pdu_end_of_data_v1_v2 eod = end_of_data(0x1234);
	struct rtr_ip_addr addr;
	enum rtr_pfxv_state result;
	uint8_t stream[sizeof(cr) + sizeof(prefix) + sizeof(eod)];

	UNUSED(state);

	rtr_pfx_table_init(&pfx_table, NULL);
	init_socket(&socket, &pfx_table);

	memcpy(stream, &cr, sizeof(cr));
	memcpy(stream + sizeof(cr), &prefix, sizeof(prefix));
	memcpy(stream + sizeof(cr) + sizeof(prefix), &eod, sizeof(eod));

	assert_int_equal(run_sync_with_stream(&socket, stream, sizeof(stream), 0), RTR_SUCCESS);
	assert_int_equal(recv_data_offset, sizeof(stream));
	assert_false(expect_error_pdu);
	assert_int_equal(rtr_ip_str_to_addr("10.0.113.0", &addr), 0);
	assert_int_equal(rtr_pfx_table_validate(&pfx_table, 64512, &addr, 24, &result), RTR_PFX_SUCCESS);
	assert_int_equal(result, RTR_BGP_PFXV_STATE_VALID);

	rtr_pfx_table_free(&pfx_table);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_ipv4_prefix_max_length_255_is_rejected),
		cmocka_unit_test(test_ipv6_prefix_max_length_129_is_rejected),
		cmocka_unit_test(test_valid_ipv4_prefix_is_accepted),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
