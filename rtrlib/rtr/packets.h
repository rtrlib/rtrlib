/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
*/

#ifndef RTR_PACKETS_H
#define RTR_PACKETS_H
#include <arpa/inet.h>
#include "rtrlib/lib/ipv6.h"
#include "rtrlib/lib/ipv4.h"
#include "rtrlib/rtr/rtr.h"


static const unsigned int RTR_MAX_PDU_LEN = 3248; //error pdu: header(8) + len(4) + ipv6_pdu(32) + len(4) + 400*8 (400 char text)
static const unsigned int RTR_RECV_TIMEOUT = 60;
static const unsigned int RTR_SEND_TIMEOUT = 60;

void rtr_change_socket_state(struct rtr_socket *rtr_socket, const enum rtr_socket_state new_state);
int rtr_sync(struct rtr_socket *rtr_socket);
int rtr_wait_for_sync(struct rtr_socket *rtr_socket);
int rtr_send_serial_query(struct rtr_socket *rtr_socket);
int rtr_send_reset_query(struct rtr_socket *rtr_socket);

#endif
