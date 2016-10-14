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
#include "rtrlib/rtr/packets.c"

int __wrap_lrtr_get_monotonic_time(time_t *seconds)
{
	return mock_type(int);
}

void test_set_last_update(void **state)
{
	struct rtr_socket socket;

	socket.connection_state_fp = NULL;

	will_return(__wrap_lrtr_get_monotonic_time, RTR_ERROR);
	assert_int_equal(rtr_set_last_update(&socket), RTR_ERROR);

	will_return(__wrap_lrtr_get_monotonic_time, RTR_SUCCESS);
	assert_int_equal(rtr_set_last_update(&socket), RTR_SUCCESS);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_set_last_update),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
