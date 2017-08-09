#include <stdio.h>
#include <stdlib.h>
#include "rtrlib/rtrlib.h"

int main(void)
	{
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

	groups[0].sockets = malloc(sizeof(struct rtr_socket *));
	groups[0].sockets_len = 1;
	groups[0].sockets[0] = &rtr_tcp;
	groups[0].preference = 1;

	struct tr_socket tr_tcp2;
	struct rtr_socket rtr_tcp2;
	struct rtr_mgr_group group2;

	tr_tcp_init(&tcp_config, &tr_tcp2);
	rtr_tcp2.tr_socket = &tr_tcp2;
	group2.sockets = malloc(sizeof(struct rtr_socket *));
	group2.sockets_len = 1;
	group2.sockets[0] = &rtr_tcp2;
	group2.preference = 2;

	struct tr_socket tr_tcp3;
	struct rtr_socket rtr_tcp3;
	struct rtr_mgr_group group3;

	tr_tcp_init(&tcp_config, &tr_tcp3);
	rtr_tcp3.tr_socket = &tr_tcp3;
	group3.sockets = malloc(sizeof(struct rtr_socket *));
	group3.sockets_len = 1;
	group3.sockets[0] = &rtr_tcp3;
	group3.preference = 2;

	struct rtr_mgr_config *conf;

	rtr_mgr_init(&conf, groups, 1, 30, 600, 600, NULL, NULL, NULL, NULL);

	//start the connection manager
	rtr_mgr_start(conf);

	//wait till at least one group is fully synchronized with the server
	while (!rtr_mgr_conf_in_sync(conf))
		sleep(1);

	assert(conf->len == 1);

	rtr_mgr_add_group(conf, &group2);

	tommy_node *node = tommy_list_head(&conf->groups);
	struct rtr_mgr_group_node *group_node = node->data;
	struct rtr_mgr_group_node *group_node2 = node->next->data;

	assert(group_node->group->preference == 1);
	assert(group_node2->group->preference == 2);
	assert(conf->len == 2);

	rtr_mgr_remove_group(conf, 1);

	node = tommy_list_head(&conf->groups);
	group_node = node->data;
	assert(group_node->group->preference == 2);
	assert(conf->len == 1);

	//checking behavior in case the group preference already exists.
	rtr_mgr_add_group(conf, &group3);
	group3.preference = 3;
	rtr_mgr_add_group(conf, &group3);
	
	//remove non-existent group
	rtr_mgr_remove_group(conf, 3);
	rtr_mgr_remove_group(conf, 3);

	//add 100 groups
	for (int i = 0; i < 100; i++) {
		rtr_mgr_add_group(conf, &group3);
		group3.preference++;
	}
	
	//remove 100 groups
	for (int i = 0; i < 100; i++) {
		rtr_mgr_remove_group(conf, --group3.preference);
	}

	rtr_mgr_stop(conf);
	rtr_mgr_free(conf);
	free(group2.sockets);
	free(groups[0].sockets);
}
