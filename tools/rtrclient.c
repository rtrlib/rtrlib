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
    printf(" %s ssh rpki.realmv6.org rtr-ssh ~/.ssh/id_rsa ~/.ssh/id_rsa.pub\n", argv[0]);
#endif

}

/*
static void state_cb(const rtr_socket* sock  __attribute__((unused)), const rtr_socket_state state, void* cb_data  __attribute__((unused))){
#ifdef NDEBUG
    printf("Socket State: ");
    switch(state)
    {
        case RTR_CONNECTING:
            printf("RTR_CONNECTING\n");
            break;
        case RTR_ESTABLISHED:
            printf("RTR_ESTABLISHED\n");
            break;
        case RTR_RESET:
            printf("RTR_RESET\n");
            break;
        case RTR_SYNC:
            printf("RTR_SYNC\n");
            break;
        case RTR_ERROR_NO_DATA_AVAIL:
            printf("RTR_ERROR_NO_DATA_AVAIL\n");
            break;
        case RTR_ERROR_NO_INCR_UPDATE_AVAIL:
            printf("RTR_ERROR_NO_INCR_UPDATE_AVAIL\n");
            break;
        case RTR_ERROR_FATAL:
            printf("RTR_ERROR_FATAL\n");
            break;
        case RTR_ERROR_TRANSPORT:
            printf("RTR_ERROR_TRANSPORT\n");
            break;
        case RTR_SHUTDOWN:
            printf("RTR_SHUTDOWN\n");
            break;
    }
#endif
}
*/

static void update_cb(struct pfx_table* p  __attribute__((unused)), const pfx_record rec, const bool added){
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
    char* user;
    char* privkey;
    char* pubkey;
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
    else if(strncasecmp(argv[1], "ssh", strlen(argv[1])) == 0){
#ifndef RTRLIB_HAVE_LIBSSH
        print_usage(argv);
        return(EXIT_FAILURE);
#endif
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
    else{
        print_usage(argv);
        return(EXIT_FAILURE);
    }

    tr_socket tr_sock;
    if(mode == TCP){
        tr_tcp_config config = {
            host,
            port,
        };
        tr_tcp_init(&config, &tr_sock);
    }
#ifdef RTRLIB_HAVE_LIBSSH
    else{
        unsigned int iport = atoi(port);
        tr_ssh_config config = {
            host,
            iport,
            user,
            NULL,
            privkey,
            pubkey
        };
    tr_ssh_init(&config, &tr_sock);
    }
#endif

    rtr_socket rtr;
    rtr.tr_socket = &tr_sock;
    rtr_mgr_config *conf;

    rtr_mgr_group groups[1];
    groups[0].sockets_len = 1;
    groups[0].sockets = malloc(1 * sizeof(rtr_socket*));
    groups[0].sockets[0] = &rtr;
    groups[0].preference = 1;

    conf = rtr_mgr_init(groups, 1, 30, 520, &update_cb, NULL, NULL);
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
