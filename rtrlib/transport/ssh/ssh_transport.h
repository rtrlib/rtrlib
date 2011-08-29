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

/**
 * @defgroup mod_ssh_transport_h SSH transport socket
 * @ingroup mod_transport_h
 * @brief An implementation of the SSH protocol as RTR transport.
 * The transport uses libssh for all ssh specific operations.\n
 * See @ref mod_transport_h "transport interface" for a list of supported operations.
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
typedef struct tr_ssh_config{
  const char* host;
  const unsigned int port;
  const char* username;
  const char* server_hostkey_path;
  const char* client_privkey_path;
  const char* client_pubkey_path;
} tr_ssh_config;

/**
 * @brief Initialize the tr_socket struct for a SSH connection.
 * @param[in] config SSH configuration for the connection.
 * @param[out] socket Initialized transport socket.
 * @returns TR_SUCCESS on success
 * @returns TR_ERROR on error
 */
int tr_ssh_init(const tr_ssh_config* config, tr_socket** socket);

#endif
/* @} */
