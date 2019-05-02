/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "rtrlib/rtrlib.h"

enum socket_type {
	SOCKET_TYPE_TCP,
#ifdef RTRLIB_HAVE_LIBSSH
	SOCKET_TYPE_SSH,
#endif
};

struct socket_config {
	struct rtr_socket socket;
	struct tr_socket tr_socket;
	enum socket_type type;
	bool print_pfx_updates;
	bool print_spki_updates;
	char *bindaddr;
	char *host;
	char *port;
	char *ssh_username;
	char *ssh_private_key;
	char *ssh_host_key;
};

/* activate update callback */
bool activate_pfx_update_cb = false;
bool activate_spki_update_cb = false;

/* print all updates regardless of socket configuration */
bool print_all_pfx_updates = false;
bool print_all_spki_updates = false;

struct socket_config **socket_config = NULL;
size_t socket_count = 0;

pthread_mutex_t stdout_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Check if argument is NULL and exit if it is
 */
static void *check_alloc_ret(void *ptr)
{
	if (ptr)
		return ptr;

	fprintf(stderr, "Memory allocation error\n");
	exit(-1);
}

/**
 * @brief Return value checking malloc wrapper. Exits program on error
 */
static inline void *checked_malloc(size_t size)
{
	return check_alloc_ret(malloc(size));
}

/**
 * @brief Return value checking realloc wrapper. Exits program on error.
 */
static inline void *checked_realloc(void *ptr, size_t size)
{
	return check_alloc_ret(realloc(ptr, size));
}

static struct socket_config *extend_socket_config()
{
	socket_config = checked_realloc(
			socket_config,
			sizeof(struct socket_config *) * (socket_count + 1));
	struct socket_config *config = socket_config[socket_count] =
		checked_malloc(sizeof(struct socket_config));

	memset(config, 0, sizeof(*config));
	++socket_count;

	return config;
}

static bool is_numeric(const char *str)
{
	for (size_t i = 0; i < strlen(str); ++i) {
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

static bool is_valid_port_number(const char *str)
{
	if (!is_numeric(str))
		return false;

	int port = atoi(str);

	if (port < 1 || port > 65535)
		return false;

	return true;
}

static bool is_resolveable_host(const char *str)
{
	struct addrinfo hints;
	struct addrinfo *result = NULL;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	int ret = getaddrinfo(str, NULL, &hints, &result);

	freeaddrinfo(result);

	if (ret != 0)
		return false;
	return true;
}

#ifdef RTRLIB_HAVE_LIBSSH
static bool is_readable_file(const char *str)
{
	if (access(str, R_OK) != 0)
		return false;

	struct stat stat_buf;

	stat(str, &stat_buf);

	return S_ISREG(stat_buf.st_mode);
}
#endif

static void print_usage(char **argv)
{
	printf("Usage:\n");
	printf(" %s [-hpk] tcp [-hpk] [-b bindaddr] <host> <port>\n", argv[0]);
	#ifdef RTRLIB_HAVE_LIBSSH
	printf(" %s ssh [options] <host> <port>", argv[0]);
	printf(" <username> <private_key> [<host_key>]\n");
	#endif
	printf("\nOptions:\n");
	printf("-b bindaddr  Hostnamne or IP address to connect from\n\n");

	printf("-k  Print information about SPKI updates.\n");
	printf("-p  Print information about PFX updates.\n\n");

	printf("-h  Print this help message.\n");

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
	pthread_mutex_lock(&stdout_mutex);
	printf("RTR-Socket changed connection status to: %s, Mgr Status: %s\n",
	       rtr_state_to_str(rtr_sock->state),
	       rtr_mgr_status_to_str(mgr_status));
	pthread_mutex_unlock(&stdout_mutex);
}

static void update_cb(struct pfx_table *p  __attribute__((unused)),
		      const struct pfx_record rec,
		      const bool added)
{
	char ip[INET6_ADDRSTRLEN];

	const struct socket_config *config =
		(const struct socket_config *)rec.socket;

	if (!print_all_pfx_updates && !config->print_pfx_updates)
		return;

	pthread_mutex_lock(&stdout_mutex);
	if (added)
		printf("+ ");
	else
		printf("- ");
	lrtr_ip_addr_to_str(&rec.prefix, ip, sizeof(ip));
	if (socket_count > 1)
		printf("%s:%s %-40s   %3u - %3u   %10u\n",
		       config->host, config->port, ip,
		       rec.min_len, rec.max_len, rec.asn);
	else
		printf("%-40s   %3u - %3u   %10u\n",
		       ip, rec.min_len, rec.max_len, rec.asn);

	pthread_mutex_unlock(&stdout_mutex);
}

static void update_spki(struct spki_table *s __attribute__((unused)),
			const struct spki_record record,
			const bool added)
{
	const struct socket_config *config =
		(const struct socket_config *)record.socket;

	if (!print_all_spki_updates && !config->print_spki_updates)
		return;

	pthread_mutex_lock(&stdout_mutex);

	char c;

	if (added)
		c = '+';
	else
		c = '-';

	printf("%c ", c);
	printf("HOST:  %s:%s\n", config->host, config->port);
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

	pthread_mutex_unlock(&stdout_mutex);
}

static void parse_global_opts(int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "+kph")) != -1) {
		switch (opt) {
		case 'k':
			activate_spki_update_cb = true;
			print_all_spki_updates = true;
			break;

		case 'p':
			activate_pfx_update_cb = true;
			print_all_pfx_updates = true;
			break;

		default:
			print_usage(argv);
			exit(EXIT_FAILURE);
		}
	}
}

static void parse_socket_opts(
		int argc,
		char **argv,
		struct socket_config *config)
{
	int opt;

	while ((opt = getopt(argc, argv, "+kphb:")) != -1) {
		switch (opt) {
		case 'k':
			activate_spki_update_cb = true;
			config->print_spki_updates = true;
			break;

		case 'p':
			activate_pfx_update_cb = true;
			config->print_pfx_updates = true;
			break;

		case 'b':
			config->bindaddr = optarg;
			break;

		default:
			print_usage(argv);
			exit(EXIT_FAILURE);
		}
	}
}

__attribute__((format (printf, 1, 2)))
static void print_error_exit(const char *fmt, ...)
{
	va_list argptr;

	va_start(argptr, fmt);

	fprintf(stderr, "error: ");

	vfprintf(stderr, fmt, argptr);

	va_end(argptr);

	exit(EXIT_FAILURE);
}

static int parse_cli(int argc, char **argv) {
	enum cli_parse_state {
		CLI_PARSE_STATE_BEGIN,
		CLI_PARSE_STATE_GLOBAL_OPTS,
		CLI_PARSE_STATE_SOCKET_BEGIN,
		CLI_PARSE_STATE_SOCKET_TCP_OPTIONS,
		CLI_PARSE_STATE_SOCKET_TCP_HOST,
		CLI_PARSE_STATE_SOCKET_TCP_PORT,
#ifdef RTRLIB_HAVE_LIBSSH
		CLI_PARSE_STATE_SOCKET_SSH_OPTIONS,
		CLI_PARSE_STATE_SOCKET_SSH_HOST,
		CLI_PARSE_STATE_SOCKET_SSH_PORT,
		CLI_PARSE_STATE_SOCKET_SSH_USERNAME,
		CLI_PARSE_STATE_SOCKET_SSH_PRIVATE_KEY,
		CLI_PARSE_STATE_SOCKET_SSH_HOST_KEY,
#endif
		CLI_PARSE_STATE_END,
	};

	enum cli_parse_state state = CLI_PARSE_STATE_BEGIN;

	struct socket_config *current_config;

	while (true) {
		switch (state) {
		case CLI_PARSE_STATE_BEGIN:
			optind = 1;
			state = CLI_PARSE_STATE_GLOBAL_OPTS;
			break;

		case CLI_PARSE_STATE_GLOBAL_OPTS:
			parse_global_opts(argc, argv);

			state = CLI_PARSE_STATE_SOCKET_BEGIN;
			break;

		case CLI_PARSE_STATE_SOCKET_BEGIN:
			current_config = extend_socket_config();

			if (strncasecmp(argv[optind],
					"tcp", strlen(argv[optind])) == 0) {
				state = CLI_PARSE_STATE_SOCKET_TCP_OPTIONS;
				current_config->type = SOCKET_TYPE_TCP;

				/* Check if enough arguments are available
				 * to configure this socket.
				 * This does not account for options.
				 */
				if ((argc - optind) < 2)
					print_error_exit("Not enough arguments for tcp socket");

			} else if (strncasecmp(argv[optind], "ssh",
					       strlen(argv[optind])) == 0) {
#ifdef RTRLIB_HAVE_LIBSSH
				state = CLI_PARSE_STATE_SOCKET_SSH_OPTIONS;
				current_config->type = SOCKET_TYPE_SSH;

				/* Check if enough arguments are available
				 * to configure this socket.
				 * This does not account for options.
				 */
				if ((argc - optind) < 4)
					print_error_exit("Not enough arguments for ssh socket");
#else
				print_error_exit("ssh support disabled at compile time\n");
#endif

			} else {
				print_error_exit(
					"\"%s\" is not a valid socket type\n",
					argv[optind]);
				break;
			}

			++optind;
			break;

		case CLI_PARSE_STATE_SOCKET_TCP_OPTIONS:
			parse_socket_opts(argc, argv, current_config);

			/* Check again if enough arguments are available,
			 * accounting for options
			 */
			if ((argc - optind) < 2)
				print_error_exit(
					"Not enough arguments for tcp socket");

			state = CLI_PARSE_STATE_SOCKET_TCP_HOST;
			break;

		case CLI_PARSE_STATE_SOCKET_TCP_HOST:
			if (!is_resolveable_host(argv[optind]))
				print_error_exit(
					"cannot resolve \"%s\"\n",
					argv[optind]);

			current_config->host = argv[optind++];

			state = CLI_PARSE_STATE_SOCKET_TCP_PORT;
			break;

		case CLI_PARSE_STATE_SOCKET_TCP_PORT:
			if (!is_valid_port_number(argv[optind]))
				print_error_exit(
					"\"%s\" is not a valid port number\n",
					argv[optind]);

			current_config->port = argv[optind++];

			state = CLI_PARSE_STATE_SOCKET_BEGIN;
			break;

#ifdef RTRLIB_HAVE_LIBSSH
		case CLI_PARSE_STATE_SOCKET_SSH_OPTIONS:
			parse_socket_opts(argc, argv, current_config);

			/* Check again if enough arguments are available,
			 * accounting for options
			 */
			if ((argc - optind) < 4)
				print_error_exit(
					"Not enough arguments for ssh socket");

			state = CLI_PARSE_STATE_SOCKET_SSH_HOST;
			break;

		case CLI_PARSE_STATE_SOCKET_SSH_HOST:
			if (!is_resolveable_host(argv[optind]))
				print_error_exit(
					"cannot resolve \"%s\"\n",
					argv[optind]);

			current_config->host = argv[optind++];

			state = CLI_PARSE_STATE_SOCKET_SSH_PORT;
			break;

		case CLI_PARSE_STATE_SOCKET_SSH_PORT:
			if (!is_valid_port_number(argv[optind]))
				print_error_exit(
					"\"%s\" is not a valid port number\n",
					argv[optind]);

			current_config->port = argv[optind++];

			state = CLI_PARSE_STATE_SOCKET_SSH_USERNAME;
			break;

		case CLI_PARSE_STATE_SOCKET_SSH_USERNAME:
			current_config->ssh_username = argv[optind++];

			state = CLI_PARSE_STATE_SOCKET_SSH_PRIVATE_KEY;
			break;

		case CLI_PARSE_STATE_SOCKET_SSH_PRIVATE_KEY:
			if (!is_readable_file(argv[optind]))
				print_error_exit(
					"\"%s\" is not a readable file\n",
					argv[optind]);

			current_config->ssh_private_key = argv[optind++];

			state = CLI_PARSE_STATE_SOCKET_SSH_HOST_KEY;
			break;

		case CLI_PARSE_STATE_SOCKET_SSH_HOST_KEY:
			if (strncasecmp(argv[optind], "tcp",
					strlen(argv[optind])) == 0 ||
			    strncasecmp(argv[optind], "ssh",
					strlen(argv[optind])) == 0) {
				state = CLI_PARSE_STATE_SOCKET_BEGIN;
				break;
			}

			if (!is_readable_file(argv[optind]))
				print_error_exit(
					"\"%s\" is not a readable file\n",
					argv[optind]);

			current_config->ssh_host_key = argv[optind++];

			state = CLI_PARSE_STATE_SOCKET_BEGIN;
			break;
#endif

		case CLI_PARSE_STATE_END:
			return 0;
		}

		if (optind >= argc)
			state = CLI_PARSE_STATE_END;
	}
}

static void init_sockets(void)
{
	for (size_t i = 0; i < socket_count; ++i) {
		struct socket_config *config = socket_config[i];
		struct tr_tcp_config tcp_config;
#ifdef RTRLIB_HAVE_LIBSSH
		struct tr_ssh_config ssh_config;
#endif

		switch (config->type) {
		case SOCKET_TYPE_TCP:
			tcp_config.host = config->host;
			tcp_config.port = config->port;
			tcp_config.bindaddr = config->bindaddr;

			tr_tcp_init(&tcp_config, &config->tr_socket);
			config->socket.tr_socket = &config->tr_socket;
			break;

#ifdef RTRLIB_HAVE_LIBSSH
		case SOCKET_TYPE_SSH:
			ssh_config.host = config->host;
			ssh_config.port = atoi(config->port);
			ssh_config.bindaddr = config->bindaddr;
			ssh_config.username = config->ssh_username;
			ssh_config.client_privkey_path =
				config->ssh_private_key;
			ssh_config.server_hostkey_path = config->ssh_host_key;

			tr_ssh_init(&ssh_config, &config->tr_socket);
			config->socket.tr_socket = &config->tr_socket;
			break;
#endif
		}
	}
}

int main(int argc, char **argv)
{
	if (argc < 4) {
		print_usage(argv);
		return EXIT_FAILURE;
	}

	parse_cli(argc, argv);

	init_sockets();

	struct rtr_mgr_config *conf;
	struct rtr_mgr_group groups[1];

	groups[0].sockets_len = socket_count;
	groups[0].sockets = (struct rtr_socket **)socket_config;
	groups[0].preference = 1;

	spki_update_fp spki_update_fp =
		activate_spki_update_cb ? update_spki : NULL;
	pfx_update_fp pfx_update_fp = activate_pfx_update_cb ? update_cb : NULL;

	int ret = rtr_mgr_init(&conf, groups, 1, 30, 600, 600, pfx_update_fp,
			       spki_update_fp, status_fp, NULL);

	if (ret == RTR_ERROR)
		fprintf(stderr, "Error in rtr_mgr_init!\n");
	else if (ret == RTR_INVALID_PARAM)
		fprintf(stderr, "Invalid params passed to rtr_mgr_init\n");

	if (!conf)
		return EXIT_FAILURE;

	if (activate_pfx_update_cb && socket_count > 1)
		printf("%-40s %-40s   %3s   %3s   %3s\n",
		       "host", "Prefix", "Prefix Length", "", "ASN");
	else if (activate_pfx_update_cb)
		printf("%-40s   %3s   %3s   %3s\n",
		       "Prefix", "Prefix Length", "", "ASN");

	rtr_mgr_start(conf);

	pause();
	rtr_mgr_stop(conf);
	rtr_mgr_free(conf);
	free(groups[0].sockets);

	return EXIT_SUCCESS;
}
