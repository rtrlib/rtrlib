#include <stdlib.h>
#include "rtrlib/rtrlib.h"

int main(){
    //create a SSH transport socket
    char ssh_host[]		= "123.231.123.221";
    char ssh_user[]		= "rpki_user";
    char ssh_hostkey[]	= "/etc/rpki-rtr/hostkey";
    char ssh_privkey[]	= "/etc/rpki-rtr/client.priv";
    struct tr_socket tr_ssh;
    struct tr_ssh_config config = {
        ssh_host,	//IP
        22,		//Port
        NULL,		//Source address
        ssh_user,
        ssh_hostkey,	//Server hostkey
        ssh_privkey,	//Private key
    };
    tr_ssh_init(&config, &tr_ssh);

    //create a TCP transport socket
    struct tr_socket tr_tcp1;
	char tcp1_host[]	= "rpki.realmv6.org";
	char tcp1_port[]	= "42420";

    struct tr_tcp_config tcp_config1 = {
	tcp1_host,	//IP
	tcp1_port,	//Port
	NULL		//Source address
    };
    tr_tcp_init(&tcp_config1, &tr_tcp1);

    //create another TCP transport socket
    struct tr_socket tr_tcp2;
	char tcp2_host[]	= "localhost";
	char tcp2_port[]	= "8282";
    struct tr_tcp_config tcp_config2 = {
	tcp2_host,                //IP
	tcp2_port,                //Port
	NULL                      //Source address
    };
    tr_tcp_init(&tcp_config2, &tr_tcp2);

    //create 3 rtr_sockets and associate them with the transprort sockets
    struct rtr_socket rtr_ssh, rtr_tcp1, rtr_tcp2;
    rtr_ssh.tr_socket = &tr_ssh;
    rtr_tcp1.tr_socket = &tr_tcp1;
    rtr_tcp2.tr_socket = &tr_tcp2;

    //create a rtr_mgr_group array with 2 elements
    struct rtr_mgr_group groups[2];

    //The first group contains both TCP RTR sockets
    groups[0].sockets = malloc(2 * sizeof(struct rtr_socket*));
    groups[0].sockets_len = 2;
    groups[0].sockets[0] = &rtr_tcp1;
    groups[0].sockets[1] = &rtr_tcp2;
    groups[0].preference = 1;       //Preference value of this group

    //The seconds group contains only the SSH RTR socket
    groups[1].sockets = malloc(1 * sizeof(struct rtr_socket*));
    groups[1].sockets_len = 1;
    groups[1].sockets[0] = &rtr_ssh;
    groups[1].preference = 2;

    //create a rtr_mgr_config struct that stores the group

    //initialize all rtr_sockets in the server pool with the same settings
    struct rtr_mgr_config *conf;
    int ret = rtr_mgr_init(&conf ,groups, 2, 30, 600, 600, NULL, NULL, NULL, NULL);

    //start the connection manager
    rtr_mgr_start(conf);

    //wait till at least one rtr_mgr_group is fully synchronized with the server
    while(!rtr_mgr_conf_in_sync(conf))
        sleep(1);

    //validate the BGP-Route 10.10.0.0/24, origin ASN: 12345
    struct lrtr_ip_addr pref;
    lrtr_ip_str_to_addr("10.10.0.0", &pref);
    enum pfxv_state result;
    rtr_mgr_validate(conf, 12345, &pref, 24, &result);

    rtr_mgr_stop(conf);
    rtr_mgr_free(conf);
    free(groups[0].sockets);
    free(groups[1].sockets);
}
