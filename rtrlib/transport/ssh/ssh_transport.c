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

#include <stdlib.h>
#include "ssh_transport.h"
#include <assert.h>
#include <stdbool.h>
#include <sys/time.h>
#include "../../lib/log.h"
#include "../../lib/utils.h"


#define SSH_DBG(fmt, sock, ...) dbg("SSH Transport(%s@%s:%u): " fmt, sock->config.username, sock->config.host, sock->config.port, ## __VA_ARGS__)
#define SSH_DBG1(a, sock) dbg("SSH Transport(%s@%s:%u): " a, sock->config.username, sock->config.host, sock->config.port)

typedef struct tr_ssh_socket {
    ssh_session session;
    ssh_channel channel;
    tr_ssh_config config;
} tr_ssh_socket;

static int tr_ssh_open(void *tr_ssh_sock);
static void tr_ssh_close(void *tr_ssh_sock);
static void tr_ssh_free(tr_socket *tr_sock);
static int tr_ssh_recv(const void *tr_ssh_sock, void *buf, const size_t buf_len, const time_t timeout);
static int tr_ssh_send(const void *tr_ssh_sock, const void *pdu, const size_t len, const time_t timeout);
static int tr_ssh_recv_async(const tr_ssh_socket *tr_ssh_sock, void *buf, const size_t buf_len);


int tr_ssh_init(const tr_ssh_config *config, tr_socket *socket) {

    socket->close_fp = &tr_ssh_close;
    socket->free_fp = &tr_ssh_free;
    socket->open_fp = &tr_ssh_open;
    socket->recv_fp = &tr_ssh_recv;
    socket->send_fp = &tr_ssh_send;;

    socket->socket = malloc(sizeof(tr_ssh_socket));
    tr_ssh_socket *ssh_socket = socket->socket;
    ssh_socket->channel = NULL;
    ssh_socket->session = NULL;
    ssh_socket->config = *config;

    return TR_SUCCESS;
}

int tr_ssh_open(void *socket) {
    tr_ssh_socket *ssh_socket = socket;
    const tr_ssh_config *config = &ssh_socket->config;

    assert(ssh_socket->channel == NULL);
    assert(ssh_socket->session == NULL);

    if((ssh_socket->session = ssh_new()) == NULL) {
        SSH_DBG1("tr_ssh_init: can't create ssh_session", ssh_socket);
        goto error;
    }

    const int verbosity = SSH_LOG_NOLOG;
    ssh_options_set(ssh_socket->session, SSH_OPTIONS_HOST, config->host);
    ssh_options_set(ssh_socket->session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(ssh_socket->session, SSH_OPTIONS_PORT, &(config->port));
    ssh_options_set(ssh_socket->session, SSH_OPTIONS_KNOWNHOSTS, (config->server_hostkey_path));

    if(ssh_connect(ssh_socket->session) == SSH_ERROR) {
        SSH_DBG1("tr_ssh_init: opening SSH connection failed", ssh_socket);
        goto error;
    }

    //check server identity
    if((config->server_hostkey_path != NULL) && (ssh_is_server_known(ssh_socket->session) != SSH_SERVER_KNOWN_OK)) {
        SSH_DBG1("tr_ssh_init: Wrong hostkey", ssh_socket);
        goto error;
    }

    //authenticate
    ssh_string pubkey = publickey_from_file(ssh_socket->session, config->client_pubkey_path, NULL);
    if(pubkey == NULL) {
        SSH_DBG1("tr_ssh_init: Loading public key failed", ssh_socket);
        goto error;
    }
    ssh_private_key privkey = privatekey_from_file(ssh_socket->session, config->client_privkey_path, 0, NULL);
    if(privkey == NULL) {
        SSH_DBG1("tr_ssh_init: Loading private key failed", ssh_socket);
        string_free(pubkey);
        goto error;
    }
    const int rtval = ssh_userauth_pubkey(ssh_socket->session, config->username, pubkey, privkey);
    string_free(pubkey);
    privatekey_free(privkey);
    if(rtval != SSH_AUTH_SUCCESS) {
        SSH_DBG1("tr_ssh_init: Authentication failed", ssh_socket);
        goto error;
    }

    if((ssh_socket->channel = channel_new(ssh_socket->session)) == NULL)
        goto error;

    if(channel_open_session(ssh_socket->channel) == SSH_ERROR)
        goto error;

    if(channel_request_subsystem(ssh_socket->channel, "rpki-rtr") == SSH_ERROR) {
        SSH_DBG1("tr_ssh_init: Error requesting subsystem rpki-rtr", ssh_socket);
        goto error;
    }
    SSH_DBG1("Connection established", ssh_socket);

    return TR_SUCCESS;

error:
    tr_ssh_close(ssh_socket);
    return TR_ERROR;

}

void tr_ssh_close(void *tr_ssh_sock) {
    tr_ssh_socket *socket = tr_ssh_sock;

    if(socket->channel != NULL) {
        if(channel_is_open(socket->channel))
            channel_close(socket->channel);
        channel_free(socket->channel);
        socket->channel = NULL;
    }
    if(socket->session != NULL) {
        ssh_disconnect(socket->session);
        ssh_free(socket->session);
        socket->session = NULL;
    }
    SSH_DBG1("Socket closed", socket);
}

void tr_ssh_free(tr_socket *tr_sock) {
    tr_ssh_socket *tr_ssh_sock = tr_sock->socket;
    assert(tr_ssh_sock->channel == NULL);
    assert(tr_ssh_sock->session == NULL);
    free(tr_ssh_sock);
    tr_sock->socket = NULL;
    SSH_DBG1("Socket freed", tr_ssh_sock);
}

/*
int tr_ssh_recv(const void* tr_ssh_sock, void* buf, unsigned int buf_len, const unsigned int timeout){
    if(timeout > 0){
        struct timeval timev;
        //timev.tv_sec = timeout;
        timev.tv_sec = 0;
        timev.tv_usec = 1;
        ssh_channel channels[2] = { (((tr_ssh_socket*) tr_ssh_sock)->channel), NULL };

        if(channel_select(channels, NULL, NULL, &timev) == SSH_OK){
        SSH_DBG("END");
            if(channels[0] == NULL)
                return -2;
        }
        SSH_DBG("END");
        //TODO: select kann INTr zurückgeben, wurde durch signal unterbrochen, dann verbleibende zeit berechnen und nochma
        //select ausführen
    }
    return channel_read_nonblocking(((tr_ssh_socket*) tr_ssh_sock)->channel, buf, buf_len, false);
}
*/
int tr_ssh_recv_async(const tr_ssh_socket *tr_ssh_sock, void *buf, const size_t buf_len) {
    const int rtval = channel_read_nonblocking(tr_ssh_sock->channel, buf, buf_len, false);
    if(rtval == 0) {
        if(channel_is_eof(tr_ssh_sock->channel) != 0) {
            SSH_DBG1("remote has sent EOF", tr_ssh_sock);
            return TR_ERROR;
        }
        else {
            return TR_WOULDBLOCK;
        }
    }
    else if(rtval == SSH_ERROR) {
        SSH_DBG1("recv(..) error", tr_ssh_sock);
        return TR_ERROR;
    }
    return rtval;
}

int tr_ssh_recv(const void *tr_ssh_sock, void *buf, const size_t buf_len, const time_t timeout) {
    if(timeout == 0)
        return tr_ssh_recv_async(tr_ssh_sock, buf, buf_len);

    time_t end_time;
    rtr_get_monotonic_time(&end_time);
    end_time += timeout;
    time_t cur_time;
    do {
        int rtval = channel_poll(((tr_ssh_socket *) tr_ssh_sock)->channel, false);
        if(rtval > 0)
            return tr_ssh_recv_async(tr_ssh_sock, buf, buf_len);
        else if(rtval == SSH_ERROR) {
            return TR_ERROR;
        }

        sleep(1);
        rtr_get_monotonic_time(&cur_time);
    } while((end_time - cur_time) >0);
    return TR_WOULDBLOCK;;
}

// channel_select is broken, it ignores the timeval parameter and blocks forever :/
/*
int tr_ssh_recv(const void* tr_ssh_sock, void* buf, const size_t buf_len, const time_t timeout){
    ssh_channel rchans[2] = { ((tr_ssh_socket*) tr_ssh_sock)->channel, NULL };

    struct timeval timev = { 1, 0 };

    const int rtval = channel_select(rchans, NULL, NULL, &timev);

    if(rtval == SSH_ERROR)
        return TR_ERROR;
    if(rtval == SSH_EINTR)
        return TR_INTR;
    if(channel_is_eof(((tr_ssh_socket*) tr_ssh_sock)->channel) != 0)
        return SSH_ERROR;

    if(rchans[0] == NULL)
        return TR_WOULDBLOCK;


    return tr_ssh_recv_async(tr_ssh_sock, buf, buf_len);
}
*/

int tr_ssh_send(const void *tr_ssh_sock, const void *pdu, const size_t len, const time_t timeout __attribute__((unused))) {
    return channel_write(((tr_ssh_socket *) tr_ssh_sock)->channel, pdu, len);
}
