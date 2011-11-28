#include <stdlib.h>
#include "rtrlib/transport/ssh/ssh_transport.h"

int main(){
    tr_socket ssh_socket;

    tr_ssh_config config = {
        "123.321.123.321",
        22,
        "rpki_user",
        "/etc/rpki-rtr/hostkey",
        "/etc/rpki-rtr/server.priv",
        "/etc/rpki-rtr/server.pub"
    };

    tr_ssh_init(&config, &ssh_socket);
    tr_open(&ssh_socket);
    tr_close(&ssh_socket);
    tr_free(&ssh_socket);
}
