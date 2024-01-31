/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "tcp_transport_private.h"

#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/lib/log_private.h"
#include "rtrlib/rtrlib_export_private.h"
#include "rtrlib/transport/transport_private.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define TCP_DBG(fmt, sock, ...)                                                                            \
	do {                                                                                               \
		const struct tr_tcp_socket *tmp = sock;                                                    \
		lrtr_dbg("TCP Transport(%s:%s): " fmt, tmp->config.host, tmp->config.port, ##__VA_ARGS__); \
	} while (0)
#define TCP_DBG1(a, sock) TCP_DBG(a, sock)

struct tr_tcp_socket {
	int socket;
	struct tr_tcp_config config;
	char *ident;
};

static int tr_tcp_open(void *tr_tcp_sock);
static void tr_tcp_close(void *tr_tcp_sock);
static void tr_tcp_free(struct tr_socket *tr_sock);
static int tr_tcp_recv(const void *tr_tcp_sock, void *pdu, const size_t len, const time_t timeout);
static int tr_tcp_send(const void *tr_tcp_sock, const void *pdu, const size_t len, const time_t timeout);
static const char *tr_tcp_ident(void *socket);

static int set_socket_blocking(int socket)
{
	int flags = fcntl(socket, F_GETFL);

	if (flags == -1)
		return TR_ERROR;

	flags &= ~O_NONBLOCK;

	if (fcntl(socket, F_SETFL, flags) == -1)
		return TR_ERROR;

	return TR_SUCCESS;
}

static int set_socket_non_blocking(int socket)
{
	int flags = fcntl(socket, F_GETFL);

	if (flags == -1)
		return TR_ERROR;

	flags |= O_NONBLOCK;

	if (fcntl(socket, F_SETFL, flags) == -1)
		return TR_ERROR;

	return TR_SUCCESS;
}

static int get_socket_error(int socket)
{
	int result;
	socklen_t result_len = sizeof(result);

	if (getsockopt(socket, SOL_SOCKET, SO_ERROR, &result, &result_len) < 0)
		return TR_ERROR;

	return result;
}

/* WARNING: This function has cancelable sections! */
int tr_tcp_open(void *tr_socket)
{
	int rtval = TR_ERROR;
	int tcp_rtval = 0;
	struct tr_tcp_socket *tcp_socket = tr_socket;
	const struct tr_tcp_config *config = &tcp_socket->config;

	assert(tcp_socket->socket == -1);

	struct addrinfo hints;
	struct addrinfo *res = NULL;
	struct addrinfo *bind_addrinfo = NULL;

	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_ADDRCONFIG;

	if (config->new_socket) {
		tcp_socket->socket = (*config->new_socket)(config->data);
		if (tcp_socket->socket <= 0) {
			TCP_DBG("Couldn't establish TCP connection, %s", tcp_socket, strerror(errno));
			goto end;
		}
	}
	if (tcp_socket->socket < 0) {
		tcp_rtval = getaddrinfo(config->host, config->port, &hints, &res);
		if (tcp_rtval != 0) {
			if (tcp_rtval == EAI_SYSTEM) {
				TCP_DBG("getaddrinfo error, %s", tcp_socket, strerror(errno));
			} else {
				TCP_DBG("getaddrinfo error, %s", tcp_socket, gai_strerror(tcp_rtval));
			}
			return TR_ERROR;
		}

		tcp_socket->socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (tcp_socket->socket == -1) {
			TCP_DBG("Socket creation failed, %s", tcp_socket, strerror(errno));
			goto end;
		}

		if (tcp_socket->config.bindaddr) {
			tcp_rtval = getaddrinfo(tcp_socket->config.bindaddr, 0, &hints, &bind_addrinfo);
			if (tcp_rtval != 0) {
				if (tcp_rtval == EAI_SYSTEM) {
					TCP_DBG("getaddrinfo error, %s", tcp_socket, strerror(errno));
				} else {
					TCP_DBG("getaddrinfo error, %s", tcp_socket, gai_strerror(tcp_rtval));
				}
				goto end;
			}
			if (bind(tcp_socket->socket, bind_addrinfo->ai_addr, bind_addrinfo->ai_addrlen) != 0) {
				TCP_DBG("Socket bind failed, %s", tcp_socket, strerror(errno));
				goto end;
			}

			freeaddrinfo(bind_addrinfo);
			bind_addrinfo = NULL;
		}

		if (set_socket_non_blocking(tcp_socket->socket) == TR_ERROR) {
			TCP_DBG("Could not set socket to non blocking, %s", tcp_socket, strerror(errno));
			goto end;
		}

		if (connect(tcp_socket->socket, res->ai_addr, res->ai_addrlen) == -1 && errno != EINPROGRESS) {
			TCP_DBG("Couldn't establish TCP connection, %s", tcp_socket, strerror(errno));
			goto end;
		}

		freeaddrinfo(res);
		res = NULL;

		fd_set wfds;

		FD_ZERO(&wfds);
		FD_SET(tcp_socket->socket, &wfds);

		struct timeval timeout = {.tv_sec = tcp_socket->config.connect_timeout, .tv_usec = 0};
		int oldcancelstate;

		/* Enable cancellability for the select call
		 * to prevent rtr_stop from blocking for a long time.
		 * It must be the only blocking call in this function.
		 * Since local resources have all been freed this should be safe.
		 */
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldcancelstate);
		int ret = select(tcp_socket->socket + 1, NULL, &wfds, NULL, &timeout);

		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldcancelstate);

		if (ret < 0) {
			TCP_DBG("Could not select tcp socket, %s", tcp_socket, strerror(errno));
			goto end;

		} else if (ret == 0) {
			TCP_DBG("Could not establish TCP connection in time", tcp_socket);
			goto end;
		}

		int socket_error = get_socket_error(tcp_socket->socket);

		if (socket_error == TR_ERROR) {
			TCP_DBG("Could not get socket error, %s", tcp_socket, strerror(errno));
			goto end;

		} else if (socket_error > 0) {
			TCP_DBG("Could not establish TCP connection, %s", tcp_socket, strerror(socket_error));
			goto end;
		}

		if (set_socket_blocking(tcp_socket->socket) == TR_ERROR) {
			TCP_DBG("Could not set socket to blocking, %s", tcp_socket, strerror(errno));
			goto end;
		}
	}

	TCP_DBG1("Connection established", tcp_socket);
	rtval = TR_SUCCESS;

end:
	if (res)
		freeaddrinfo(res);

	if (bind_addrinfo)
		freeaddrinfo(bind_addrinfo);
	if (rtval == -1)
		tr_tcp_close(tr_socket);
	return rtval;
}

void tr_tcp_close(void *tr_tcp_sock)
{
	struct tr_tcp_socket *tcp_socket = tr_tcp_sock;

	if (tcp_socket->socket != -1)
		close(tcp_socket->socket);
	TCP_DBG1("Socket closed", tcp_socket);
	tcp_socket->socket = -1;
}

void tr_tcp_free(struct tr_socket *tr_sock)
{
	struct tr_tcp_socket *tcp_sock = tr_sock->socket;

	assert(tcp_sock);
	assert(tcp_sock->socket == -1);

	TCP_DBG1("Freeing socket", tcp_sock);

	lrtr_free(tcp_sock->config.host);
	lrtr_free(tcp_sock->config.port);
	lrtr_free(tcp_sock->config.bindaddr);

	if (tcp_sock->ident)
		lrtr_free(tcp_sock->ident);
	tr_sock->socket = NULL;
	lrtr_free(tcp_sock);
}

int tr_tcp_recv(const void *tr_tcp_sock, void *pdu, const size_t len, const time_t timeout)
{
	const struct tr_tcp_socket *tcp_socket = tr_tcp_sock;
	int rtval;

	if (timeout == 0) {
		rtval = recv(tcp_socket->socket, pdu, len, MSG_DONTWAIT);
	} else {
		struct timeval t = {timeout, 0};

		if (setsockopt(tcp_socket->socket, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t)) == -1) {
			TCP_DBG("setting SO_RCVTIMEO failed, %s", tcp_socket, strerror(errno));
			return TR_ERROR;
		}
		rtval = recv(tcp_socket->socket, pdu, len, 0);
	}

	if (rtval == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return TR_WOULDBLOCK;
		if (errno == EINTR)
			return TR_INTR;
		TCP_DBG("recv(..) error: %s", tcp_socket, strerror(errno));
		return TR_ERROR;
	}
	if (rtval == 0)
		return TR_CLOSED;
	return rtval;
}

int tr_tcp_send(const void *tr_tcp_sock, const void *pdu, const size_t len, const time_t timeout)
{
	const struct tr_tcp_socket *tcp_socket = tr_tcp_sock;
	int rtval;

	if (timeout == 0) {
		rtval = send(tcp_socket->socket, pdu, len, MSG_DONTWAIT);
	} else {
		struct timeval t = {timeout, 0};

		if (setsockopt(tcp_socket->socket, SOL_SOCKET, SO_SNDTIMEO, &t, sizeof(t)) == -1) {
			TCP_DBG("setting SO_SNDTIMEO failed, %s", tcp_socket, strerror(errno));
			return TR_ERROR;
		}
		rtval = send(tcp_socket->socket, pdu, len, 0);
	}

	if (rtval == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return TR_WOULDBLOCK;
		if (errno == EINTR)
			return TR_INTR;
		TCP_DBG("send(..) error: %s", tcp_socket, strerror(errno));
		return TR_ERROR;
	}
	if (rtval == 0)
		return TR_ERROR;
	return rtval;
}

const char *tr_tcp_ident(void *socket)
{
	size_t len;
	struct tr_tcp_socket *sock = socket;

	assert(sock);

	if (sock->ident)
		return sock->ident;

	len = strlen(sock->config.port) + strlen(sock->config.host) + 2;
	sock->ident = lrtr_malloc(len);
	if (!sock->ident)
		return NULL;
	snprintf(sock->ident, len, "%s:%s", sock->config.host, sock->config.port);

	return sock->ident;
}

RTRLIB_EXPORT int tr_tcp_init(const struct tr_tcp_config *config, struct tr_socket *socket)
{
	socket->close_fp = &tr_tcp_close;
	socket->free_fp = &tr_tcp_free;
	socket->open_fp = &tr_tcp_open;
	socket->recv_fp = &tr_tcp_recv;
	socket->send_fp = &tr_tcp_send;
	socket->ident_fp = &tr_tcp_ident;

	socket->socket = lrtr_malloc(sizeof(struct tr_tcp_socket));
	struct tr_tcp_socket *tcp_socket = socket->socket;

	tcp_socket->socket = -1;
	tcp_socket->config.host = lrtr_strdup(config->host);
	tcp_socket->config.port = lrtr_strdup(config->port);
	if (config->bindaddr)
		tcp_socket->config.bindaddr = lrtr_strdup(config->bindaddr);
	else
		tcp_socket->config.bindaddr = NULL;

	if (config->connect_timeout == 0)
		tcp_socket->config.connect_timeout = RTRLIB_TRANSPORT_CONNECT_TIMEOUT_DEFAULT;
	else
		tcp_socket->config.connect_timeout = config->connect_timeout;

	tcp_socket->ident = NULL;
	tcp_socket->config.data = config->data;
	tcp_socket->config.new_socket = config->new_socket;

	return TR_SUCCESS;
}
