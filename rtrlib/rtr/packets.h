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

#ifndef RTR_PACKETS_H
#define RTR_PACKETS_H
#include <arpa/inet.h>
#include "rtrlib/lib/ipv6.h"
#include "rtrlib/lib/ipv4.h"
#include "rtrlib/rtr/rtr.h"

static const uint8_t RTR_PROTOCOL_VERSION = 0;
static const unsigned int RTR_MAX_PDU_LEN = 3248; //error pdu: header(8) + len(4) + ipv6_pdu(32) + len(4) + 400*8 (400 char text)
static const unsigned int RTR_RECV_TIMEOUT = 60;
static const unsigned int RTR_SEND_TIMEOUT = 60;

void rtr_change_socket_state(rtr_socket* rtr_socket, const rtr_socket_state new_state);
int rtr_sync(rtr_socket* rtr_socket);
int rtr_wait_for_sync(rtr_socket* rtr_socket);
int rtr_send_serial_query(rtr_socket* rtr_socket);
int rtr_send_reset_query(rtr_socket* rtr_socket);

#endif
