/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
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
 * @param bindaddr Hostname or IP address to connect from. NULL for
 *		   determination by OS.
 * to use the source address of the system's default route to the server
 */
struct tr_tcp_config {
	char *host;
	char *port;
	char *bindaddr;
};

/**
 * @brief Initializes the tr_socket struct for a TCP connection.
 * @param[in] config TCP configuration for the connection.
 * @param[out] socket Initialized transport socket.
 * @returns TR_SUCCESS On success.
 * @returns TR_ERROR On error.
 */
int tr_tcp_init(const struct tr_tcp_config *config, struct tr_socket *socket);
#endif
/** @} */
