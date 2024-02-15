/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "rtrlib/rtrlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct test_validity_query {
	const char *pfx;
	int len;
	int asn;
	unsigned int val;
};

/*
 * Verification is based on ROAs for RIPE RIS Routing Beacons, see:
 * (https://www.ripe.net/analyse/internet-measurements/
 *  routing-information-service-ris/current-ris-routing-beacons)
 */
const struct test_validity_query queries[] = {{"93.175.146.0", 24, 12654, BGP_PFXV_STATE_VALID},
					      {"2001:7fb:fd02::", 48, 12654, BGP_PFXV_STATE_VALID},
					      {"93.175.147.0", 24, 12654, BGP_PFXV_STATE_INVALID},
					      {"2001:7fb:fd03::", 48, 12654, BGP_PFXV_STATE_INVALID},
					      {"84.205.83.0", 24, 12654, BGP_PFXV_STATE_NOT_FOUND},
					      {"2001:7fb:ff03::", 48, 12654, BGP_PFXV_STATE_NOT_FOUND},
					      {NULL, 0, 0, 0} };

const int connection_timeout = 100;
enum rtr_mgr_status connection_status = -1;

static void connection_status_callback(const struct rtr_mgr_group *group __attribute__((unused)),
				       enum rtr_mgr_status status,
				       const struct rtr_socket *socket __attribute__((unused)),
				       void *data __attribute__((unused)))
{
	if (status == RTR_MGR_ERROR)
		connection_status = status;
}

/**
 * @brief live prefix validation test
 * This test requires an active network connection. It runs an on-line live
 * validation of specific IP prefixes, i.e., RIPE BGP beacons, that have known
 * RPKI validation states. This tests uses a TCP transport connection.
 */
int main(void)
{
	/* These variables are not in the global scope
	 * because it would cause warnings about discarding constness
	 */
	char RPKI_CACHE_HOST[] = "rpki-cache.netd.cs.tu-dresden.de";
	char RPKI_CACHE_PORT[] = "3323";

	/* create a TCP transport socket */
	struct tr_socket tr_tcp;
	struct tr_tcp_config tcp_config = {RPKI_CACHE_HOST, RPKI_CACHE_PORT, NULL, NULL, NULL, 0};
	struct rtr_socket rtr_tcp;
	struct rtr_mgr_group groups[1];

	/* init a TCP transport and create rtr socket */
	tr_tcp_init(&tcp_config, &tr_tcp);

	return EXIT_SUCCESS;
}
