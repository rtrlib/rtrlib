#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "rtrlib/rtrlib.h"

enum rtr_mgr_status connectionStatus = -1;
static void connectionStatusCallback(struct rtr_mgr_group *group, enum rtr_mgr_status status) {
	if(status == RTR_MGR_ERROR) {
		connectionStatus = status;
	}
}

int connectionError(enum rtr_mgr_status status) {
	if(status == RTR_MGR_ERROR) {
		// Wait for input before printing error to avoid "broken pipe" error while communicating
		// with the Python program.
		char input[256];
		int inputLength;
		fgets(input, 256, stdin);
		
		printf("error\n");
		fflush(stdout);
		return 1;
	} else {
		return 0;
	}
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Usage: %s [host] [port]\n", argv[0]);
		return 0;
	}

	// Create a TCP transport socket
	struct tr_socket tr_tcp;
	struct tr_tcp_config tcp_config = { argv[1], argv[2], NULL };
	tr_tcp_init(&tcp_config, &tr_tcp);

	// Create an rtr_socket and associate it with the transport socket
	struct rtr_socket rtr_tcp;
	rtr_tcp.tr_socket = &tr_tcp;
	struct rtr_mgr_config *conf;

	// Create a rtr_mgr_group array with 1 element
	struct rtr_mgr_group groups[1];

	groups[0].sockets = malloc(1 * sizeof(struct rtr_socket*));
	groups[0].sockets_len = 1;
	groups[0].sockets[0] = &rtr_tcp;
	groups[0].preference = 1;

	conf = rtr_mgr_init(groups, 1, 
                            30, 520, 
                            connectionStatusCallback, 
                            NULL, 
                            NULL, 
                            NULL);
	rtr_mgr_start(conf);

	char input[256];
	int inputLength;
	
	int sleepCounter = 0;
	int timeout = 20;
	// Wait till at least one rtr_mgr_group is fully synchronized with the server
	while (!rtr_mgr_conf_in_sync(conf)) {
		if(connectionError(connectionStatus)) {
			return 0;
		}
		sleep(1);
		sleepCounter++;
		if (sleepCounter >= timeout) {
			// Wait for input before printing "timeout" to avoid "broken pipe" error while communicating
			// with the Python program.
			fgets(input, 256, stdin);
			printf("timeout\n");
			fflush(stdout);
			return 0;
		}
	}

	char ip[128];
	int mask;
	int asn;
	int counter;
	int spaces;
	while (1) {
		if(connectionError(connectionStatus)) {
			return 0;
		}

		fgets(input, 256, stdin);
		// Remove newline, if present
		inputLength = strlen(input) - 1;
		if (input[inputLength] == '\n')
			input[inputLength] = '\0';

		// Check if there are exactly 3 arguments
		spaces = 0;
		for (counter = 0; counter < inputLength; counter++) {
			if (input[counter] == ' ' && input[counter + 1] != ' '
					&& input[counter + 1] != '\0' && counter != 0) {
				spaces++;
			}
		}

		if (spaces != 2) {
			printf("Arguments required: IP Mask ASN\n");
			fflush(stdout);
			continue;
		}

		char delims[] = " ";
		char *inputToken = NULL;
		inputToken = strtok(input, delims);
		strcpy(ip, inputToken);

		inputToken = strtok(NULL, delims);
		mask = atoi(inputToken);

		inputToken = strtok(NULL, delims);
		asn = atoi(inputToken);

		struct ip_addr pref;
		ip_str_to_addr(ip, &pref);
		enum pfxv_state result;
		struct pfx_record* reason = NULL;
		unsigned int reason_len = 0;

		pfx_table_validate_r(groups[0].sockets[0]->pfx_table, &reason,
				&reason_len, asn, &pref, mask, &result);
		int validityNr = 0;

		if (result == BGP_PFXV_STATE_NOT_FOUND) {
			validityNr = -1;
		} else if (result == BGP_PFXV_STATE_INVALID) {
			validityNr = 0;
		} else if (result == BGP_PFXV_STATE_VALID) {
			validityNr = 1;
		}

		// IP Mask BGP-ASN|
		printf("%s %d %d|", ip, mask, asn);

		// ROA-ASN IP MaskMin MaskMax, ..
		if (reason != NULL && reason_len > 0) {
			unsigned int i;
			for (i = 0; i < reason_len; i++) {
				char tmp[100];
				ip_addr_to_str(&(reason[i].prefix), tmp, sizeof(tmp));
				printf("%u %s %u %u", reason[i].asn, tmp, reason[i].min_len,
						reason[i].max_len);
				if ((i + 1) < reason_len)
					printf(",");
			}
		}

		// |ValidityNr
		printf("|%d", validityNr);

		printf("\n");
		fflush(stdout);
	}

	rtr_mgr_stop(conf);
	rtr_mgr_free(conf);
	free(groups[0].sockets);
}
