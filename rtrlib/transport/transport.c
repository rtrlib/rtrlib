/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "transport_private.h"

#include "rtrlib/lib/utils_private.h"

inline int tr_open(struct tr_socket *socket)
{
	return socket->open_fp(socket->socket);
}

inline void tr_close(struct tr_socket *socket)
{
	socket->close_fp(socket->socket);
}

inline void tr_free(struct tr_socket *socket)
{
	socket->free_fp(socket);
}

inline int tr_send(const struct tr_socket *socket, const void *pdu, const size_t len, const time_t timeout)
{
	return socket->send_fp(socket->socket, pdu, len, timeout);
}

inline int tr_recv(const struct tr_socket *socket, void *buf, const size_t len, const time_t timeout)
{
	return socket->recv_fp(socket->socket, buf, len, timeout);
}

/* cppcheck-suppress unusedFunction */
inline const char *tr_ident(struct tr_socket *sock)
{
	return sock->ident_fp(sock->socket);
}

int tr_send_all(const struct tr_socket *socket, const void *pdu, const size_t len, const time_t timeout)
{
	unsigned int total_send = 0;
	time_t end_time;

	lrtr_get_monotonic_time(&end_time);
	end_time = end_time + timeout;

	while (total_send < len) {
		time_t cur_time;
		int rtval;

		lrtr_get_monotonic_time(&cur_time);

		rtval = tr_send(socket, ((char *)pdu) + total_send, (len - total_send), (end_time - cur_time));
		if (rtval < 0)
			return rtval;
		total_send += rtval;
	}
	return total_send;
}

int tr_recv_all(const struct tr_socket *socket, const void *pdu, const size_t len, const time_t timeout)
{
	size_t total_recv = 0;
	time_t end_time;

	lrtr_get_monotonic_time(&end_time);
	end_time += timeout;

	while (total_recv < len) {
		time_t cur_time;
		int rtval;

		lrtr_get_monotonic_time(&cur_time);

		rtval = tr_recv(socket, ((char *)pdu) + total_recv, (len - total_recv), end_time - cur_time);
		if (rtval < 0)
			return rtval;
		total_recv += rtval;
	}
	return total_recv;
}
