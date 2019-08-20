#include "rtrlib/rtrlib.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	//create a SSH transport socket
	char ssh_host[] = "123.231.123.221";
	char ssh_user[] = "rpki_user";
	char ssh_hostkey[] = "/etc/rpki-rtr/hostkey";
	char ssh_privkey[] = "/etc/rpki-rtr/client.priv";
	struct tr_socket tr_ssh;
	struct tr_ssh_config config = {
		ssh_host, //IP
		22, //Port
		NULL, //Source address
		ssh_user,
		ssh_hostkey, //Server hostkey
		ssh_privkey, //Private key
	};
	tr_ssh_init(&config, &tr_ssh);

	//create a TCP transport socket
	struct tr_socket tr_tcp;
	char tcp_host[] = "rpki-validator.realmv6.org";
	char tcp_port[] = "8282";

	struct tr_tcp_config tcp_config = {
		tcp_host, //IP
		tcp_port, //Port
		NULL //Source address
	};
	tr_tcp_init(&tcp_config, &tr_tcp);

	//create 3 rtr_sockets and associate them with the transprort sockets
	struct rtr_socket rtr_ssh, rtr_tcp;
	rtr_ssh.tr_socket = &tr_ssh;
	rtr_tcp.tr_socket = &tr_tcp;

	//create a rtr_mgr_group array with 2 elements
	struct rtr_mgr_group groups[2];

	//The first group contains both TCP RTR sockets
	groups[0].sockets = malloc(sizeof(struct rtr_socket *));
	groups[0].sockets_len = 1;
	groups[0].sockets[0] = &rtr_tcp;
	groups[0].preference = 1; //Preference value of this group

	//The seconds group contains only the SSH RTR socket
	groups[1].sockets = malloc(1 * sizeof(struct rtr_socket *));
	groups[1].sockets_len = 1;
	groups[1].sockets[0] = &rtr_ssh;
	groups[1].preference = 2;

	//create a rtr_mgr_config struct that stores the group

	//initialize all rtr_sockets in the server pool with the same settings
	struct rtr_mgr_config *conf;
	rtr_mgr_init(&conf, groups, 2, 30, 600, 600, NULL, NULL, NULL, NULL);

	//start the connection manager
	rtr_mgr_start(conf);

	//wait till at least one rtr_mgr_group is fully synchronized with the server
	while (!rtr_mgr_conf_in_sync(conf)) {
		sleep(1);
	}

	//validate the BGP-Route 10.10.0.0/24, origin ASN: 12345
	struct lrtr_ip_addr pref;
	lrtr_ip_str_to_addr("10.10.0.0", &pref);
	enum pfxv_state result;
	const uint8_t mask = 24;
	rtr_mgr_validate(conf, 12345, &pref, mask, &result);

	//output the result of the prefix validation above
	//to showcase the returned states.
	char buffer[INET_ADDRSTRLEN];
	lrtr_ip_addr_to_str(&pref, buffer, sizeof(buffer));

	printf("RESULT: The prefix %s/%i ", buffer, mask);
	switch (result) {
	case BGP_PFXV_STATE_VALID:
		printf("is valid.\n");
		break;
	case BGP_PFXV_STATE_INVALID:
		printf("is invalid.\n");
		break;
	case BGP_PFXV_STATE_NOT_FOUND:
		printf("was not found.\n");
		break;
	default:
		break;
	}

	rtr_mgr_stop(conf);
	rtr_mgr_free(conf);
	free(groups[0].sockets);
	free(groups[1].sockets);
}
