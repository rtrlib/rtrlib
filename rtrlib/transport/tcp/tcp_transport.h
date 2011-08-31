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
 * @defgroup mod_tcp_transport_h TCP transport socket
 * @ingroup mod_transport_h
 * @brief An implementation of the TCP protocol for the RTR transport.
 * See @ref mod_transport_h "transport interface" for a list of supported operations.
 *
 * @{
 */

#ifndef RTR_TCP_TRANSPORT_H
#define RTR_TCP_TRANSPORT_H
#include "rtrlib/transport/transport.h"

/**
 * @brief  A tr_tcp_config struct holds configuration for a TCP connection.
 * @param host Hostname or IP address to connect to.
 * @param port Port to connect to.
 */
typedef struct tr_tcp_config{
  const char* host;
  const char* port;
} tr_tcp_config;


/**
 * @brief Initialize the tr_socket struct for a TCP connection.
 * @param[in] config TCP configuration for the connection.
 * @param[out] socket Initialized transport socket.
 * @returns TR_SUCCESS on success
 * @returns TR_ERROR on error
 */
int tr_tcp_init(const tr_tcp_config* config, tr_socket** socket);
#endif
/* @} */
