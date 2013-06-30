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

#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "rtrlib/lib/log.h"
#include "rtrlib/transport/tcp/tcp_transport.h"

#define TCP_DBG(fmt, sock, ...) dbg("TCP Transport(%s:%s): " fmt, sock->config.host, sock->config.port, ## __VA_ARGS__)
#define TCP_DBG1(a, sock) dbg("TCP Transport(%s:%s): " a,sock->config.host, sock->config.port)

typedef struct tr_tcp_socket {
    int socket;
    tr_tcp_config config;
    char *ident;
} tr_tcp_socket;

static int tr_tcp_open(void *tr_tcp_sock);
static void tr_tcp_close(void *tr_tcp_sock);
static void tr_tcp_free(tr_socket *tr_sock);
static int tr_tcp_recv(const void *tr_tcp_sock, void *pdu, const size_t len, const time_t timeout);
static int tr_tcp_send(const void *tr_tcp_sock, const void *pdu, const size_t len, const time_t timeout);
static const char *tr_tcp_ident(void *socket);

int tr_tcp_open(void *tr_socket) {

    int rtval = TR_ERROR;
    tr_tcp_socket *tcp_socket = tr_socket;
    assert(tcp_socket->socket == -1);

    struct addrinfo hints;
    struct addrinfo *res;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG;
    if(getaddrinfo(tcp_socket->config.host, tcp_socket->config.port, &hints, &res) != 0) {
        TCP_DBG("getaddrinfo error, %s", tcp_socket, gai_strerror(errno));
        return TR_ERROR;
    }

    if ((tcp_socket->socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        TCP_DBG("Socket creation failed, %s", tcp_socket, strerror(errno));
        goto end;
    }

    if (connect(tcp_socket->socket, res->ai_addr, res->ai_addrlen) == -1) {
        TCP_DBG("Couldn't establish TCP connection, %s", tcp_socket, strerror(errno));
        goto end;
    }

    TCP_DBG1("Connection established", tcp_socket);
    rtval = TR_SUCCESS;

end:
    freeaddrinfo(res);
    if(rtval == -1)
        tr_tcp_close(tr_socket);
    return rtval;
}

void tr_tcp_close(void *tr_tcp_sock) {
    tr_tcp_socket *tcp_socket = tr_tcp_sock;
    if(tcp_socket->socket != -1)
        close(tcp_socket->socket);
    TCP_DBG1("Socket closed", tcp_socket);
    tcp_socket->socket = -1;
}

void tr_tcp_free(tr_socket *tr_sock) {
    tr_tcp_socket *tcp_sock = tr_sock->socket;
    assert(tcp_sock != NULL);
    assert(tcp_sock->socket == -1);

    if (tcp_sock->ident != NULL)
        free(tcp_sock->ident);
    free(tcp_sock);
    tr_sock->socket = NULL;
    TCP_DBG1("Socket freed", tcp_sock);
}

int tr_tcp_recv(const void *tr_tcp_sock, void *pdu, const size_t len, const time_t timeout) {
    const tr_tcp_socket *tcp_socket = tr_tcp_sock;
    int rtval;
    if(timeout == 0)
        rtval = recv(tcp_socket->socket, pdu, len, MSG_DONTWAIT);
    else {
        struct timeval t = { timeout, 0 };
        if(setsockopt(tcp_socket->socket, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t)) == -1) {
            TCP_DBG("setting SO_RCVTIMEO failed, %s", tcp_socket, strerror(errno));
            return TR_ERROR;
        }
        rtval = recv(tcp_socket->socket, pdu, len, 0);
    }

    if(rtval == -1) {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
            return TR_WOULDBLOCK;
        if(errno == EINTR)
            return TR_INTR;
        TCP_DBG("recv(..) error: %s", tcp_socket, strerror(errno));
        return TR_ERROR;
    }
    if(rtval == 0)
        return TR_ERROR;
    return rtval;
}

int tr_tcp_send(const void *tr_tcp_sock, const void *pdu, const size_t len, const time_t timeout) {
    const tr_tcp_socket *tcp_socket = tr_tcp_sock;
    int rtval;
    if(timeout == 0)
        rtval = send(tcp_socket->socket, pdu, len, MSG_DONTWAIT);
    else {
        struct timeval t = { timeout, 0 };
        if(setsockopt(tcp_socket->socket, SOL_SOCKET, SO_SNDTIMEO, &t, sizeof(t)) == -1) {
            TCP_DBG("setting SO_SNDTIMEO failed, %s", tcp_socket, strerror(errno));
            return TR_ERROR;
        }
        rtval = send(tcp_socket->socket, pdu, len, 0);
    }

    if(rtval == -1) {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
            return TR_WOULDBLOCK;
        if(errno == EINTR)
            return TR_INTR;
        TCP_DBG("send(..) error: %s", tcp_socket, strerror(errno));
        return TR_ERROR;
    }
    if(rtval == 0)
        return TR_ERROR;
    return rtval;
}

const char *tr_tcp_ident(void *socket) {
    size_t len;
    tr_tcp_socket *sock = socket;

    assert(socket != NULL);

    if (sock->ident != NULL)
        return sock->ident;

    len = strlen(sock->config.port) + strlen(sock->config.host) + 2;
    sock->ident = malloc(len);
    if (sock->ident== NULL)
        return NULL;
    snprintf(sock->ident, len, "%s:%s", sock->config.host, sock->config.port);

    return sock->ident;
}

int tr_tcp_init(const tr_tcp_config *config, tr_socket *socket) {

    socket->close_fp = &tr_tcp_close;
    socket->free_fp = &tr_tcp_free;
    socket->open_fp = &tr_tcp_open;
    socket->recv_fp = &tr_tcp_recv;
    socket->send_fp = &tr_tcp_send;
    socket->ident_fp = &tr_tcp_ident;

    socket->socket = malloc(sizeof(tr_tcp_socket));
    tr_tcp_socket *tcp_socket = socket->socket;
    tcp_socket->socket = -1;
    tcp_socket->config = *config;
    tcp_socket->ident = NULL;

    return TR_SUCCESS;
}
