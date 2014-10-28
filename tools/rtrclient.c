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

static void print_usage(char** argv)
{
    printf("Usage:\n");
    printf(" %s tcp <host> <port> [options]\n", argv[0]);
#ifdef RTRLIB_HAVE_LIBSSH
    printf(" %s ssh <host> <port> <username> <private_key> [<host_key>] \n", argv[0]);
#endif
    printf("\nOptions:\n");
    printf("-K  Print inforamtions about SPKI updates.\n");
    printf("-P  Print informations about PFX updates.\n");

    printf("\nExamples:\n");
    printf(" %s tcp rpki.realmv6.org 42420\n", argv[0]);
    printf(" %s tcp rpki.realmv6.org 42420 -K -P\n", argv[0]);
#ifdef RTRLIB_HAVE_LIBSSH
    printf(" %s ssh rpki.realmv6.org 22 rtr-ssh ~/.ssh/id_rsa ~/.ssh/known_hosts\n", argv[0]);
#endif

}

static void status_fp(const struct rtr_mgr_group *group __attribute__((unused)),
                      enum rtr_mgr_status mgr_status, const struct rtr_socket *rtr_sock,
                      void *data __attribute__((unused)))
{
    printf("RTR-Socket changed connection status to: %s, Mgr Status: %s\n",
           rtr_state_to_str(rtr_sock->state),
           rtr_mgr_status_to_str(mgr_status));
}

static void update_cb(struct pfx_table* p  __attribute__((unused)), const struct pfx_record rec, const bool added)
{
    char ip[INET6_ADDRSTRLEN];
    if(added)
        printf("+ ");
    else
        printf("- ");
    ip_addr_to_str(&(rec.prefix), ip, sizeof(ip));
    printf("%-40s   %3u - %3u   %10u\n", ip, rec.min_len, rec.max_len, rec.asn);
}

static void update_spki(struct spki_table* s __attribute__((unused)), const struct spki_record record, const bool added)
{
    char c;
    if(added)
        c = '+';
    else
        c = '-';

    printf("%c ",c);
    printf("ASN:  %u\n  ",record.asn);

    int i;
    int size = sizeof(record.ski);
    printf("SKI:  ");
    for(i = 0; i<size; i++) {
        printf("%02x",record.ski[i]);
        if(i < size-1)
            printf(":");
    }
    printf("\n  ");

    i = 0;
    size = sizeof(record.spki);
    printf("SPKI: ");
    for(i = 0; i<size; i++) {
        if(i % 40 == 0 && i != 0)
            printf("\n        ");

        printf("%02x",record.spki[i]);
        if(i < size-1)
            printf(":");
    }
    printf("\n");
}

int main(int argc, char** argv)
{
    enum mode_t { TCP, SSH } mode;
    char* host;
    char* port;
    spki_update_fp spki_update_fp = NULL;
    pfx_update_fp pfx_update_fp = NULL;
#ifdef RTRLIB_HAVE_LIBSSH
    char* user;
    char* privkey;
    char* hostkey;

#endif
    if(argc == 1) {
        print_usage(argv);
        return(EXIT_FAILURE);
    }

    if(strncasecmp(argv[1], "tcp", strlen(argv[1])) == 0) {
        if(argc < 4) {
            print_usage(argv);
            return(EXIT_FAILURE);
        }
        mode = TCP;
        host = argv[2];
        port = argv[3];
        int i;
        for(i=4; i<argc; i++) {
            if(argv[i][0] == '-') {
                if(argv[i][1] == 'K')
                    spki_update_fp = update_spki;
                if(argv[i][1] == 'P')
                    pfx_update_fp = update_cb;
            }
        }

    }
#ifdef RTRLIB_HAVE_LIBSSH
    else if(strncasecmp(argv[1], "ssh", strlen(argv[1])) == 0) {
        if(argc < 6) {
            print_usage(argv);
            return(EXIT_FAILURE);
        }

        mode = SSH;
        host = argv[2];
        port = argv[3];
        user = argv[4];
        privkey = argv[5];
        if (argc == 7)
            hostkey = argv[6];
        else
            hostkey = NULL;
    }
#endif
    else {
        print_usage(argv);
        return(EXIT_FAILURE);
    }



    struct tr_socket tr_sock;
    struct tr_tcp_config tcp_config;
#ifdef RTRLIB_HAVE_LIBSSH
    struct tr_ssh_config ssh_config;
#endif
    if(mode == TCP){
        tcp_config = (struct tr_tcp_config) { host, port, 0 };
        tr_tcp_init(&tcp_config, &tr_sock);
    }
#ifdef RTRLIB_HAVE_LIBSSH
    else {
        unsigned int iport = atoi(port);
        ssh_config = (struct tr_ssh_config) {
            host,
            iport,
            NULL,
            user,
            hostkey,
            privkey,
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

    conf = rtr_mgr_init(groups, 1, 30, 520, pfx_update_fp, spki_update_fp, status_fp, NULL);
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
