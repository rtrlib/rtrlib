/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 */

#include "rtrlib_unittests.h"
#include "test_packets_eod.h"

#include "rtrlib/rtr/packets_private.h"
#include "rtrlib/rtr/rtr_pdus.h"
#include <arpa/inet.h>
#include <string.h>

static const uint8_t *recv_data;
static size_t recv_data_len;
static size_t recv_data_offset;

struct expected_error_pdu {
	struct pdu_end_of_data_v1_v2 eod_network;
	uint32_t error_len;
	int enabled;
};

static struct expected_error_pdu expected_error;

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

	assert_true(expected_error.enabled);
	assert_int_equal(len, expected_error.error_len);
	assert_int_equal(error_pdu->ver, RTR_PROTOCOL_VERSION_1);
	assert_int_equal(error_pdu->type, ERROR);
	assert_int_equal(ntohs(error_pdu->error_code), CORRUPT_DATA);
	assert_int_equal(ntohl(error_pdu->len), expected_error.error_len);
	assert_int_equal(ntohl(error_pdu->len_enc_pdu), sizeof(expected_error.eod_network));
	assert_memory_equal(error_pdu->rest, &expected_error.eod_network, sizeof(expected_error.eod_network));

	expected_error.enabled = 0;

	return (int)mock();
}

static void test_eod_session_mismatch_error_uses_eod_length(void **state)
{
	struct rtr_socket socket = {0};
	struct pdu_cache_response cache_response = {0};
	struct pdu_end_of_data_v1_v2 eod = {0};
	struct pdu_cache_response cache_response_network = {0};
	uint8_t stream[sizeof(cache_response) + sizeof(eod)];
	char txt[67];
	int ret;

	UNUSED(state);

	socket.state = RTR_SYNC;
	socket.version = RTR_PROTOCOL_VERSION_1;
	socket.session_id = 0x1234;
	socket.request_session_id = false;
	socket.iv_mode = RTR_INTERVAL_MODE_IGNORE_ANY;

	cache_response.ver = RTR_PROTOCOL_VERSION_1;
	cache_response.type = CACHE_RESPONSE;
	cache_response.session_id = socket.session_id;
	cache_response.len = sizeof(cache_response);

	eod.ver = RTR_PROTOCOL_VERSION_1;
	eod.type = EOD;
	eod.session_id = 0x5678;
	eod.len = sizeof(eod);
	eod.sn = 1;
	eod.refresh_interval = RTR_REFRESH_MIN;
	eod.retry_interval = RTR_RETRY_MIN;
	eod.expire_interval = RTR_EXPIRATION_MIN;

	cache_response_network = cache_response;
	cache_response_network.session_id = htons(cache_response_network.session_id);
	cache_response_network.len = htonl(cache_response_network.len);

	expected_error.eod_network = eod;
	expected_error.eod_network.session_id = htons(expected_error.eod_network.session_id);
	expected_error.eod_network.len = htonl(expected_error.eod_network.len);
	expected_error.eod_network.sn = htonl(expected_error.eod_network.sn);
	expected_error.eod_network.refresh_interval = htonl(expected_error.eod_network.refresh_interval);
	expected_error.eod_network.retry_interval = htonl(expected_error.eod_network.retry_interval);
	expected_error.eod_network.expire_interval = htonl(expected_error.eod_network.expire_interval);
	expected_error.enabled = 1;

	snprintf(txt, sizeof(txt), "Expected session_id: %u, received session_id. %u in EOD PDU", socket.session_id,
		 eod.session_id);
	expected_error.error_len = sizeof(struct pdu_error) + 4 + sizeof(eod) + strlen(txt) + 1;

	memcpy(stream, &cache_response_network, sizeof(cache_response_network));
	memcpy(stream + sizeof(cache_response_network), &expected_error.eod_network,
	       sizeof(expected_error.eod_network));
	recv_data = stream;
	recv_data_len = sizeof(stream);
	recv_data_offset = 0;

	will_return(__wrap_tr_send_all, expected_error.error_len);

	ret = rtr_sync(&socket);

	assert_int_equal(ret, RTR_ERROR);
	assert_int_equal(socket.state, RTR_ERROR_FATAL);
	assert_int_equal(recv_data_offset, sizeof(stream));
	assert_false(expected_error.enabled);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_eod_session_mismatch_error_uses_eod_length),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
