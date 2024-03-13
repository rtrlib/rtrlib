/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "rtrlib/aspa/aspa_private.h"
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
const int connection_timeout = 20;
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

	//char RPKI_CACHE_HOST[] = "rpki-validator.realmv6.org";
	//char RPKI_CACHE_PORT[] = "8283";

	// REPLACE THIS BY YOUR RTR SERVER
	char RPKI_CACHE_HOST[] = "rtrlab.tanneberger.me";
	char RPKI_CACHE_PORT[] = "3325"; // rir rtr
	//char RPKI_CACHE_PORT[] = "3324"; // aspa rtr

	/* create a TCP transport socket */
	struct tr_socket tr_tcp;
	struct tr_tcp_config tcp_config = {RPKI_CACHE_HOST, RPKI_CACHE_PORT, NULL, NULL, NULL, 0};
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

	if (rtr_mgr_init(&conf, groups, 1, 30, 600, 600, NULL, NULL, NULL, &connection_status_callback, NULL) < 0)
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

	/* printing all fetched objects */
	struct aspa_array *array = ((*groups[0].sockets)->aspa_table->store)->aspa_array;

	printf("ASPA:\n");
	for (uint32_t i = 0; i < array->size; i++) {
		printf("CAS %u => [ ", array->data[i].customer_asn);
		for (uint32_t j = 0; j < array->data[i].provider_count; j++) {
			printf("%u", array->data[i].provider_asns[j]);
			if (j < array->data[i].provider_count - 1)
				printf(", ");
		}
		printf(" ]\n");
	}

	rtr_mgr_stop(conf);
	rtr_mgr_free(conf);

	return EXIT_SUCCESS;
}
