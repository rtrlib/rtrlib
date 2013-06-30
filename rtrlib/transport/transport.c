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

#include "rtrlib/transport/transport.h"
#include "rtrlib/lib/utils.h"

inline int tr_open(tr_socket *socket) {
    return socket->open_fp(socket->socket);
}

inline void tr_close(tr_socket *socket) {
    socket->close_fp(socket->socket);
}

inline void tr_free(tr_socket *socket) {
    socket->free_fp(socket);
}

inline int tr_send(const tr_socket *socket, const void *pdu, const size_t len, const time_t timeout) {
    return socket->send_fp(socket->socket, pdu, len, timeout);
}

inline int tr_recv(const tr_socket *socket, void *buf, const size_t len, const time_t timeout) {
    return socket->recv_fp(socket->socket, buf, len, timeout);
}

inline const char *tr_ident(tr_socket *sock) {
	return sock->ident_fp(sock->socket);
}

int tr_send_all(const tr_socket *socket, const void *pdu, const size_t len, const time_t timeout) {
    unsigned int total_send = 0;
    int rtval = 0;
    time_t end_time;
    rtr_get_monotonic_time(&end_time);
    end_time = end_time + timeout;

    while(total_send < len) {
        time_t cur_time;
        rtr_get_monotonic_time(&cur_time);

        rtval = tr_send(socket, ((char *) pdu) + total_send, (len - total_send), (end_time - cur_time));
        if(rtval < 0)
            return rtval;
        total_send += rtval;
    }
    return total_send;
}

int tr_recv_all(const tr_socket *socket, const void *pdu, const size_t len, const time_t timeout) {
    size_t total_recv = 0;
    int rtval = 0;
    time_t end_time;
    rtr_get_monotonic_time(&end_time);
    end_time += timeout;

    while(total_recv < len) {
        time_t cur_time;
        rtr_get_monotonic_time(&cur_time);

        rtval = tr_recv(socket, ((char *) pdu)+total_recv, (len - total_recv), end_time - cur_time);
        if(rtval < 0)
            return rtval;
        total_recv += rtval;
    }
    return total_recv;
}
