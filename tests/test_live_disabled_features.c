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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
 * @brief This is a live validation test for checking that no segmentation faults happen when
 *        PDUs of disabled but supported features are received.
 * This test requires an active network connection. This tests uses a TCP transport connection.
 */
int main(void)
{
	/* These variables are not in the global scope
	 * because it would cause warnings about discarding constness
	 */

	// REPLACE THIS BY YOUR RTR SERVER
	char RPKI_CACHE_HOST[] = "rpki-cache.netd.cs.tu-dresden.de";
	char RPKI_CACHE_PORT[] = "3323"; // rir rtr

	/* create a TCP transport socket */
	struct rtr_tr_socket tr_tcp;
	struct rtr_tr_tcp_config tcp_config = {RPKI_CACHE_HOST, RPKI_CACHE_PORT, NULL, NULL, NULL, 0};
	struct rtr_socket rtr_tcp;
	struct rtr_mgr_group groups[1];

	/* init a TCP transport and create rtr socket */
	rtr_tr_tcp_init(&tcp_config, &tr_tcp);
	rtr_tcp.tr_socket = &tr_tcp;

	/* create a rtr_mgr_group array with 1 element */
	groups[0].sockets = malloc(1 * sizeof(struct rtr_socket *));
	groups[0].sockets_len = 1;
	groups[0].sockets[0] = &rtr_tcp;
	groups[0].preference = 1;

	struct rtr_mgr_config *conf;

	if (rtr_mgr_init(&conf, groups, 1, &connection_status_callback, NULL, NULL, NULL) < 0)
		return EXIT_FAILURE;

	rtr_mgr_setup_sockets(conf, groups, 1, 50, 600, 600);
	rtr_mgr_start(conf);
	int sleep_counter = 0;

	/* wait for connection, or timeout and exit eventually */
	while (!rtr_mgr_conf_in_sync(conf)) {
		if (connection_status == RTR_MGR_ERROR) {
			return EXIT_FAILURE;
		}

		sleep(1);
		sleep_counter++;
		if (sleep_counter >= connection_timeout) {
			return EXIT_FAILURE;
		}
	}

	rtr_mgr_stop(conf);
	rtr_mgr_free(conf);

	return EXIT_SUCCESS;
}
