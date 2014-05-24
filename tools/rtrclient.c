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
#include <pthread.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "rtrlib/rtrlib.h"

static void print_usage(char** argv){
    printf("Usage:\n");
    printf(" %s tcp <host> <port>\n", argv[0]);
#ifdef RTRLIB_HAVE_LIBSSH
    printf(" %s ssh <host> <port> <username> <private_key> <public_key> \n", argv[0]);
#endif
    printf("\nExamples:\n");
    printf(" %s tcp rpki.realmv6.org 42420\n", argv[0]);
#ifdef RTRLIB_HAVE_LIBSSH
    printf(" %s ssh rpki.realmv6.org 22 rtr-ssh ~/.ssh/id_rsa ~/.ssh/id_rsa.pub\n", argv[0]);
#endif

}

static void status_fp(const struct rtr_mgr_group *group __attribute__((unused)),
			   rtr_mgr_status mgr_status, const struct rtr_socket *rtr_sock,
			   void *data __attribute__((unused)))
{
	printf("RTR-Socket changed connection status to: %s, Mgr Status: %s\n",
		   rtr_state_to_str(rtr_sock->state),
		   rtr_mgr_status_to_str(mgr_status));
}

static void update_cb(struct pfx_table* p  __attribute__((unused)), const struct pfx_record rec, const bool added){
    char ip[INET6_ADDRSTRLEN];
    if(added)
        printf("+ ");
    else
        printf("- ");
    ip_addr_to_str(&(rec.prefix), ip, sizeof(ip));
    printf("%-40s   %3u - %3u   %10u\n", ip, rec.min_len, rec.max_len, rec.asn);
}

int main(int argc, char** argv){
    enum mode_t { TCP, SSH } mode;
    char* host;
    char* port;
#ifdef RTRLIB_HAVE_LIBSSH
    char* user;
    char* privkey;
    char* pubkey;
#endif
    if(argc == 1){
        print_usage(argv);
        return(EXIT_FAILURE);
    }

    if(strncasecmp(argv[1], "tcp", strlen(argv[1])) == 0){
        if(argc != 4){
            print_usage(argv);
            return(EXIT_FAILURE);
        }
        mode = TCP;
        host = argv[2];
        port = argv[3];

    }
#ifdef RTRLIB_HAVE_LIBSSH
    else if(strncasecmp(argv[1], "ssh", strlen(argv[1])) == 0){
        if(argc != 7){
            print_usage(argv);
            return(EXIT_FAILURE);
        }

        mode = SSH;
        host = argv[2];
        port = argv[3];
        user = argv[4];
        privkey = argv[5];
        pubkey = argv[6];
    }
#endif
    else{
        print_usage(argv);
        return(EXIT_FAILURE);
    }

    struct tr_socket tr_sock;
    struct tr_tcp_config tcp_config;
#ifdef RTRLIB_HAVE_LIBSSH
    struct tr_ssh_config ssh_config;
#endif
    if(mode == TCP){
        tcp_config = (struct tr_tcp_config) { host, port };
        tr_tcp_init(&tcp_config, &tr_sock);
    }
#ifdef RTRLIB_HAVE_LIBSSH
    else{
        unsigned int iport = atoi(port);
        ssh_config = (struct tr_ssh_config) {
            host,
            iport,
            user,
            NULL,
            privkey,
            pubkey
        };
    tr_ssh_init(&ssh_config, &tr_sock);
    }
#endif

    struct rtr_socket rtr;
    rtr.tr_socket = &tr_sock;
    struct rtr_mgr_config *conf;

    struct rtr_mgr_group groups[1];
    groups[0].sockets_len = 1;
    groups[0].sockets = malloc(1 * sizeof(rtr));
    groups[0].sockets[0] = &rtr;
    groups[0].preference = 1;

    conf = rtr_mgr_init(groups, 1, 30, 520, &update_cb, status_fp, NULL);
    if (conf == NULL)
	    return EXIT_FAILURE;
    rtr_mgr_start(conf);
    printf("%-40s   %3s   %3s   %3s\n", "Prefix", "Prefix Length", "", "ASN");
    pause();
    rtr_mgr_stop(conf);
    rtr_mgr_free(conf);
    free(groups[0].sockets);

    return(EXIT_SUCCESS);
}
