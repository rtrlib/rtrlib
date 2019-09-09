/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "ssh_transport_private.h"

#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/lib/log_private.h"
#include "rtrlib/lib/utils_private.h"
#include "rtrlib/rtrlib_export_private.h"
#include "rtrlib/transport/transport_private.h"

#include <assert.h>
#include <libssh/libssh.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define SSH_DBG(fmt, sock, ...)                                                                                     \
	do {                                                                                                        \
		const struct tr_ssh_socket *tmp = sock;                                                             \
		lrtr_dbg("SSH Transport(%s@%s:%u): " fmt, tmp->config.username, tmp->config.host, tmp->config.port, \
			 ##__VA_ARGS__);                                                                            \
	} while (0)
#define SSH_DBG1(a, sock) SSH_DBG(a, sock)

struct tr_ssh_socket {
	ssh_session session;
	ssh_channel channel;
	struct tr_ssh_config config;
	char *ident;
};

static int tr_ssh_open(void *tr_ssh_sock);
static void tr_ssh_close(void *tr_ssh_sock);
static void tr_ssh_free(struct tr_socket *tr_sock);
static int tr_ssh_recv(const void *tr_ssh_sock, void *buf, const size_t buf_len, const time_t timeout);
static int tr_ssh_send(const void *tr_ssh_sock, const void *pdu, const size_t len, const time_t timeout);
static int tr_ssh_recv_async(const struct tr_ssh_socket *tr_ssh_sock, void *buf, const size_t buf_len);
static const char *tr_ssh_ident(void *tr_ssh_sock);

int tr_ssh_open(void *socket)
{
	struct tr_ssh_socket *ssh_socket = socket;
	const struct tr_ssh_config *config = &ssh_socket->config;

	assert(!ssh_socket->channel);
	assert(!ssh_socket->session);

	ssh_socket->session = ssh_new();
	if (!ssh_socket->session) {
		SSH_DBG1("tr_ssh_init: can't create ssh_session", ssh_socket);
		goto error;
	}

	const int verbosity = SSH_LOG_NOLOG;

	ssh_options_set(ssh_socket->session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

	ssh_options_set(ssh_socket->session, SSH_OPTIONS_HOST, config->host);
	ssh_options_set(ssh_socket->session, SSH_OPTIONS_PORT, &(config->port));
	ssh_options_set(ssh_socket->session, SSH_OPTIONS_BINDADDR, config->bindaddr);
	ssh_options_set(ssh_socket->session, SSH_OPTIONS_USER, config->username);

	if (config->server_hostkey_path)
		ssh_options_set(ssh_socket->session, SSH_OPTIONS_KNOWNHOSTS, config->server_hostkey_path);

	if (config->client_privkey_path)
		ssh_options_set(ssh_socket->session, SSH_OPTIONS_IDENTITY, config->client_privkey_path);

	if (ssh_connect(ssh_socket->session) == SSH_ERROR) {
		SSH_DBG1("tr_ssh_init: opening SSH connection failed", ssh_socket);
		goto error;
	}

	// check server identity
	if ((config->server_hostkey_path) && (ssh_is_server_known(ssh_socket->session) != SSH_SERVER_KNOWN_OK)) {
		SSH_DBG1("tr_ssh_init: Wrong hostkey", ssh_socket);
		goto error;
	}

#if LIBSSH_VERSION_MAJOR > 0 || LIBSSH_VERSION_MINOR > 5
	const int rtval = ssh_userauth_publickey_auto(ssh_socket->session, NULL, NULL);
#else /* else use libSSH version 0.5.0 */
	const int rtval = ssh_userauth_autopubkey(ssh_socket->session, NULL);
#endif
	if (rtval != SSH_AUTH_SUCCESS) {
		SSH_DBG1("tr_ssh_init: Authentication failed", ssh_socket);
		goto error;
	}

	ssh_socket->channel = ssh_channel_new(ssh_socket->session);
	if (!ssh_socket->channel)
		goto error;

	if (ssh_channel_open_session(ssh_socket->channel) == SSH_ERROR)
		goto error;

	if (ssh_channel_request_subsystem(ssh_socket->channel, "rpki-rtr") == SSH_ERROR) {
		SSH_DBG1("tr_ssh_init: Error requesting subsystem rpki-rtr", ssh_socket);
		goto error;
	}
	SSH_DBG1("Connection established", ssh_socket);

	return TR_SUCCESS;

error:
	tr_ssh_close(ssh_socket);
	return TR_ERROR;
}

void tr_ssh_close(void *tr_ssh_sock)
{
	struct tr_ssh_socket *socket = tr_ssh_sock;

	if (socket->channel) {
		if (ssh_channel_is_open(socket->channel))
			ssh_channel_close(socket->channel);
		ssh_channel_free(socket->channel);
		socket->channel = NULL;
	}
	if (socket->session) {
		ssh_disconnect(socket->session);
		ssh_free(socket->session);
		socket->session = NULL;
	}
	SSH_DBG1("Socket closed", socket);
}

void tr_ssh_free(struct tr_socket *tr_sock)
{
	struct tr_ssh_socket *tr_ssh_sock = tr_sock->socket;

	assert(!tr_ssh_sock->channel);
	assert(!tr_ssh_sock->session);

	SSH_DBG1("Freeing socket", tr_ssh_sock);

	lrtr_free(tr_ssh_sock->config.host);
	lrtr_free(tr_ssh_sock->config.bindaddr);
	lrtr_free(tr_ssh_sock->config.username);
	lrtr_free(tr_ssh_sock->config.client_privkey_path);
	lrtr_free(tr_ssh_sock->config.server_hostkey_path);

	if (tr_ssh_sock->ident)
		lrtr_free(tr_ssh_sock->ident);
	lrtr_free(tr_ssh_sock);
	tr_sock->socket = NULL;
}

int tr_ssh_recv_async(const struct tr_ssh_socket *tr_ssh_sock, void *buf, const size_t buf_len)
{
	const int rtval = ssh_channel_read_nonblocking(tr_ssh_sock->channel, buf, buf_len, false);

	if (rtval == 0) {
		if (ssh_channel_is_eof(tr_ssh_sock->channel) != 0) {
			SSH_DBG1("remote has sent EOF", tr_ssh_sock);
			return TR_CLOSED;
		} else {
			return TR_WOULDBLOCK;
		}
	} else if (rtval == SSH_ERROR) {
		SSH_DBG1("recv(..) error", tr_ssh_sock);
		return TR_ERROR;
	}
	return rtval;
}

int tr_ssh_recv(const void *tr_ssh_sock, void *buf, const size_t buf_len, const time_t timeout)
{
	ssh_channel rchans[2] = {((struct tr_ssh_socket *)tr_ssh_sock)->channel, NULL};

	struct timeval timev = {1, 0};

	if (ssh_channel_select(rchans, NULL, NULL, &timev) == SSH_EINTR)
		return TR_INTR;

	if (ssh_channel_is_eof(((struct tr_ssh_socket *)tr_ssh_sock)->channel) != 0)
		return SSH_ERROR;

	if (!rchans[0])
		return TR_WOULDBLOCK;

	return tr_ssh_recv_async(tr_ssh_sock, buf, buf_len);
}

int tr_ssh_send(const void *tr_ssh_sock, const void *pdu, const size_t len,
		const time_t timeout __attribute__((unused)))
{
	return ssh_channel_write(((struct tr_ssh_socket *)tr_ssh_sock)->channel, pdu, len);
}

const char *tr_ssh_ident(void *tr_ssh_sock)
{
	size_t len;
	struct tr_ssh_socket *sock = tr_ssh_sock;

	assert(sock);

	if (sock->ident)
		return sock->ident;

	len = strlen(sock->config.username) + 1 + strlen(sock->config.host) + 1 + 5 + 1;
	sock->ident = lrtr_malloc(len);
	if (!sock->ident)
		return NULL;
	snprintf(sock->ident, len, "%s@%s:%u", sock->config.username, sock->config.host, sock->config.port);
	return sock->ident;
}

RTRLIB_EXPORT int tr_ssh_init(const struct tr_ssh_config *config, struct tr_socket *socket)
{
	socket->close_fp = &tr_ssh_close;
	socket->free_fp = &tr_ssh_free;
	socket->open_fp = &tr_ssh_open;
	socket->recv_fp = &tr_ssh_recv;
	socket->send_fp = &tr_ssh_send;
	socket->ident_fp = &tr_ssh_ident;

	socket->socket = lrtr_malloc(sizeof(struct tr_ssh_socket));
	struct tr_ssh_socket *ssh_socket = socket->socket;

	ssh_socket->channel = NULL;
	ssh_socket->session = NULL;
	ssh_socket->config.host = lrtr_strdup(config->host);
	ssh_socket->config.port = config->port;
	ssh_socket->config.username = lrtr_strdup(config->username);

	if (config->bindaddr)
		ssh_socket->config.bindaddr = lrtr_strdup(config->bindaddr);
	else
		ssh_socket->config.bindaddr = NULL;

	if (config->client_privkey_path)
		ssh_socket->config.client_privkey_path = lrtr_strdup(config->client_privkey_path);
	else
		ssh_socket->config.client_privkey_path = NULL;

	if (config->server_hostkey_path)
		ssh_socket->config.server_hostkey_path = lrtr_strdup(config->server_hostkey_path);
	else
		ssh_socket->config.server_hostkey_path = NULL;

	ssh_socket->ident = NULL;
	;

	return TR_SUCCESS;
}
