#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rtrlib/rtrlib.h"

//int validatePrefix(char* ip, int mask, int asn){
int main(int argc, char *argv[]) {
	if(argc<3) {
		printf("Usage: %s [host] [port]\n", argv[0]);
		return 0;
	}
	//create a TCP transport socket
	tr_socket tr_tcp1;
	tr_tcp_config tcp_config1 = { argv[1], //IP, old: "rpki.realmv6.org"
			argv[2] //Port, old: "42420"
			};
	tr_tcp_init(&tcp_config1, &tr_tcp1);

	//create 3 rtr_sockets and associate them with the transprort sockets
	rtr_socket rtr_tcp1;
	rtr_tcp1.tr_socket = &tr_tcp1;

	//create a rtr_mgr_group array with 1 element
	rtr_mgr_group groups[1];
	//The first group contains both TCP RTR sockets
	groups[0].sockets = malloc(1 * sizeof(rtr_socket*));
	groups[0].sockets_len = 1;
	groups[0].sockets[0] = &rtr_tcp1;
	groups[0].preference = 1; //Preference value of this group
	//create a rtr_mgr_config struct that stores the group
	rtr_mgr_config conf;
	conf.groups = groups;
	conf.len = 1; //1 element in the groups array
	//initialize all rtr_sockets in the server pool with the same settings
	rtr_mgr_init(&conf, 240, 520, NULL);
	//start the connection manager
	rtr_mgr_start(&conf);
	//wait till at least one rtr_mgr_group is fully synchronized with the server
	while (!rtr_mgr_conf_in_sync(&conf))
		sleep(1);

	char ip[128];
	int mask;
	int asn;
	int counter;
	int spaces;

	while (1) {
		char input[256];
		int inputLength;

		fgets(input, 256, stdin);
		// Remove newline, if present
		inputLength = strlen(input) - 1;
		if (input[inputLength] == '\n')
			input[inputLength] = '\0';


		// Check if there are exactly 3 arguments
		spaces = 0;
		for (counter = 0; counter < inputLength; counter++) {
			if (input[counter] == ' '
					&& input[counter+1] != ' '
					&& input[counter+1] != '\0'
					&& counter != 0) {
				spaces++;
			}
		}

		if (spaces != 2) {
			break;
			printf("Arguments required: IP, Mask, ASN, %d\n", spaces);
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

		//validate the BGP-Route ip/mask, origin ASN: asn
		ip_addr pref;
		ip_str_to_addr(ip, &pref);
		pfxv_state result;
		pfx_record* reason = NULL;
		unsigned int reason_len = 0;
		//rtr_mgr_validate(&conf, asn, &pref, mask, &result);
		pfx_table_validate_r(conf.groups[0].sockets[0]->pfx_table, &reason,
				&reason_len, asn, &pref, mask, &result);
		int validityNr = 0;

		if (result == BGP_PFXV_STATE_NOT_FOUND) {
			validityNr = -1;
		} else if (result == BGP_PFXV_STATE_INVALID) {
			validityNr = 0;
		} else if (result == BGP_PFXV_STATE_VALID) {
			validityNr = 1;
		}
		
		// |ValidityNr
		printf("%d|", validityNr);
		
		// IP Mask BGP-ASN|
		printf("%s %d %d", ip, mask, asn);
		
		if(validityNr>=0)
			printf("|");

		// ROA-ASN IP MaskMin MaskMax, ..
		if (reason != NULL && reason_len > 0) {
			unsigned int i;
			for (i = 0; i < reason_len; i++) {
				char tmp[100];
				ip_addr_to_str(&(reason[i].prefix), tmp, sizeof(tmp));
				printf("%u %s %u %u", reason[i].asn, tmp,
						reason[i].min_len, reason[i].max_len);
				if ((i + 1) < reason_len)
					printf(",");
			}
		}

		

		printf("\n");
		fflush(stdout);
	}
	rtr_mgr_stop(&conf);
	rtr_mgr_free(&conf);
	free(groups[0].sockets);
}
