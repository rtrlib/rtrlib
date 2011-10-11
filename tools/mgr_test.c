#include <stdlib.h>
#include <stdio.h>
#include "rtrlib/rtrlib.h"
#include <rtrlib/rtr_mgr.h>


int main(){
    pfx_table pfxt;
    pfx_table_init(&pfxt, NULL);

    tr_socket* tr_tcp;
    tr_socket* tr_tcp1;
    tr_socket* tr_ssh;
    tr_tcp_config tcp_config = {
        "localhost",          //IP
        "8282"                      //Port
    };

    tr_tcp_init(&tcp_config, &tr_tcp);
    tr_tcp_init(&tcp_config, &tr_tcp1);
    rtr_socket rtr_tcp;
    rtr_init(&rtr_tcp, tr_tcp, &pfxt, 240, 520);
    rtr_socket rtr_tcp1;
    rtr_init(&rtr_tcp1, tr_tcp1, &pfxt, 240, 520);

    tr_ssh_config ssh_config = {
        "141.22.26.232",
        22,
        "fho",
        NULL,
        "/tmp/key",
        "/tmp/key.pub"
    };

    tr_ssh_init(&ssh_config, &tr_ssh);
    rtr_socket rtr_ssh;
    rtr_init(&rtr_ssh, tr_ssh, &pfxt, 240, 520);


    rtr_mgr_config conf[2];
    conf[0].sockets_len = 2;
    conf[0].sockets = malloc(2 * sizeof(rtr_socket*));
    conf[0].sockets[0] = &rtr_ssh;
    conf[0].sockets[1] = &rtr_tcp;
    conf[0].preference = 2;
    conf[1].sockets = malloc(1 * sizeof(rtr_socket*));
    conf[1].sockets_len = 1;
    conf[1].sockets[0] = &rtr_tcp1;
    conf[1].preference = 3;

    rtr_mgr_init(conf, 2);
    rtr_mgr_start(conf);
    printf("started\n");
    sleep(500000);
}
