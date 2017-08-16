#include <stdio.h>
#include <stdlib.h>
#include "rtrlib/rtrlib.h"

int main(){
	//create a TCP transport socket
	struct tr_socket tr_tcp;
	char tcp_host[] = "rpki-validator.realmv6.org";
	char tcp_port[] = "8282";

	struct tr_tcp_config tcp_config = {
	tcp_host, //IP
	tcp_port, //Port
	NULL      //Source address
	};
	tr_tcp_init(&tcp_config, &tr_tcp);
	struct rtr_socket rtr_tcp;
	rtr_tcp.tr_socket = &tr_tcp;
	struct rtr_mgr_group groups[1];

	groups[0].sockets = malloc(sizeof(struct rtr_socket*));
	groups[0].sockets_len = 1;
	groups[0].sockets[0] = &rtr_tcp;
	groups[0].preference = 1;

	struct tr_socket tr_tcp2;
	struct rtr_socket rtr_tcp2;
	struct rtr_mgr_group group2;

	tr_tcp_init(&tcp_config, &tr_tcp2);
	rtr_tcp2.tr_socket = &tr_tcp2;
	group2.sockets = malloc(sizeof(struct rtr_socket*));
	group2.sockets_len = 1;
	group2.sockets[0] = &rtr_tcp2;
	group2.preference = 2;


	struct rtr_mgr_config_ll *conf;
	rtr_mgr_init(&conf, groups, 1, 30, 600, 600, NULL, NULL, NULL, NULL);

	//start the connection manager
	rtr_mgr_start(conf);

	//wait till at least one rtr_mgr_group is fully synchronized with the server
	while(!rtr_mgr_conf_in_sync(conf)) {
	sleep(1);
	}

	rtr_mgr_add_group(conf, &group2);

	tommy_node* node = tommy_list_head(&conf->groups);
	struct rtr_mgr_group_node* group_node = node->data;
	struct rtr_mgr_group_node* group_node2 = node->next->data;

	assert(group_node->group->preference == 1);
	assert(group_node2->group->preference == 2);

	rtr_mgr_remove_group(conf, 1);

	node = tommy_list_head(&conf->groups);
	group_node = node->data;
	assert(group_node->group->preference == 2);


	rtr_mgr_stop(conf);
	rtr_mgr_free(conf);
	//free(groups[0].sockets);
	//free(groups[1].sockets);
}
