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
 * Website: http://rpki.realmv6.org/
 */

/**
 * @defgroup mod_ssh_transport_h SSH transport socket
 * @ingroup mod_transport_h
 * @brief An implementation of the SSH protocol for the RTR transport.
 * @details This transport implementation uses libssh
 * (http://www.libssh.org/) for all ssh specific operations.\n
 * See @ref mod_transport_h "transport interface" for a list of supported
 * operations.
 *
 * @{
 *
 * @example ssh_tr.c
 * Example of how to open a SSH transport connection.
 */

#ifndef SSH_TRANSPORT_H
#define SSH_TRANSPORT_H
#include <libssh/libssh.h>
#include "../transport.h"

/**
 * @brief A tr_ssh_config struct holds configuration data for an tr_ssh socket.
 * @param host Hostname or IP address to connect to.
 * @param port Port to connect to.
 * @param username Username for authentication.
 * @param server_hostkey_path Path to the public SSH key of the server.
 * @param client_privkey_path Path to the private key of the authentication keypair.
 * @param client_pubkey_path Path to the public key of the authentication keypair.
 */
struct tr_ssh_config {
    char *host;
    unsigned int port;
    char *username;
    char *server_hostkey_path;
    char *client_privkey_path;
    char *client_pubkey_path;
};

/**
 * @brief Initializes the tr_socket struct for a SSH connection.
 * @param[in] config SSH configuration for the connection.
 * @param[out] socket Initialized transport socket.
 * @returns TR_SUCCESS On success.
 * @returns TR_ERROR On error.
 */
int tr_ssh_init(const struct tr_ssh_config *config, struct tr_socket *socket);

#endif
/* @} */
