#include <stdlib.h>
#include "rtrlib/transport/ssh/ssh_transport.h"

int main(){
    struct tr_socket ssh_socket;
    char ssh_host[]		= "123.231.123.221";
    char ssh_user[]		= "rpki_user";
    char ssh_hostkey[]	= "/etc/rpki-rtr/hostkey";
    char ssh_privkey[]	= "/etc/rpki-rtr/client.priv";

    struct tr_ssh_config config = {
        ssh_host,
        22,
        NULL,
        ssh_user,
        ssh_hostkey,
        ssh_privkey,
    };

    tr_ssh_init(&config, &ssh_socket);
    tr_open(&ssh_socket);
    tr_close(&ssh_socket);
    tr_free(&ssh_socket);
}
