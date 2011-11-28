#include <stdlib.h>
#include "rtrlib/rtrlib.h"

int main(){
    //create a SSH transport socket
    tr_socket tr_ssh;
    tr_ssh_config config = {
        "123.231.123.221",          //IP
        22,                         //Port
        "rpki_user",
        "/etc/rpki-rtr/hostkey",    //Server hostkey
        "/etc/rpki-rtr/client.priv", //Authentication private key
        "/etc/rpki-rtr/client.pub"  //Authentication public key
    };
    tr_ssh_init(&config, &tr_ssh);

    //create a TCP transport socket
    tr_socket tr_tcp1;
    tr_tcp_config tcp_config1 = {
        "141.22.26.252",          //IP
        "8282"                      //Port
    };
    tr_tcp_init(&tcp_config1, &tr_tcp1);

    //create another TCP transport socket
    tr_socket tr_tcp2;
    tr_tcp_config tcp_config2 = {
        "141.22.26.252",          //IP
        "8282"                      //Port
    };
    tr_tcp_init(&tcp_config2, &tr_tcp2);

    //create 3 rtr_sockets and associate them with the transprort sockets
    rtr_socket rtr_ssh, rtr_tcp1, rtr_tcp2;
    rtr_ssh.tr_socket = &tr_ssh;
    rtr_tcp1.tr_socket = &tr_tcp1;
    rtr_tcp2.tr_socket = &tr_tcp2;

    //create a rtr_mgr_group array with 2 elements
    rtr_mgr_group groups[2];
    
    //The first group contains both TCP RTR sockets
    groups[0].sockets = malloc(2 * sizeof(rtr_socket*));
    groups[0].sockets_len = 2;
    groups[0].sockets[0] = &rtr_tcp1;
    groups[0].sockets[1] = &rtr_tcp2;
    groups[0].preference = 1;       //Preference value of this group

    //The seconds group contains only the SSH RTR socket
    groups[1].sockets = malloc(1 * sizeof(rtr_socket*));
    groups[1].sockets_len = 1;
    groups[1].sockets[0] = &rtr_ssh;
    groups[1].preference = 2;

    //create a rtr_mgr_config struct that stores the group
    rtr_mgr_config conf;
    conf.groups = groups;
    conf.len = 2;                   //2 elements in the groups array

    //initialize all rtr_sockets in the server pool with the same settings
    rtr_mgr_init(&conf, 240, 520, NULL);

    //start the connection manager
    rtr_mgr_start(&conf);

    //wait till at least one rtr_mgr_group is fully synchronized with the server
    while(!rtr_mgr_conf_in_sync(&conf))
        sleep(1);
    //validate the BGP origin ASN 12345 for prefix 0x0A0A/24
    ip_addr pref;
    pref.ver = IPV4;
    pref.u.addr4.addr = 0x0A0A0000;
    pfxv_state result;
    rtr_mgr_validate(&conf, 12345, &pref, 24, &result);

    rtr_mgr_stop(&conf);
    rtr_mgr_free(&conf);
    free(groups[0].sockets);
    free(groups[1].sockets);
}
