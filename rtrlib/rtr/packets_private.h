/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef RTR_PACKETS_PRIVATE_H
#define RTR_PACKETS_PRIVATE_H

#include "rtrlib/rtr/rtr_private.h"

#include <arpa/inet.h>

// error pdu: header(8) + len(4) + ipv6_pdu(32) + len(4) + 400*8 (400 char text)
static const unsigned int RTR_MAX_PDU_LEN = 3248;
static const unsigned int RTR_RECV_TIMEOUT = 60;
static const unsigned int RTR_SEND_TIMEOUT = 60;

void __attribute__((weak))
rtr_change_socket_state(struct rtr_socket *rtr_socket, const enum rtr_socket_state new_state);
int rtr_sync(struct rtr_socket *rtr_socket);
int rtr_wait_for_sync(struct rtr_socket *rtr_socket);
int rtr_send_serial_query(struct rtr_socket *rtr_socket);
int rtr_send_reset_query(struct rtr_socket *rtr_socket);
int rtr_check_interval_range(uint32_t interval, uint32_t minimum, uint32_t maximum);
void apply_interval_value(struct rtr_socket *rtr_socket, uint32_t interval, enum rtr_interval_type type);
int rtr_check_interval_option(struct rtr_socket *rtr_socket, int interval_mode, uint32_t interval,
			      enum rtr_interval_type type);
#endif
