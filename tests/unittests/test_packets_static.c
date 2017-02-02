/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website; http://rtrlib.realmv6.org/
 */

#include "rtrlib_unittests.h"
#include "rtrlib/rtr/packets.c"
#include "test_packets_static.h"

int __wrap_lrtr_get_monotonic_time(time_t *seconds)
{
	UNUSED(seconds);
	return mock_type(int);
}

int __wrap_tr_send_all(const struct tr_socket *socket,
		       const void *pdu, const size_t len,
		       const time_t timeout)
{
	check_expected(socket);
	check_expected(pdu);
	check_expected(len);
	check_expected(timeout);
	return (int)mock();
}

static int cmp_serial_in_error_pdu(const LargestIntegralType test,
				   const LargestIntegralType reference)
{
	struct pdu_error *test_pdu =
		(struct pdu_error *)test;
	struct pdu_serial_query *reference_pdu =
		(struct pdu_serial_query *)reference;
	struct pdu_serial_query *encapsulated_pdu =
		(struct pdu_serial_query *)test_pdu->rest;

	if (encapsulated_pdu->ver != reference_pdu->ver ||
	    encapsulated_pdu->type != reference_pdu->type ||
	    encapsulated_pdu->session_id != reference_pdu->session_id ||
	    encapsulated_pdu->len != reference_pdu->len ||
	    encapsulated_pdu->sn != reference_pdu->sn) {
		return 0;
	}

	return 1;
}

static int cmp_header_in_error_pdu(const LargestIntegralType test,
				   const LargestIntegralType reference)
{
	struct pdu_error *test_pdu =
		(struct pdu_error *)test;
	struct pdu_serial_query *reference_pdu =
		(struct pdu_serial_query *)reference;
	struct pdu_serial_query *encapsulated_pdu =
		(struct pdu_serial_query *)test_pdu->rest;

	if (encapsulated_pdu->ver != reference_pdu->ver ||
	    encapsulated_pdu->type != reference_pdu->type ||
	    encapsulated_pdu->session_id != reference_pdu->session_id ||
	    encapsulated_pdu->len != reference_pdu->len) {
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
	struct pdu_serial_query pdu;

	UNUSED(state);

	pdu.ver = 0;
	pdu.type = SERIAL_QUERY;
	pdu.session_id = 0x32A5;
	pdu.len = 0xC;
	pdu.sn = 0xCC56E9BB;

	rtr_pdu_to_network_byte_order(&pdu);

	assert_int_equal(pdu.ver, 0);
	assert_int_equal(pdu.type, SERIAL_QUERY);
	assert_int_equal(pdu.session_id, 0xA532);
	assert_int_equal(pdu.len, 0x0C000000);
	assert_int_equal(pdu.sn, 0xBBE956CC);
}

static void test_pdu_to_host_byte_order(void **state)
{
	struct pdu_serial_notify pdu_serial;
	struct pdu_end_of_data_v1 pdu_eod;

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
}

static void test_rtr_pdu_check_size(void **state)
{
	struct pdu_header pdu;
	struct pdu_error *error = malloc(30);

	UNUSED(state);

	memset(error, 0, 30);

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

	/* test error pdu error string termination */
	error->len = 25;
	error->rest[11] = 0x1;
	error->rest[12] = 0x20;
	assert_false(rtr_pdu_check_size((struct pdu_header *)error));
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

	memcpy(&pdu_serial_network,
	       &pdu_serial,
	       sizeof(struct pdu_serial_query));
	rtr_pdu_to_network_byte_order(&pdu_serial_network);

	expect_any_count(__wrap_tr_send_all, socket, 1);
	expect_any_count(__wrap_tr_send_all, len, 1);
	expect_any_count(__wrap_tr_send_all, timeout, 1);
	expect_check(__wrap_tr_send_all,
		     pdu,
		     cmp_serial_in_error_pdu,
		     &pdu_serial_network);
	will_return(__wrap_tr_send_all,
		    sizeof(pdu_serial) + sizeof(struct pdu_error));

	ret = rtr_send_error_pdu_from_host(&socket,
					   &pdu_serial,
					   pdu_serial.len,
					   INTERNAL_ERROR,
					   NULL,
					   0);
	assert_int_equal(ret, RTR_SUCCESS);

	expect_any_count(__wrap_tr_send_all, socket, 1);
	expect_any_count(__wrap_tr_send_all, len, 1);
	expect_any_count(__wrap_tr_send_all, timeout, 1);
	expect_check(__wrap_tr_send_all,
		     pdu,
		     cmp_serial_in_error_pdu,
		     &pdu_serial_network);
	will_return(__wrap_tr_send_all,
		    sizeof(pdu_serial_network) + sizeof(struct pdu_error));

	ret = rtr_send_error_pdu_from_network(&socket,
					      &pdu_serial_network,
					      pdu_serial.len,
					      INTERNAL_ERROR,
					      NULL,
					      0);
	assert_int_equal(ret, RTR_SUCCESS);

	expect_any_count(__wrap_tr_send_all, socket, 1);
	expect_any_count(__wrap_tr_send_all, len, 1);
	expect_any_count(__wrap_tr_send_all, timeout, 1);
	expect_check(__wrap_tr_send_all,
		     pdu,
		     cmp_header_in_error_pdu,
		     &pdu_serial_network);
	will_return(__wrap_tr_send_all,
		    sizeof(pdu_serial) + sizeof(struct pdu_error));

	ret = rtr_send_error_pdu_from_host(&socket,
					   &pdu_serial,
					   sizeof(struct pdu_header),
					   INTERNAL_ERROR,
					   NULL,
					   0);
	assert_int_equal(ret, RTR_SUCCESS);

	ret = rtr_send_error_pdu_from_host(&socket,
					   &pdu_serial,
					   2,
					   INTERNAL_ERROR,
					   NULL,
					   0);
	assert_int_equal(ret, RTR_ERROR);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_set_last_update),
		cmocka_unit_test(test_rtr_get_pdu_type),
		cmocka_unit_test(test_pdu_to_network_byte_order),
		cmocka_unit_test(test_pdu_to_host_byte_order),
		cmocka_unit_test(test_rtr_pdu_check_size),
		cmocka_unit_test(test_rtr_send_error_pdu),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
