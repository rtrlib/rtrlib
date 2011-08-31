#include <stdlib.h>
#include "rtrlib/rtrlib.h"

int main(){
    //create a SSH transport socket
    tr_socket* tr_ssh;
    tr_ssh_config config = {
        "123.321.123.321",          //IP
        22,                         //Port
        "rpki_user",
        "/etc/rpki-rtr/hostkey",    //Server hostkey
        "/etc/rpki-rtr/client.priv" //Authentication private key
        "/etc/rpki-rtr/client.pub"  //Authentication public key
    };
    tr_ssh_init(&config, &tr_ssh);

    //create a TCP transport socket
    tr_socket* tr_tcp;
    tr_tcp_config tcp_config = {
        "123.321.123.321",          //IP
        "1234"                      //Port
    };
    tr_tcp_init(&tcp_config, &tr_tcp);

    //create 2 rtr_sockets for both transport sockets
    rtr_socket rtr_ssh, rtr_tcp;
    rtr_ssh.tr_socket = tr_ssh;
    rtr_tcp.tr_socket = tr_tcp;

    //create a rtr_server_pool with both rtr sockets and preference values
    rtr_server_pool p1 = {
        3,                          //preference value
        &rtr_tcp,                   //rtr_socket
        NULL                        //next pointer
    };

    rtr_server_pool p0 = {
        5,                          //preference value
        &rtr_ssh,                   //rtr_socket
        &p1                        //next pointer
    };

    //init all rtr_sockets in the server pool with the same settings
    rtr_mgr_init(&p0, 60, 120, NULL, 0, NULL, 0); 
    //create a management socket and start the connection manager
    rtr_mgr_socket mgr_sock;
    rtr_mgr_start(&mgr_sock, &p0);

    //validate the BGP origin ASN 12345 for prefix 0x0A0A/24
    ip_addr pref;
    pref.ver = IPV4;
    pref.u.addr4.addr = htonl(0x0A0A0000);
    pfxv_state result;
    rtr_mgr_validate(&mgr_sock, 12345, &pref, 24, &result);
}
