/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 */

#include "rtrlib/transport/transport_private.h"

int __wrap_tr_recv_all(const struct rtr_tr_socket *socket, const void *buf, const size_t len, const time_t timeout);
int __wrap_tr_send_all(const struct rtr_tr_socket *socket, const void *pdu, const size_t len, const time_t timeout);
