/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "templates.h"

#include "rtrlib/rtrlib.h"

#include "third-party/mustach/mustach.h"
#include "third-party/tommyds/tommyarray.h"
#include "third-party/tommyds/tommyhashlin.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
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

__attribute__((format(printf, 1, 2), noreturn)) static void print_error_exit(const char *fmt, ...);

static bool is_readable_file(const char *str);

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
#ifdef RTRLIB_HAVE_LIBSSH
	char *ssh_username;
	char *ssh_private_key;
	char *ssh_host_key;
#endif
};

/* activate update callback */
bool activate_pfx_update_cb = false;
bool activate_spki_update_cb = false;

/* print all updates regardless of socket configuration */
bool print_all_pfx_updates = false;
bool print_all_spki_updates = false;

bool print_status_updates = false;

bool export_pfx = false;
char *export_file_path = NULL;
const char *template_name = NULL;

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
	socket_config = checked_realloc(socket_config, sizeof(struct socket_config *) * (socket_count + 1));
	struct socket_config *config = socket_config[socket_count] = checked_malloc(sizeof(struct socket_config));

	memset(config, 0, sizeof(*config));
	++socket_count;

	return config;
}

static const char *get_template(const char *name)
{
	const struct pfx_output_template *current = templates;

	while (current->name) {
		if (strcmp(name, current->name) == 0)
			return current->template;

		++current;
	}

	if (is_readable_file(name)) {
		FILE *template_file = fopen(name, "r");

		if (fseek(template_file, 0, SEEK_END) == -1)
			goto read_error;

		long file_size = ftell(template_file);

		if (file_size == -1)
			goto read_error;

		rewind(template_file);

		char *template = checked_malloc(file_size);

		size_t bytes_read = fread(template, 1, file_size, template_file);

		if (bytes_read != (unsigned long)file_size)
			goto read_error;

		fclose(template_file);
		return template;

	read_error:
		fclose(template_file);
		print_error_exit("Could not read template");
	}

	print_error_exit("Template \"%s\" not found", name);
}

static void print_templates(void)
{
	const struct pfx_output_template *current = templates;

	while (current->name) {
		if (template_name && strcmp(current->name, template_name) == 0)
			printf("%s", current->template);
		else if (!template_name)
			printf("%s\n", current->name);

		++current;
	}
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

static bool is_file(const char *str)
{
	struct stat stat_buf;

	if (stat(str, &stat_buf) == -1)
		return false;

	return S_ISREG(stat_buf.st_mode);
}

static bool is_readable_file(const char *str)
{
	if (access(str, R_OK) != 0)
		return false;

	return is_file(str);
}

struct exporter_state {
	bool roa_section;
	tommy_count_t current_roa;
	tommy_array *roas;
};

struct pfx_export_cb_arg {
	tommy_array *array;
	tommy_hashlin *hashtable;
};

static void pfx_export_cb(const struct pfx_record *pfx_record, void *data);

static tommy_uint32_t hash_pfx_record(const struct pfx_record *record)
{
	struct pfx_record_packed {
		uint8_t ip[16];
		uint32_t asn;
		uint8_t min_len;
		uint8_t max_len;
	} __attribute__((packed));

	struct pfx_record_packed packed_record;

	memset(&packed_record, 0, sizeof(packed_record));

	if (record->prefix.ver == LRTR_IPV4)
		memcpy(&packed_record.ip, &record->prefix.u.addr4, sizeof(struct lrtr_ipv4_addr));
	else
		memcpy(&packed_record.ip, &record->prefix.u.addr6, sizeof(struct lrtr_ipv6_addr));

	packed_record.asn = record->asn;
	packed_record.min_len = record->min_len;
	packed_record.max_len = record->max_len;

	return tommy_hash_u32(0, &packed_record, sizeof(packed_record));
}

static int pfx_record_cmp(const void *data1, const void *data2)
{
	const struct pfx_record *arg1 = data1;
	const struct pfx_record *arg2 = data2;

	if ((arg1->asn == arg2->asn) && (arg1->max_len == arg2->max_len) && (arg1->min_len == arg2->min_len) &&
	    lrtr_ip_addr_equal(arg1->prefix, arg2->prefix))
		return 0;

	return 1;
}

static int template_enter(void *closure, const char *name)
{
	struct exporter_state *state = closure;

	if (strncmp("roas", name, strlen(name)) == 0) {
		state->roa_section = true;
		state->current_roa = 0;

		return 1;

	} else if (state->current_roa && strcmp("last", name) == 0 &&
		   state->current_roa == tommy_array_size(state->roas) - 1) {
		return 1;
	}

	return 0;
}

static int template_leave(void *closure)
{
	struct exporter_state *state = closure;

	state->roa_section = false;
	return 0;
}

static int template_next(void *closure)
{
	struct exporter_state *state = closure;

	if (++state->current_roa >= tommy_array_size(state->roas))
		return 0;

	return 1;
}

static int template_put(void *closure, const char *name, __attribute__((unused)) int escape, FILE *file)
{
	struct exporter_state *state = closure;

	size_t namelen = strlen(name);

	if (!state->roa_section || namelen == 0)
		return MUSTACH_ERROR_SYSTEM;

	struct pfx_record *pfx_record = tommy_array_get(state->roas, state->current_roa);

	if (strncmp("prefix", name, namelen) == 0) {
		char prefix_str[INET6_ADDRSTRLEN];

		lrtr_ip_addr_to_str(&pfx_record->prefix, prefix_str, sizeof(prefix_str));
		fputs(prefix_str, file);

	} else if (strncmp("length", name, namelen) == 0) {
		fprintf(file, "%d", pfx_record->min_len);

	} else if (strncmp("maxlen", name, namelen) == 0) {
		fprintf(file, "%d", pfx_record->max_len);

	} else if (strncmp("origin", name, namelen) == 0) {
		fprintf(file, "%d", pfx_record->asn);
	}

	return MUSTACH_OK;
}

struct mustach_itf template_itf = {.start = NULL,
				   .put = template_put,
				   .enter = template_enter,
				   .next = template_next,
				   .leave = template_leave};

static void print_usage(char **argv)
{
	printf("Usage:\n");
	printf(" %s [-hpkels] [-o file] [-t template] <socket>...\n", argv[0]);
	printf("\nSocket:\n");
	printf(" tcp [-hpkb bindaddr] <host> <port>\n");
#ifdef RTRLIB_HAVE_LIBSSH
	printf(" ssh [-hpkb bindaddr] <host> <port> <username> <private_key> [<host_key>]\n");
#endif
	printf("\nOptions:\n");
	printf("-b  bindaddr Hostnamne or IP address to connect from\n\n");

	printf("-k  Print information about SPKI updates.\n");
	printf("-p  Print information about PFX updates.\n");
	printf("-s  Print information about connection status updates.\n\n");

	printf("-e  export pfx table and exit\n");
	printf("-o  output file for export\n");
	printf("-t  template used for export\n");
	printf("-l  list available templates\n\n");

	printf("-h  Print this help message.\n");

	printf("\nExamples:\n");
	printf(" %s tcp rpki-validator.realmv6.org 8283\n", argv[0]);
	printf(" %s tcp -k -p rpki-validator.realmv6.org 8283\n", argv[0]);
	printf(" %s tcp -k rpki-validator.realmv6.org 8283 tcp -s example.com 323\n", argv[0]);
	printf(" %s -kp tcp rpki-validator.realmv6.org 8283 tcp example.com 323\n", argv[0]);
#ifdef RTRLIB_HAVE_LIBSSH
	printf(" %s ssh rpki-validator.realmv6.org 22 rtr-ssh", argv[0]);
	printf(" ~/.ssh/id_rsa ~/.ssh/known_hosts\n");
	printf(" %s ssh -k -p rpki-validator.realmv6.org 22 rtr-ssh ~/.ssh/id_rsa ~/.ssh/known_hosts\n", argv[0]);
	printf(" %s ssh -k -p rpki-validator.realmv6.org 22 rtr-ssh ~/.ssh/id_rsa ~/.ssh/known_hosts", argv[0]);
	printf(" ssh -k -p example.com 22 rtr-ssh ~/.ssh/id_rsa_example\n");
	printf(" %s ssh -k -p rpki-validator.realmv6.org 22 rtr-ssh ~/.ssh/id_rsa ~/.ssh/known_hosts", argv[0]);
	printf(" tcp -k -p example.com 323\n");
#endif

	printf(" %s -e tcp rpki-validator.realmv6.org 8283\n", argv[0]);
	printf(" %s -e -t csv -o roa.csv tcp rpki-validator.realmv6.org 8283\n", argv[0]);
}

static void status_fp(const struct rtr_mgr_group *group __attribute__((unused)), enum rtr_mgr_status mgr_status,
		      const struct rtr_socket *rtr_sock, void *data __attribute__((unused)))
{
	if (print_status_updates) {
		pthread_mutex_lock(&stdout_mutex);
		printf("RTR-Socket changed connection status to: %s, Mgr Status: %s\n",
		       rtr_state_to_str(rtr_sock->state), rtr_mgr_status_to_str(mgr_status));
		pthread_mutex_unlock(&stdout_mutex);
	}
}

static void update_cb(struct pfx_table *p __attribute__((unused)), const struct pfx_record rec, const bool added)
{
	char ip[INET6_ADDRSTRLEN];

	const struct socket_config *config = (const struct socket_config *)rec.socket;

	if (!print_all_pfx_updates && !config->print_pfx_updates)
		return;

	pthread_mutex_lock(&stdout_mutex);
	if (added)
		printf("+ ");
	else
		printf("- ");
	lrtr_ip_addr_to_str(&rec.prefix, ip, sizeof(ip));
	if (socket_count > 1)
		printf("%s:%s %-40s   %3u - %3u   %10u\n", config->host, config->port, ip, rec.min_len, rec.max_len,
		       rec.asn);
	else
		printf("%-40s   %3u - %3u   %10u\n", ip, rec.min_len, rec.max_len, rec.asn);

	pthread_mutex_unlock(&stdout_mutex);
}

static void update_spki(struct spki_table *s __attribute__((unused)), const struct spki_record record, const bool added)
{
	const struct socket_config *config = (const struct socket_config *)record.socket;

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

	bool print_template = false;

	while ((opt = getopt(argc, argv, "+kphelo:t:s")) != -1) {
		switch (opt) {
		case 'k':
			activate_spki_update_cb = true;
			print_all_spki_updates = true;
			break;

		case 'p':
			activate_pfx_update_cb = true;
			print_all_pfx_updates = true;
			break;

		case 'e':
			export_pfx = true;
			break;

		case 'o':
			if (export_file_path)
				print_error_exit("output file can not be specified more than once");

			export_file_path = optarg;
			break;

		case 't':
			template_name = optarg;
			break;

		case 'l':
			print_template = true;
			break;

		case 's':
			print_status_updates = true;
			break;

		default:
			print_usage(argv);
			exit(EXIT_FAILURE);
		}
	}

	if (print_template) {
		print_templates();
		exit(EXIT_SUCCESS);
	}

	if ((export_file_path || template_name) && !export_pfx)
		print_error_exit("Specifying -o or -t without -e does not make sense");
}

static void parse_socket_opts(int argc, char **argv, struct socket_config *config)
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

static void print_error_exit(const char *fmt, ...)
{
	va_list argptr;

	va_start(argptr, fmt);

	fprintf(stderr, "error: ");

	vfprintf(stderr, fmt, argptr);

	fprintf(stderr, "\n");

	va_end(argptr);

	exit(EXIT_FAILURE);
}

static int parse_cli(int argc, char **argv)
{
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

			if (strncasecmp(argv[optind], "tcp", strlen(argv[optind])) == 0) {
				state = CLI_PARSE_STATE_SOCKET_TCP_OPTIONS;
				current_config->type = SOCKET_TYPE_TCP;

				/* Check if enough arguments are available
				 * to configure this socket.
				 * This does not account for options.
				 */
				if ((argc - optind) < 2)
					print_error_exit("Not enough arguments for tcp socket");

			} else if (strncasecmp(argv[optind], "ssh", strlen(argv[optind])) == 0) {
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
				print_error_exit("\"%s\" is not a valid socket type\n", argv[optind]);
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
				print_error_exit("Not enough arguments for tcp socket");

			state = CLI_PARSE_STATE_SOCKET_TCP_HOST;
			break;

		case CLI_PARSE_STATE_SOCKET_TCP_HOST:
			if (!is_resolveable_host(argv[optind]))
				print_error_exit("cannot resolve \"%s\"\n", argv[optind]);

			current_config->host = argv[optind++];

			state = CLI_PARSE_STATE_SOCKET_TCP_PORT;
			break;

		case CLI_PARSE_STATE_SOCKET_TCP_PORT:
			if (!is_valid_port_number(argv[optind]))
				print_error_exit("\"%s\" is not a valid port number\n", argv[optind]);

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
				print_error_exit("Not enough arguments for ssh socket");

			state = CLI_PARSE_STATE_SOCKET_SSH_HOST;
			break;

		case CLI_PARSE_STATE_SOCKET_SSH_HOST:
			if (!is_resolveable_host(argv[optind]))
				print_error_exit("cannot resolve \"%s\"\n", argv[optind]);

			current_config->host = argv[optind++];

			state = CLI_PARSE_STATE_SOCKET_SSH_PORT;
			break;

		case CLI_PARSE_STATE_SOCKET_SSH_PORT:
			if (!is_valid_port_number(argv[optind]))
				print_error_exit("\"%s\" is not a valid port number\n", argv[optind]);

			current_config->port = argv[optind++];

			state = CLI_PARSE_STATE_SOCKET_SSH_USERNAME;
			break;

		case CLI_PARSE_STATE_SOCKET_SSH_USERNAME:
			current_config->ssh_username = argv[optind++];

			state = CLI_PARSE_STATE_SOCKET_SSH_PRIVATE_KEY;
			break;

		case CLI_PARSE_STATE_SOCKET_SSH_PRIVATE_KEY:
			if (!is_readable_file(argv[optind]))
				print_error_exit("\"%s\" is not a readable file\n", argv[optind]);

			current_config->ssh_private_key = argv[optind++];

			state = CLI_PARSE_STATE_SOCKET_SSH_HOST_KEY;
			break;

		case CLI_PARSE_STATE_SOCKET_SSH_HOST_KEY:
			if (strncasecmp(argv[optind], "tcp", strlen(argv[optind])) == 0 ||
			    strncasecmp(argv[optind], "ssh", strlen(argv[optind])) == 0) {
				state = CLI_PARSE_STATE_SOCKET_BEGIN;
				break;
			}

			if (!is_readable_file(argv[optind]))
				print_error_exit("\"%s\" is not a readable file\n", argv[optind]);

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
			ssh_config.client_privkey_path = config->ssh_private_key;
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
	parse_cli(argc, argv);

	if (socket_count == 0) {
		print_usage(argv);
		exit(EXIT_FAILURE);
	}

	init_sockets();

	struct rtr_mgr_config *conf;
	struct rtr_mgr_group groups[1];

	groups[0].sockets_len = socket_count;
	groups[0].sockets = (struct rtr_socket **)socket_config;
	groups[0].preference = 1;

	spki_update_fp spki_update_fp = activate_spki_update_cb ? update_spki : NULL;
	pfx_update_fp pfx_update_fp = activate_pfx_update_cb ? update_cb : NULL;

	int ret = rtr_mgr_init(&conf, groups, 1, 30, 600, 600, pfx_update_fp, spki_update_fp, status_fp, NULL);

	if (ret == RTR_ERROR)
		fprintf(stderr, "Error in rtr_mgr_init!\n");
	else if (ret == RTR_INVALID_PARAM)
		fprintf(stderr, "Invalid params passed to rtr_mgr_init\n");

	if (!conf)
		return EXIT_FAILURE;

	if (!export_pfx && activate_pfx_update_cb && socket_count > 1)
		printf("%-40s %-40s   %3s   %3s   %3s\n", "host", "Prefix", "Prefix Length", "", "ASN");
	else if (!export_pfx && activate_pfx_update_cb)
		printf("%-40s   %3s   %3s   %3s\n", "Prefix", "Prefix Length", "", "ASN");

	if (export_pfx) {
		const char *template;

		if (template_name)
			template = get_template(template_name);
		else
			template = templates->template;
		FILE *export_file = stdout;

		if (export_file_path) {
			export_file = fopen(export_file_path, "w");
			if (!export_file) {
				char *errormsg = strerror(errno);

				print_error_exit("\"%s\" could not be opened for writing: %s", optarg, errormsg);
			}
		}

		rtr_mgr_start(conf);

		while (!rtr_mgr_conf_in_sync(conf))
			sleep(1);

		printf("Sync done\n");
		tommy_array prefixes;
		tommy_hashlin prefix_hash;
		struct pfx_export_cb_arg arg = {
			.array = &prefixes,
			.hashtable = &prefix_hash,
		};

		tommy_array_init(&prefixes);
		tommy_hashlin_init(&prefix_hash);

		pfx_table_for_each_ipv4_record(conf->pfx_table, pfx_export_cb, &arg);
		pfx_table_for_each_ipv6_record(conf->pfx_table, pfx_export_cb, &arg);

		struct exporter_state state = {
			.roa_section = false,
			.current_roa = 0,
			.roas = &prefixes,
		};

		fmustach(template, &template_itf, &state, export_file);

		tommy_hashlin_foreach(&prefix_hash, free);
		tommy_hashlin_done(&prefix_hash);
		tommy_array_done(&prefixes);

		if (export_file != stdout && fclose(export_file) == EOF) {
			char *errormsg = strerror(errno);

			print_error_exit("\"Error during write into output file: %s\"", errormsg);
		}

	} else {
		rtr_mgr_start(conf);
		pause();
	}

	rtr_mgr_stop(conf);
	rtr_mgr_free(conf);
	free(groups[0].sockets);

	return EXIT_SUCCESS;
}

struct pfx_record_entry {
	struct pfx_record record;
	tommy_node hash_node;
};

static void pfx_export_cb(const struct pfx_record *pfx_record, void *data)
{
	struct pfx_export_cb_arg *arg = data;
	tommy_array *roa_array = arg->array;
	tommy_hashlin *roa_hashtable = arg->hashtable;

	tommy_hash_t record_hash = hash_pfx_record(pfx_record);

	if (tommy_hashlin_search(roa_hashtable, &pfx_record_cmp, pfx_record, record_hash) != 0)
		return;

	struct pfx_record_entry *pfx_record_entry = malloc(sizeof(struct pfx_record_entry));

	memcpy(&pfx_record_entry->record, pfx_record, sizeof(struct pfx_record));

	tommy_hashlin_insert(roa_hashtable, &pfx_record_entry->hash_node, pfx_record_entry, record_hash);
	tommy_array_insert(roa_array, pfx_record_entry);
}
