#include <stdlib.h>
#include <stdio.h>
#include "rtrlib/rtrlib.h"
#include <rtrlib/rtr_mgr.h>

int main(){
    pfx_table pfxt;
    pfx_table_init(&pfxt, NULL, 0);

    tr_socket* tr_tcp;
    tr_socket* tr_ssh;
    tr_tcp_config tcp_config = {
        "141.22.26.232",          //IP
        "42420"                      //Port
    };

    tr_tcp_init(&tcp_config, &tr_tcp);
    rtr_socket rtr_tcp;
    rtr_init(&rtr_tcp, tr_tcp, &pfxt, 240, 480);

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
    rtr_init(&rtr_ssh, tr_ssh, &pfxt, 10, 480);


    rtr_mgr_config conf[2];
    for(int i=0;i<2;i++){
        conf[i].sockets = malloc(1 * sizeof(rtr_socket*));
        if(i==0){
            conf[i].preference = 2;
            conf[i].sockets[0] = &rtr_ssh;
        }
        else if(i ==1){
            conf[i].preference = 3;
            conf[i].sockets[0] = &rtr_tcp;
        }
        conf[i].sockets_len = 1;
    }

    rtr_mgr_init(conf, 2);
    rtr_mgr_start(conf);
    printf("started\n");
    sleep(500000);
}
