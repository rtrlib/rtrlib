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
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define TCP_DBG(fmt, sock, ...) \
	lrtr_dbg("TCP Transport(%s:%s): " fmt, sock->config.host, sock->config.port, ##__VA_ARGS__)
#define TCP_DBG1(a, sock) lrtr_dbg("TCP Transport(%s:%s): " a, sock->config.host, sock->config.port)

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

int tr_tcp_open(void *tr_socket)
{
	int rtval = TR_ERROR;
	struct tr_tcp_socket *tcp_socket = tr_socket;
	assert(tcp_socket->socket == -1);

	struct addrinfo hints;
	struct addrinfo *res;
	struct addrinfo *bind_addrinfo = NULL;

	bzero(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_ADDRCONFIG;
	if (getaddrinfo(tcp_socket->config.host, tcp_socket->config.port, &hints, &res) != 0) {
		TCP_DBG("getaddrinfo error, %s", tcp_socket, gai_strerror(errno));
		return TR_ERROR;
	}

	if ((tcp_socket->socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
		TCP_DBG("Socket creation failed, %s", tcp_socket, strerror(errno));
		goto end;
	}

	if (tcp_socket->config.bindaddr) {
		if (getaddrinfo(tcp_socket->config.bindaddr, 0, &hints, &bind_addrinfo) != 0) {
			TCP_DBG("getaddrinfo error, %s", tcp_socket, gai_strerror(errno));
			goto end;
		}

		if (bind(tcp_socket->socket, bind_addrinfo->ai_addr, bind_addrinfo->ai_addrlen) != 0) {
			TCP_DBG("Socket bind failed, %s", tcp_socket, strerror(errno));
			goto end;
		}
	}

	if (connect(tcp_socket->socket, res->ai_addr, res->ai_addrlen) == -1) {
		TCP_DBG("Couldn't establish TCP connection, %s", tcp_socket, strerror(errno));
		goto end;
	}

	TCP_DBG1("Connection established", tcp_socket);
	rtval = TR_SUCCESS;

end:
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
	assert(tcp_sock != NULL);
	assert(tcp_sock->socket == -1);

	TCP_DBG1("Freeing socket", tcp_sock);

	lrtr_free(tcp_sock->config.host);
	lrtr_free(tcp_sock->config.port);
	lrtr_free(tcp_sock->config.bindaddr);

	if (tcp_sock->ident != NULL)
		lrtr_free(tcp_sock->ident);
	tr_sock->socket = NULL;
	lrtr_free(tcp_sock);
}

int tr_tcp_recv(const void *tr_tcp_sock, void *pdu, const size_t len, const time_t timeout)
{
	const struct tr_tcp_socket *tcp_socket = tr_tcp_sock;
	int rtval;
	if (timeout == 0)
		rtval = recv(tcp_socket->socket, pdu, len, MSG_DONTWAIT);
	else {
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
	if (timeout == 0)
		rtval = send(tcp_socket->socket, pdu, len, MSG_DONTWAIT);
	else {
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

	assert(sock != NULL);

	if (sock->ident != NULL)
		return sock->ident;

	len = strlen(sock->config.port) + strlen(sock->config.host) + 2;
	sock->ident = lrtr_malloc(len);
	if (sock->ident == NULL)
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
	if (config->bindaddr) {
		tcp_socket->config.bindaddr = lrtr_strdup(config->bindaddr);
	} else {
		tcp_socket->config.bindaddr = NULL;
	}
	tcp_socket->ident = NULL;

	return TR_SUCCESS;
}
