/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "rtrlib/rtrlib.h"

static void print_usage(char **argv)
{
	printf("Usage:\n");
	printf(" %s tcp [options] <host> <port>\n", argv[0]);
	#ifdef RTRLIB_HAVE_LIBSSH
	printf(" %s ssh [options] <host> <port>", argv[0]);
	printf(" <username> <private_key> [<host_key>]\n");
	#endif
	printf("\nOptions:\n");
	printf("-k  Print information about SPKI updates.\n");
	printf("-p  Print information about PFX updates.\n");

	printf("\nExamples:\n");
	printf(" %s tcp rpki-validator.realmv6.org 8282\n", argv[0]);
	printf(" %s tcp -k -p rpki-validator.realmv6.org 8282\n", argv[0]);
	#ifdef RTRLIB_HAVE_LIBSSH
	printf(" %s ssh rpki-validator.realmv6.org 22 rtr-ssh", argv[0]);
	printf(" ~/.ssh/id_rsa ~/.ssh/known_hosts\n");
	printf(" %s ssh -k -p rpki-validator.realmv6.org 22 rtr-ssh", argv[0]);
	printf(" ~/.ssh/id_rsa ~/.ssh/known_hosts\n");
	#endif
}

static void status_fp(const struct rtr_mgr_group *group __attribute__((unused)),
		      enum rtr_mgr_status mgr_status,
		      const struct rtr_socket *rtr_sock,
		      void *data __attribute__((unused)))
{
	printf("RTR-Socket changed connection status to: %s, Mgr Status: %s\n",
	       rtr_state_to_str(rtr_sock->state),
	       rtr_mgr_status_to_str(mgr_status));
}

static void update_cb(struct pfx_table *p  __attribute__((unused)),
		      const struct pfx_record rec,
		      const bool added)
{
	char ip[INET6_ADDRSTRLEN];

	if (added)
		printf("+ ");
	else
		printf("- ");
	lrtr_ip_addr_to_str(&rec.prefix, ip, sizeof(ip));
	printf("%-40s   %3u - %3u   %10u\n",
	       ip, rec.min_len, rec.max_len, rec.asn);
}

static void update_spki(struct spki_table *s __attribute__((unused)),
			const struct spki_record record,
			const bool added)
{
	char c;

	if (added)
		c = '+';
	else
		c = '-';

	printf("%c ", c);
	printf("ASN:  %u\n  ", record.asn);

	int i;
	int size = sizeof(record.ski);

	printf("SKI:  ");
	for (i = 0; i < size; i++) {
		printf("%02x", record.ski[i]);
		if (i < size - 1)
			printf(":");
	}
	printf("\n  ");

	i = 0;
	size = sizeof(record.spki);
	printf("SPKI: ");
	for (i = 0; i < size; i++) {
		if ((i % 40 == 0) && (i != 0))
			printf("\n	");

		printf("%02x", record.spki[i]);
		if (i < size - 1)
			printf(":");
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	enum mode_t { TCP, SSH } mode = TCP;
	char *host = NULL;
	char *port = NULL;

	spki_update_fp spki_update_fp = NULL;
	pfx_update_fp pfx_update_fp = NULL;

	#ifdef RTRLIB_HAVE_LIBSSH
	char *user = NULL;
	char *privkey = NULL;
	char *hostkey = NULL;
	#endif

	if (argc == 1) {
		print_usage(argv);
		return EXIT_FAILURE;
	}

	if (strncasecmp(argv[1], "tcp", strlen(argv[1])) == 0) {
		mode = TCP;
	} else if (strncasecmp(argv[1], "ssh", strlen(argv[1])) == 0) {
		mode = SSH;
	} else {
		print_usage(argv);
		return EXIT_FAILURE;
	}

	int args_it = 2;
	/* Optional args */
	for (args_it = 2; args_it < argc; args_it++) {
		if (argv[args_it][0] == '-') {
			if (argv[args_it][1] == 'k') {
				spki_update_fp = update_spki;
			} else if (argv[args_it][1] == 'p') {
				pfx_update_fp = update_cb;
			} else {
				print_usage(argv);
				return EXIT_FAILURE;
			}
		} else {
			break;
		}
	}

	if (mode == TCP) {
		if ((argc - args_it) < 2) {
			print_usage(argv);
			return EXIT_FAILURE;
		}
		host = argv[args_it++];
		port = argv[args_it++];
	}
	#ifdef RTRLIB_HAVE_LIBSSH
	else if (mode == SSH) {
		if ((argc - args_it) < 4) {
			print_usage(argv);
			return EXIT_FAILURE;
		}
		host = argv[args_it++];
		port = argv[args_it++];
		user = argv[args_it++];
		privkey = argv[args_it++];
		if ((argc - args_it) == 1)
			hostkey = argv[args_it++];
		else
			hostkey = NULL;
	}
	#endif

	struct tr_socket tr_sock;
	struct tr_tcp_config tcp_config;

	#ifdef RTRLIB_HAVE_LIBSSH
	struct tr_ssh_config ssh_config;
	#endif

	if (mode == TCP) {
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
	struct rtr_mgr_config *conf;
	struct rtr_mgr_group groups[1];

	rtr.tr_socket = &tr_sock;
	groups[0].sockets_len = 1;
	groups[0].sockets = malloc(1 * sizeof(rtr));
	groups[0].sockets[0] = &rtr;
	groups[0].preference = 1;

	int ret = rtr_mgr_init(&conf, groups, 1, 30, 600, 600, pfx_update_fp,
			       spki_update_fp, status_fp, NULL);

	if (ret == RTR_ERROR)
		printf("Error in rtr_mgr_init!\n");
	else if (ret == RTR_INVALID_PARAM)
		printf("Invalid params passed to rtr_mgr_init\n");

	if (!conf)
		return EXIT_FAILURE;

	rtr_mgr_start(conf);
	printf("%-40s   %3s   %3s   %3s\n",
	       "Prefix", "Prefix Length", "", "ASN");
	pause();
	rtr_mgr_stop(conf);
	rtr_mgr_free(conf);
	free(groups[0].sockets);

	return EXIT_SUCCESS;
}
