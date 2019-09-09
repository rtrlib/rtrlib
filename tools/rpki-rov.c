/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "rtrlib/rtrlib.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const int connection_timeout = 20;
enum rtr_mgr_status connection_status = -1;

static void connection_status_callback(const struct rtr_mgr_group *group, enum rtr_mgr_status status,
				       const struct rtr_socket *socket, void *data)
{
	if (status == RTR_MGR_ERROR)
		connection_status = status;
}

static int connection_error(enum rtr_mgr_status status)
{
	if (status == RTR_MGR_ERROR) {
		/*
		 * Wait for input before printing error to avoid "broken pipe" error
		 * while communicating with the Python program.
		 */
		char input[256];

		if (fgets(input, 256, stdin))
			;
		printf("error\n");
		fflush(stdout);
		return 1;
	}
	return 0;
}

static int str_to_int(const char *str, int *value)
{
	errno = 0;
	int tmp = strtol(str, NULL, 10);

	if (errno != 0)
		return 1;

	*value = tmp;
	return 0;
}

int main(int argc, char *argv[])
{
	/* check arguments, need hostname/IP and port of cache-server */
	if (argc < 3) {
		printf("Usage: %s [host] [port]\n", argv[0]);
		return EXIT_FAILURE;
	}

	struct tr_socket tr_tcp;
	struct tr_tcp_config tcp_config = {argv[1], argv[2], NULL};
	struct rtr_socket rtr_tcp;
	struct rtr_mgr_config *conf;
	struct rtr_mgr_group groups[1];

	/* init a TCP transport and create rtr socket */
	tr_tcp_init(&tcp_config, &tr_tcp);
	rtr_tcp.tr_socket = &tr_tcp;

	/* create a rtr_mgr_group array with 1 element */
	groups[0].sockets = malloc(1 * sizeof(struct rtr_socket *));
	groups[0].sockets_len = 1;
	groups[0].sockets[0] = &rtr_tcp;
	groups[0].preference = 1;

	if (rtr_mgr_init(&conf, groups, 1, 30, 600, 600, NULL, NULL, &connection_status_callback, NULL) < 0)
		return EXIT_FAILURE;

	rtr_mgr_start(conf);

	char input[256];
	int sleep_counter = 0;

	/* wait till at least one rtr_mgr_group is synchronized with server */
	while (!rtr_mgr_conf_in_sync(conf)) {
		if (connection_error(connection_status))
			return EXIT_FAILURE;

		sleep(1);
		sleep_counter++;
		if (sleep_counter >= connection_timeout) {
			/*
			 * Wait for input before printing "timeout",
			 * to avoid "broken pipee error while communicating
			 * with the Python program
			 */
			if (fgets(input, 256, stdin))
				;
			printf("timeout\n");
			fflush(stdout);
			return EXIT_FAILURE;
		}
	}

	int counter;
	/* loop for input */
	while (1) {
		int input_len;
		int spaces;

		/* recheck connection, exit on failure */
		if (connection_error(connection_status))
			return EXIT_FAILURE;

		/* try reading from stdin, exit on failure */
		if (!fgets(input, 256, stdin)) {
			printf("input error\n");
			return EXIT_FAILURE;
		}

		/* remove newline, if present */
		input_len = strlen(input) - 1;
		if (input[input_len] == '\n')
			input[input_len] = '\0';

		/* check if there are exactly 3 arguments */
		spaces = 0;
		for (counter = 0; counter < input_len; counter++) {
			if (input[counter] == ' ' && input[counter + 1] != ' ' && input[counter + 1] != '\0' &&
			    counter != 0)
				spaces++;
		}

		/* check input matching pattern */
		if (spaces != 2) {
			printf("Arguments required: IP Mask ASN\n");
			fflush(stdout);
			continue;
		}

		char delims[] = " ";
		char *input_tok = NULL;

		input_tok = strtok(input, delims);
		struct lrtr_ip_addr pref;
		char ip[INET6_ADDRSTRLEN];

		if (strlen(input_tok) > sizeof(ip) - 1) {
			fprintf(stderr, "Error: Invalid ip addr\n");
			continue;
		}

		memset(ip, 0, sizeof(ip));
		strncpy(ip, input_tok, sizeof(ip) - 1);

		if (lrtr_ip_str_to_addr(ip, &pref) != 0) {
			fprintf(stderr, "Error: Invalid ip addr\n");
			continue;
		}

		input_tok = strtok(NULL, delims);
		int mask;

		if (str_to_int(input_tok, &mask)) {
			fprintf(stderr, "Error: Invalid mask\n");
			continue;
		}

		input_tok = strtok(NULL, delims);
		int asn;

		if (str_to_int(input_tok, &asn)) {
			fprintf(stderr, "Error: Invalid asn\n");
			continue;
		}

		enum pfxv_state result;
		struct pfx_record *reason = NULL;
		unsigned int reason_len = 0;

		/* do validation */
		pfx_table_validate_r(groups[0].sockets[0]->pfx_table, &reason, &reason_len, asn, &pref, mask, &result);

		int validity_code = -1;
		/* translate validation result */
		if (result == BGP_PFXV_STATE_VALID)
			validity_code = 0;
		else if (result == BGP_PFXV_STATE_NOT_FOUND)
			validity_code = 1;
		else if (result == BGP_PFXV_STATE_INVALID)
			validity_code = 2;

		/* IP Mask BGP-ASN| */
		printf("%s %d %d|", ip, mask, asn);

		/* ROA-ASN IP MaskMin MaskMax, ... */
		if (reason && (reason_len > 0)) {
			unsigned int i;

			for (i = 0; i < reason_len; i++) {
				char tmp[100];

				lrtr_ip_addr_to_str(&reason[i].prefix, tmp, sizeof(tmp));
				printf("%u %s %u %u", reason[i].asn, tmp, reason[i].min_len, reason[i].max_len);
				if ((i + 1) < reason_len)
					printf(",");
			}
		}

		/* |validity_code */
		printf("|%d", validity_code);

		printf("\n");
		fflush(stdout);
	}

	rtr_mgr_stop(conf);
	rtr_mgr_free(conf);
	free(groups[0].sockets);

	return EXIT_SUCCESS;
}
