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

#define RPKI_CACHE_HOST "rpki-validator.realmv6.org"
#define RPKI_CACHE_POST "8283"

struct test_validity_query {
	char *pfx;
	int len;
	int asn;
	int val;
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

const int connection_timeout = 20;
enum rtr_mgr_status connection_status = -1;

static void connection_status_callback(const struct rtr_mgr_group *group, enum rtr_mgr_status status,
				       const struct rtr_socket *socket, void *data)
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
	/* create a TCP transport socket */
	struct tr_socket tr_tcp;
	struct tr_tcp_config tcp_config = {RPKI_CACHE_HOST, RPKI_CACHE_POST, NULL};
	struct rtr_socket rtr_tcp;
	struct rtr_mgr_group groups[1];

	/* init a TCP transport and create rtr socket */
	tr_tcp_init(&tcp_config, &tr_tcp);
	rtr_tcp.tr_socket = &tr_tcp;

	/* create a rtr_mgr_group array with 1 element */
	groups[0].sockets = malloc(1 * sizeof(struct rtr_socket *));
	groups[0].sockets_len = 1;
	groups[0].sockets[0] = &rtr_tcp;
	groups[0].preference = 1;

	struct rtr_mgr_config *conf;

	if (rtr_mgr_init(&conf, groups, 1, 30, 600, 600, NULL, NULL, &connection_status_callback, NULL) < 0)
		return EXIT_FAILURE;

	rtr_mgr_start(conf);
	int sleep_counter = 0;
	/* wait for connection, or timeout and exit eventually */
	while (!rtr_mgr_conf_in_sync(conf)) {
		if (connection_status == RTR_MGR_ERROR)
			return EXIT_FAILURE;

		sleep(1);
		sleep_counter++;
		if (sleep_counter >= connection_timeout)
			return EXIT_FAILURE;
	}

	int i = 0;
	struct test_validity_query q = queries[i];
	/* test validity of entries in queries[] */
	while (q.pfx) {
		struct lrtr_ip_addr pref;
		enum pfxv_state result;
		struct pfx_record *reason = NULL;
		unsigned int reason_len = 0;

		lrtr_ip_str_to_addr(q.pfx, &pref);
		pfx_table_validate_r(groups[0].sockets[0]->pfx_table, &reason, &reason_len, q.asn, &pref, q.len,
				     &result);
		if (result != q.val) {
			printf("ERROR: prefix validation mismatch.\n");
			return EXIT_FAILURE;
		}
		printf("%s/%d	\tOK\n", q.pfx, q.len);
		q = queries[++i];
	}

	rtr_mgr_stop(conf);
	rtr_mgr_free(conf);

	return EXIT_SUCCESS;
}
