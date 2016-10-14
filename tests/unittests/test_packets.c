/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "rtrlib/rtr/packets.h"

void rtr_change_socket_state(struct rtr_socket *rtr_socket, const enum rtr_socket_state new_state)
{
	;
}


int __wrap_tr_send_all(const struct tr_socket *socket,
                       const void *pdu, const size_t len,
                       const time_t timeout)
{
	return (int) mock();
}


void test_rtr_send_reset_query(void **state)
{
	struct rtr_socket socket;

	will_return(__wrap_tr_send_all, 0);
	assert_int_equal(rtr_send_reset_query(&socket), RTR_ERROR);

	will_return(__wrap_tr_send_all, 10);
	assert_int_equal(rtr_send_reset_query(&socket), RTR_SUCCESS);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_rtr_send_reset_query),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
