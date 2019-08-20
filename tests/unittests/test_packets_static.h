/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website; http://rtrlib.realmv6.org/
 */

#include "rtrlib/transport/transport_private.h"

int __wrap_lrtr_get_monotonic_time(time_t *seconds);

int __wrap_tr_send_all(const struct tr_socket *socket, const void *pdu, const size_t len, const time_t timeout);
