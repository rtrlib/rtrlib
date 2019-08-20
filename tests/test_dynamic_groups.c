#include "rtrlib/rtr_mgr_private.h"
#include "rtrlib/rtrlib.h"

#include "third-party/tommyds/tommylist.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const int connection_timeout = 20;
enum rtr_mgr_status connection_status = -1;

static void connection_status_callback(const struct rtr_mgr_group *group, enum rtr_mgr_status status,
				       const struct rtr_socket *socket, void *data)
{
	connection_status = status;
}

int main(void)
{
	//create a TCP transport socket
	int retval = 0;
	struct tr_socket tr_tcp;
	char tcp_host[] = "rpki-validator.realmv6.org";
	char tcp_port[] = "8283";

	struct tr_tcp_config tcp_config = {
		tcp_host, //IP
		tcp_port, //Port
		NULL //Source address
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

	struct rtr_mgr_config *conf;

	rtr_mgr_init(&conf, groups, 1, 30, 600, 600, NULL, NULL, &connection_status_callback, NULL);

	//start the connection manager
	rtr_mgr_start(conf);

	int sleep_counter = 0;
	// wait 20 sec till at least one group is fully synchronized with the server
	// otherwise EXIT_FAILURE.
	while (!rtr_mgr_conf_in_sync(conf)) {
		sleep_counter++;
		if (connection_status == RTR_MGR_ERROR || sleep_counter > connection_timeout)
			return EXIT_FAILURE;

		sleep(1);
	}

	assert(conf->len == 1);

	retval = rtr_mgr_add_group(conf, &group2);
	assert(retval == RTR_SUCCESS);

	//checking behavior in case the group preference already exists
	//by adding the same group twice.
	retval = rtr_mgr_add_group(conf, &group2);
	assert(retval == RTR_INVALID_PARAM);

	tommy_node *node = tommy_list_head(&conf->groups->list);
	struct rtr_mgr_group_node *group_node = node->data;
	struct rtr_mgr_group_node *group_node2 = node->next->data;

	assert(group_node->group->preference == 1);
	assert(group_node2->group->preference == 2);
	assert(conf->len == 2);

	rtr_mgr_remove_group(conf, 1);

	node = tommy_list_head(&conf->groups->list);
	group_node = node->data;
	assert(group_node->group->preference == 2);
	assert(conf->len == 1);

	struct tr_socket tr_tcp3;
	struct rtr_socket rtr_tcp3;
	struct rtr_mgr_group group3;

	tr_tcp_init(&tcp_config, &tr_tcp3);
	rtr_tcp3.tr_socket = &tr_tcp3;
	group3.sockets = malloc(sizeof(struct rtr_socket *));
	group3.sockets_len = 1;
	group3.sockets[0] = &rtr_tcp3;
	group3.preference = 3;

	struct tr_socket tr_tcp4;
	struct rtr_socket rtr_tcp4;
	struct rtr_mgr_group group4;

	tr_tcp_init(&tcp_config, &tr_tcp4);
	rtr_tcp4.tr_socket = &tr_tcp4;
	group4.sockets = malloc(sizeof(struct rtr_socket *));
	group4.sockets_len = 1;
	group4.sockets[0] = &rtr_tcp4;
	group4.preference = 4;

	rtr_mgr_add_group(conf, &group4);

	// remove group 2 so group 4 becomes the active group.
	rtr_mgr_remove_group(conf, 2);

	// add group 3 which has a higher preference than group 4
	// and check whether it will be set as the active group.
	rtr_mgr_add_group(conf, &group3);

	node = tommy_list_head(&conf->groups->list);
	group_node = node->data;
	assert(group_node->group->preference == 3);

	//try to remove non-existent group
	retval = rtr_mgr_remove_group(conf, 10);
	assert(retval == RTR_ERROR);

	struct tr_socket tr_tcp5;
	struct rtr_socket rtr_tcp5;
	struct rtr_mgr_group group5;

	tr_tcp_init(&tcp_config, &tr_tcp5);
	rtr_tcp5.tr_socket = &tr_tcp5;
	group5.sockets = malloc(sizeof(struct rtr_socket *));
	group5.sockets_len = 1;
	group5.sockets[0] = &rtr_tcp5;
	group5.preference = 5;

	//add 100 groups
	for (int i = 0; i < 100; i++) {
		retval = rtr_mgr_add_group(conf, &group5);
		group5.preference++;
		assert(retval == RTR_SUCCESS);
	}

	//remove 100 groups
	for (int i = 104; i >= 5; i--) {
		retval = rtr_mgr_remove_group(conf, i);
		assert(retval == RTR_SUCCESS);
	}

	rtr_mgr_remove_group(conf, 4);

	//try to remove last remainig group.
	retval = rtr_mgr_remove_group(conf, 3);
	assert(retval == RTR_ERROR);

	rtr_mgr_stop(conf);
	rtr_mgr_free(conf);
	free(groups[0].sockets);
	free(group2.sockets);
	free(group3.sockets);
	free(group4.sockets);
	free(group5.sockets);
}
