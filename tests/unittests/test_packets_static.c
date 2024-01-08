/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website; http://rtrlib.realmv6.org/
 */

#include "rtrlib_unittests.h"
#include "test_packets_static.h"

#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/rtr/packets.c"
#include "rtrlib/rtr_mgr_private.h"

int __wrap_lrtr_get_monotonic_time(time_t *seconds)
{
	UNUSED(seconds);
	return mock_type(int);
}

int __wrap_tr_send_all(const struct tr_socket *socket, const void *pdu, const size_t len, const time_t timeout)
{
	check_expected(socket);
	check_expected(pdu);
	check_expected(len);
	check_expected(timeout);
	return (int)mock();
}

static int cmp_serial_in_error_pdu(const LargestIntegralType test, const LargestIntegralType reference)
{
	struct pdu_error *test_pdu = (struct pdu_error *)test;
	struct pdu_serial_query *reference_pdu = (struct pdu_serial_query *)reference;
	struct pdu_serial_query *encapsulated_pdu = (struct pdu_serial_query *)test_pdu->rest;

	if (encapsulated_pdu->ver != reference_pdu->ver || encapsulated_pdu->type != reference_pdu->type ||
	    encapsulated_pdu->session_id != reference_pdu->session_id || encapsulated_pdu->len != reference_pdu->len ||
	    encapsulated_pdu->sn != reference_pdu->sn) {
		return 0;
	}

	return 1;
}

static int cmp_header_in_error_pdu(const LargestIntegralType test, const LargestIntegralType reference)
{
	struct pdu_error *test_pdu = (struct pdu_error *)test;
	struct pdu_serial_query *reference_pdu = (struct pdu_serial_query *)reference;
	struct pdu_serial_query *encapsulated_pdu = (struct pdu_serial_query *)test_pdu->rest;

	if (encapsulated_pdu->ver != reference_pdu->ver || encapsulated_pdu->type != reference_pdu->type ||
	    encapsulated_pdu->session_id != reference_pdu->session_id || encapsulated_pdu->len != reference_pdu->len) {
		return 0;
	}

	return 1;
}

static void test_set_last_update(void **state)
{
	struct rtr_socket socket;

	UNUSED(state);

	socket.connection_state_fp = NULL;

	will_return(__wrap_lrtr_get_monotonic_time, RTR_ERROR);
	assert_int_equal(rtr_set_last_update(&socket), RTR_ERROR);

	will_return(__wrap_lrtr_get_monotonic_time, RTR_SUCCESS);
	assert_int_equal(rtr_set_last_update(&socket), RTR_SUCCESS);
}

static void test_rtr_get_pdu_type(void **state)
{
	struct pdu_header pdu;

	UNUSED(state);

	pdu.type = SERIAL_NOTIFY;
	assert_int_equal(rtr_get_pdu_type(&pdu), SERIAL_NOTIFY);

	pdu.type = SERIAL_QUERY;
	assert_int_equal(rtr_get_pdu_type(&pdu), SERIAL_QUERY);

	pdu.type = RESET_QUERY;
	assert_int_equal(rtr_get_pdu_type(&pdu), RESET_QUERY);

	pdu.type = CACHE_RESPONSE;
	assert_int_equal(rtr_get_pdu_type(&pdu), CACHE_RESPONSE);

	pdu.type = RESERVED;
	assert_int_equal(rtr_get_pdu_type(&pdu), RESERVED);

	pdu.type = IPV6_PREFIX;
	assert_int_equal(rtr_get_pdu_type(&pdu), IPV6_PREFIX);

	pdu.type = EOD;
	assert_int_equal(rtr_get_pdu_type(&pdu), EOD);

	pdu.type = CACHE_RESET;
	assert_int_equal(rtr_get_pdu_type(&pdu), CACHE_RESET);

	pdu.type = ROUTER_KEY;
	assert_int_equal(rtr_get_pdu_type(&pdu), ROUTER_KEY);

	pdu.type = ERROR;
	assert_int_equal(rtr_get_pdu_type(&pdu), ERROR);
}

static void test_pdu_to_network_byte_order(void **state)
{
	struct pdu_serial_query pdu_serial;
	struct pdu_aspa *aspa = malloc(24);

	memset(aspa, 0, 24);

	UNUSED(state);

	pdu_serial.ver = 0;
	pdu_serial.type = SERIAL_QUERY;
	pdu_serial.session_id = 0x32A5;
	pdu_serial.len = 0xC;
	pdu_serial.sn = 0xCC56E9BB;

	rtr_pdu_to_network_byte_order(&pdu_serial);

	assert_int_equal(pdu_serial.ver, 0);
	assert_int_equal(pdu_serial.type, SERIAL_QUERY);
	assert_int_equal(pdu_serial.session_id, 0xA532);
	assert_int_equal(pdu_serial.len, 0x0C000000);
	assert_int_equal(pdu_serial.sn, 0xBBE956CC);

	aspa->ver = 2;
	aspa->type = ASPA;
	aspa->zero = 0;
	aspa->flags = 0xC6;
	aspa->afi_flags = 0xC6;
	aspa->provider_count = 2;
	aspa->len = 0x18;
	aspa->customer_asn = 0xCC56E9BB;
	aspa->provider_asns[0] = 0xBADCFE00;
	aspa->provider_asns[1] = 0x1122;

	rtr_pdu_to_network_byte_order(aspa);

	assert_int_equal(aspa->ver, 2);
	assert_int_equal(aspa->type, ASPA);
	assert_int_equal(aspa->zero, 0);
	assert_int_equal(aspa->flags, 0xC6);
	assert_int_equal(aspa->afi_flags, 0xC6);
	assert_int_equal(aspa->len, 0x18000000);
	assert_int_equal(aspa->customer_asn, 0xBBE956CC);
	assert_int_equal(aspa->provider_count, 0x0200);
	assert_int_equal(aspa->provider_asns[0], 0xFEDCBA);
	assert_int_equal(aspa->provider_asns[1], 0x22110000);
}

static void test_pdu_to_host_byte_order(void **state)
{
	struct pdu_serial_notify pdu_serial;
	struct pdu_end_of_data_v1_v2 pdu_eod;
	struct pdu_aspa *aspa = malloc(24);

	memset(aspa, 0, 24);

	UNUSED(state);

	pdu_serial.ver = 1;
	pdu_serial.type = SERIAL_NOTIFY;
	pdu_serial.session_id = 0xDDFF;
	pdu_serial.len = 0xC;
	pdu_serial.sn = 0xDF;

	rtr_pdu_footer_to_host_byte_order(&pdu_serial);
	rtr_pdu_header_to_host_byte_order(&pdu_serial);

	assert_int_equal(pdu_serial.ver, 1);
	assert_int_equal(pdu_serial.type, SERIAL_NOTIFY);
	assert_int_equal(pdu_serial.len, 0xC000000);
	assert_int_equal(pdu_serial.sn, 0xDF000000);

	pdu_eod.ver = 1;
	pdu_eod.type = EOD;
	pdu_eod.session_id = 0xFDDF;
	pdu_eod.len = 0x18;
	pdu_eod.sn = 0xFEDCBA;
	pdu_eod.refresh_interval = 0xAF;
	pdu_eod.retry_interval = 0xDC;
	pdu_eod.expire_interval = 0xCCF;

	rtr_pdu_header_to_host_byte_order(&pdu_eod);
	rtr_pdu_footer_to_host_byte_order(&pdu_eod);

	assert_int_equal(pdu_eod.ver, 1);
	assert_int_equal(pdu_eod.type, EOD);
	assert_int_equal(pdu_eod.session_id, 0xDFFD);
	assert_int_equal(pdu_eod.len, 0x18000000);
	assert_int_equal(pdu_eod.sn, 0xBADCFE00);
	assert_int_equal(pdu_eod.refresh_interval, 0xAF000000);
	assert_int_equal(pdu_eod.retry_interval, 0xDC000000);
	assert_int_equal(pdu_eod.expire_interval, 0xCF0C0000);

	aspa->ver = 2;
	aspa->type = ASPA;
	aspa->zero = 0;
	aspa->flags = 0xC6;
	aspa->afi_flags = 0xC6;
	aspa->provider_count = 0x0200;
	aspa->len = 0x18000000;
	aspa->customer_asn = 0xBBE956CC;
	aspa->provider_asns[0] = 0xFEDCBA;
	aspa->provider_asns[1] = 0x22110000;

	rtr_pdu_header_to_host_byte_order(aspa);
	rtr_pdu_footer_to_host_byte_order(aspa);

	assert_int_equal(aspa->ver, 2);
	assert_int_equal(aspa->type, ASPA);
	assert_int_equal(aspa->zero, 0);
	assert_int_equal(aspa->flags, 0xC6);
	assert_int_equal(aspa->afi_flags, 0xC6);
	assert_int_equal(aspa->len, 0x18);
	assert_int_equal(aspa->customer_asn, 0xCC56E9BB);
	assert_int_equal(aspa->provider_count, 2);
	assert_int_equal(aspa->provider_asns[0], 0xBADCFE00);
	assert_int_equal(aspa->provider_asns[1], 0x1122);
}

static void test_rtr_pdu_check_size(void **state)
{
	struct pdu_header pdu;
	struct pdu_error *error = malloc(30);
	struct pdu_aspa *aspa = malloc(24);

	UNUSED(state);

	memset(error, 0, 30);
	memset(aspa, 0, 24);

	pdu.type = SERIAL_NOTIFY;
	pdu.len = 20;
	assert_false(rtr_pdu_check_size(&pdu));
	pdu.len = 12;
	assert_true(rtr_pdu_check_size(&pdu));

	pdu.type = CACHE_RESPONSE;
	pdu.len = 5;
	assert_false(rtr_pdu_check_size(&pdu));
	pdu.len = 8;
	assert_true(rtr_pdu_check_size(&pdu));

	pdu.type = IPV4_PREFIX;
	pdu.len = 25;
	assert_false(rtr_pdu_check_size(&pdu));
	pdu.len = 20;
	assert_true(rtr_pdu_check_size(&pdu));

	pdu.type = IPV6_PREFIX;
	pdu.len = 15;
	assert_false(rtr_pdu_check_size(&pdu));
	pdu.len = 32;
	assert_true(rtr_pdu_check_size(&pdu));

	pdu.type = CACHE_RESET;
	pdu.len = 10;
	assert_false(rtr_pdu_check_size(&pdu));
	pdu.len = 8;
	assert_true(rtr_pdu_check_size(&pdu));

	pdu.type = SERIAL_QUERY;
	pdu.len = 14;
	assert_false(rtr_pdu_check_size(&pdu));
	pdu.len = 12;
	assert_true(rtr_pdu_check_size(&pdu));

	pdu.type = RESET_QUERY;
	pdu.len = 10;
	assert_false(rtr_pdu_check_size(&pdu));
	pdu.len = 8;
	assert_true(rtr_pdu_check_size(&pdu));

	pdu.type = RESERVED;
	pdu.len = 0;
	assert_false(rtr_pdu_check_size(&pdu));

	pdu.type = EOD;
	pdu.ver = RTR_PROTOCOL_VERSION_1;
	pdu.len = 12;
	assert_false(rtr_pdu_check_size(&pdu));
	pdu.len = 24;
	assert_true(rtr_pdu_check_size(&pdu));
	pdu.ver = RTR_PROTOCOL_VERSION_0;
	pdu.len = 18;
	assert_false(rtr_pdu_check_size(&pdu));
	pdu.len = 12;
	assert_true(rtr_pdu_check_size(&pdu));

	pdu.type = ROUTER_KEY;
	pdu.len = 124;
	assert_false(rtr_pdu_check_size(&pdu));
	pdu.len = 123;
	assert_true(rtr_pdu_check_size(&pdu));

	/* Test error pdu size checks */
	error->type = ERROR;
	error->len = 14;
	error->len_enc_pdu = 0;
	assert_false(rtr_pdu_check_size((struct pdu_header *)error));
	error->len = 20;
	error->len_enc_pdu = 0x8000000;
	assert_false(rtr_pdu_check_size((struct pdu_header *)error));
	error->len = 24;
	error->rest[11] = 0xA;
	assert_false(rtr_pdu_check_size((struct pdu_header *)error));

	/* Test ASPA pdu size checks */
	aspa->type = ASPA;
	aspa->len = 25;
	aspa->provider_count = 0x0200;
	assert_false(rtr_pdu_check_size((struct pdu_header *)aspa));
	aspa->len = 23;
	assert_false(rtr_pdu_check_size((struct pdu_header *)aspa));
	aspa->len = 24;
	assert_true(rtr_pdu_check_size((struct pdu_header *)aspa));
}

static void test_rtr_send_error_pdu(void **state)
{
	struct pdu_serial_query pdu_serial, pdu_serial_network;
	struct rtr_socket socket;
	int ret;

	UNUSED(state);

	pdu_serial.ver = 1;
	pdu_serial.type = SERIAL_NOTIFY;
	pdu_serial.session_id = 0xDDFF;
	pdu_serial.len = sizeof(struct pdu_serial_query);
	pdu_serial.sn = 0xDF;

	memcpy(&pdu_serial_network, &pdu_serial, sizeof(struct pdu_serial_query));
	rtr_pdu_to_network_byte_order(&pdu_serial_network);

	expect_any_count(__wrap_tr_send_all, socket, 1);
	expect_any_count(__wrap_tr_send_all, len, 1);
	expect_any_count(__wrap_tr_send_all, timeout, 1);
	expect_check(__wrap_tr_send_all, pdu, cmp_serial_in_error_pdu, &pdu_serial_network);
	will_return(__wrap_tr_send_all, sizeof(pdu_serial) + sizeof(struct pdu_error));

	ret = rtr_send_error_pdu_from_host(&socket, &pdu_serial, pdu_serial.len, INTERNAL_ERROR, NULL, 0);
	assert_int_equal(ret, RTR_SUCCESS);

	expect_any_count(__wrap_tr_send_all, socket, 1);
	expect_any_count(__wrap_tr_send_all, len, 1);
	expect_any_count(__wrap_tr_send_all, timeout, 1);
	expect_check(__wrap_tr_send_all, pdu, cmp_serial_in_error_pdu, &pdu_serial_network);
	will_return(__wrap_tr_send_all, sizeof(pdu_serial_network) + sizeof(struct pdu_error));

	ret = rtr_send_error_pdu_from_network(&socket, &pdu_serial_network, pdu_serial.len, INTERNAL_ERROR, NULL, 0);
	assert_int_equal(ret, RTR_SUCCESS);

	expect_any_count(__wrap_tr_send_all, socket, 1);
	expect_any_count(__wrap_tr_send_all, len, 1);
	expect_any_count(__wrap_tr_send_all, timeout, 1);
	expect_check(__wrap_tr_send_all, pdu, cmp_header_in_error_pdu, &pdu_serial_network);
	will_return(__wrap_tr_send_all, sizeof(pdu_serial) + sizeof(struct pdu_error));

	ret = rtr_send_error_pdu_from_host(&socket, &pdu_serial, sizeof(struct pdu_header), INTERNAL_ERROR, NULL, 0);
	assert_int_equal(ret, RTR_SUCCESS);

	ret = rtr_send_error_pdu_from_host(&socket, &pdu_serial, 2, INTERNAL_ERROR, NULL, 0);
	assert_int_equal(ret, RTR_ERROR);
}

static void test_rtr_pdu_check_interval(void **state)
{
	UNUSED(state);

	struct rtr_socket rtr_socket;
	struct pdu_end_of_data_v1_v2 pdu_eod;

	int retval;

	rtr_socket.refresh_interval = 0;
	rtr_socket.retry_interval = 0;
	rtr_socket.expire_interval = 0;

	pdu_eod.refresh_interval = 1;
	pdu_eod.retry_interval = 2;
	pdu_eod.expire_interval = 601;

	/* test appliance of interval values to the rtr_socket */
	apply_interval_value(&rtr_socket, pdu_eod.refresh_interval, RTR_INTERVAL_TYPE_REFRESH);
	assert_int_equal(rtr_socket.refresh_interval, pdu_eod.refresh_interval);

	apply_interval_value(&rtr_socket, pdu_eod.retry_interval, RTR_INTERVAL_TYPE_RETRY);
	assert_int_equal(rtr_socket.retry_interval, pdu_eod.retry_interval);

	apply_interval_value(&rtr_socket, pdu_eod.expire_interval, RTR_INTERVAL_TYPE_EXPIRATION);
	assert_int_equal(rtr_socket.expire_interval, pdu_eod.expire_interval);

	/* test checks that determine if value is inside range */
	retval = rtr_check_interval_range(pdu_eod.refresh_interval, RTR_REFRESH_MIN, RTR_REFRESH_MAX);
	assert_int_equal(retval, RTR_INSIDE_INTERVAL_RANGE);

	retval = rtr_check_interval_range(pdu_eod.retry_interval, RTR_RETRY_MIN, RTR_RETRY_MAX);
	assert_int_equal(retval, RTR_INSIDE_INTERVAL_RANGE);

	retval = rtr_check_interval_range(pdu_eod.expire_interval, RTR_EXPIRATION_MIN, RTR_EXPIRATION_MAX);
	assert_int_equal(retval, RTR_INSIDE_INTERVAL_RANGE);

	/* test checks that determine if value is below range */
	pdu_eod.refresh_interval = RTR_REFRESH_MIN - 1;
	pdu_eod.retry_interval = RTR_RETRY_MIN - 1;
	pdu_eod.expire_interval = RTR_EXPIRATION_MIN - 1;

	retval = rtr_check_interval_range(pdu_eod.refresh_interval, RTR_REFRESH_MIN, RTR_REFRESH_MAX);
	assert_int_equal(retval, RTR_BELOW_INTERVAL_RANGE);

	retval = rtr_check_interval_range(pdu_eod.retry_interval, RTR_RETRY_MIN, RTR_RETRY_MAX);
	assert_int_equal(retval, RTR_BELOW_INTERVAL_RANGE);

	retval = rtr_check_interval_range(pdu_eod.expire_interval, RTR_EXPIRATION_MIN, RTR_EXPIRATION_MAX);
	assert_int_equal(retval, RTR_BELOW_INTERVAL_RANGE);

	/* test checks that determine if value is above range */
	pdu_eod.refresh_interval = RTR_REFRESH_MAX + 1;
	pdu_eod.retry_interval = RTR_RETRY_MAX + 1;
	pdu_eod.expire_interval = RTR_EXPIRATION_MAX + 1;

	retval = rtr_check_interval_range(pdu_eod.refresh_interval, RTR_REFRESH_MIN, RTR_REFRESH_MAX);
	assert_int_equal(retval, RTR_ABOVE_INTERVAL_RANGE);

	retval = rtr_check_interval_range(pdu_eod.retry_interval, RTR_RETRY_MIN, RTR_RETRY_MAX);
	assert_int_equal(retval, RTR_ABOVE_INTERVAL_RANGE);

	retval = rtr_check_interval_range(pdu_eod.expire_interval, RTR_EXPIRATION_MIN, RTR_EXPIRATION_MAX);
	assert_int_equal(retval, RTR_ABOVE_INTERVAL_RANGE);

	/* test the different interval options the user can choose */
	rtr_socket.refresh_interval = 0;
	pdu_eod.refresh_interval = 42;
	retval = rtr_check_interval_option(&rtr_socket, RTR_INTERVAL_MODE_ACCEPT_ANY, pdu_eod.refresh_interval,
					   RTR_INTERVAL_TYPE_REFRESH);
	assert_int_equal(retval, RTR_SUCCESS);
	assert_int_equal(rtr_socket.refresh_interval, pdu_eod.refresh_interval);

	rtr_socket.refresh_interval = 0;
	pdu_eod.refresh_interval = RTR_REFRESH_MAX + 1;
	retval = rtr_check_interval_option(&rtr_socket, RTR_INTERVAL_MODE_DEFAULT_MIN_MAX, pdu_eod.refresh_interval,
					   RTR_INTERVAL_TYPE_REFRESH);
	assert_int_equal(retval, RTR_SUCCESS);
	assert_int_equal(rtr_socket.refresh_interval, RTR_REFRESH_MAX);

	rtr_socket.refresh_interval = 0;
	pdu_eod.refresh_interval = RTR_REFRESH_MIN - 1;
	retval = rtr_check_interval_option(&rtr_socket, RTR_INTERVAL_MODE_DEFAULT_MIN_MAX, pdu_eod.refresh_interval,
					   RTR_INTERVAL_TYPE_REFRESH);
	assert_int_equal(retval, RTR_SUCCESS);
	assert_int_equal(rtr_socket.refresh_interval, RTR_REFRESH_MIN);

	rtr_socket.refresh_interval = 42;
	pdu_eod.refresh_interval = RTR_REFRESH_MIN - 1;
	retval = rtr_check_interval_option(&rtr_socket, RTR_INTERVAL_MODE_IGNORE_ON_FAILURE, pdu_eod.refresh_interval,
					   RTR_INTERVAL_TYPE_REFRESH);
	assert_int_equal(retval, RTR_SUCCESS);
	assert_int_equal(rtr_socket.refresh_interval, 42);

	rtr_socket.refresh_interval = 0;
	pdu_eod.refresh_interval = RTR_REFRESH_MAX + 1;
	retval = rtr_check_interval_option(&rtr_socket, RTR_INTERVAL_MODE_ACCEPT_ANY, pdu_eod.refresh_interval,
					   RTR_INTERVAL_TYPE_REFRESH);
	assert_int_equal(retval, RTR_SUCCESS);
	assert_int_equal(rtr_socket.refresh_interval, RTR_REFRESH_MAX + 1);
}

static void test_set_interval_option(void **state)
{
	UNUSED(state);

	struct rtr_socket rtr_socket;

	rtr_set_interval_mode(&rtr_socket, RTR_INTERVAL_MODE_IGNORE_ANY);
	assert_int_equal(rtr_get_interval_mode(&rtr_socket), RTR_INTERVAL_MODE_IGNORE_ANY);

	rtr_set_interval_mode(&rtr_socket, RTR_INTERVAL_MODE_ACCEPT_ANY);
	assert_int_equal(rtr_get_interval_mode(&rtr_socket), RTR_INTERVAL_MODE_ACCEPT_ANY);

	rtr_set_interval_mode(&rtr_socket, RTR_INTERVAL_MODE_DEFAULT_MIN_MAX);
	assert_int_equal(rtr_get_interval_mode(&rtr_socket), RTR_INTERVAL_MODE_DEFAULT_MIN_MAX);

	rtr_set_interval_mode(&rtr_socket, RTR_INTERVAL_MODE_IGNORE_ON_FAILURE);
	assert_int_equal(rtr_get_interval_mode(&rtr_socket), RTR_INTERVAL_MODE_IGNORE_ON_FAILURE);

	rtr_set_interval_mode(&rtr_socket, 4);
	assert(rtr_get_interval_mode(&rtr_socket) != 4);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_set_last_update),		  cmocka_unit_test(test_rtr_get_pdu_type),
		cmocka_unit_test(test_pdu_to_network_byte_order), cmocka_unit_test(test_pdu_to_host_byte_order),
		cmocka_unit_test(test_rtr_pdu_check_size),	cmocka_unit_test(test_rtr_send_error_pdu),
		cmocka_unit_test(test_rtr_pdu_check_interval),    cmocka_unit_test(test_set_interval_option),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
