/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website; http://rtrlib.realmv6.org/
 */

#include "rtrlib/rtr/rtr.h"

void __real_rtr_change_socket_state(struct rtr_socket *rtr_socket, const enum rtr_socket_state new_state);
int __wrap_tr_send_all(const struct tr_socket *socket, const void *pdu, const size_t len, const time_t timeout);
void __wrap_rtr_change_socket_state(struct rtr_socket *rtr_socket, const enum rtr_socket_state new_state);
