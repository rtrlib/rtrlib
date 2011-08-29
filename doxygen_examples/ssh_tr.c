/*
 * This file is part of RTRlib.
 *
 * RTRlib is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * RTRlib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RTRlib; see the file COPYING.LESSER.
 *
 * written by Fabian Holler, in cooperation with:
 * INET group, Hamburg University of Applied Sciences,
 * CST group, Freie Universitaet Berlin
 * Webpage: http://rpki.realmv6.org/
 */

#include <stdlib.h>
#include "rtrlib/transport/ssh/ssh_transport.h"

int main(){
    tr_socket* ssh_socket;

    tr_ssh_config config = {
        "123.321.123.321",
        22,
        "rpki_user",
        "/etc/rpki-rtr/hostkey",
        "/etc/rpki-rtr/server.priv"
        "/etc/rpki-rtr/server.pub"
    };

    tr_ssh_init(&config, &ssh_socket);
    tr_open(ssh_socket);
}
