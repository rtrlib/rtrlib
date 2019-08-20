/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "rtrlib/lib/ip.h"
#include "rtrlib/pfx/pfx.h"
#include "rtrlib/pfx/trie/trie-pfx.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

uint32_t min_i = 0xFF000000;
uint32_t max_i = 0xFFFFFFF0;

/**
 * @brief Add records to prefix table
 */
static void *rec_add(struct pfx_table *pfxt)
{
	const int tid = getpid();
	struct pfx_record rec;

	rec.min_len = 32;
	rec.max_len = 32;
	rec.prefix.ver = LRTR_IPV4;
	rec.prefix.u.addr4.addr = 0;

	printf("Inserting %u records\n", (max_i - min_i) * 3);
	for (uint32_t i = max_i; i >= min_i; i--) {
		rec.min_len = 32;
		rec.max_len = 32;
		rec.socket = NULL;
		rec.asn = tid % 2;
		rec.prefix.u.addr4.addr = htonl(i);
		rec.prefix.ver = LRTR_IPV4;
		pfx_table_add(pfxt, &rec);
		rec.asn = (tid % 2) + 1;
		pfx_table_add(pfxt, &rec);

		rec.min_len = 128;
		rec.max_len = 128;
		rec.prefix.ver = LRTR_IPV6;
		rec.prefix.u.addr6.addr[1] = min_i + 0xFFFFFFFF;
		rec.prefix.u.addr6.addr[0] = htonl(i) + 0xFFFFFFFF;
		pfx_table_add(pfxt, &rec);
		usleep(rand() / (RAND_MAX / 20));
	}

	return NULL;
}

/**
 * @brief Validate records in prefix table
 */
static void *rec_val(struct pfx_table *pfxt)
{
	const int tid = getpid();
	struct pfx_record rec;
	enum pfxv_state res;

	rec.min_len = 32;
	rec.max_len = 32;
	rec.prefix.ver = LRTR_IPV4;
	rec.prefix.u.addr4.addr = 0;
	printf("validating..\n");
	for (uint32_t i = max_i; i >= min_i; i--) {
		rec.min_len = 32;
		rec.max_len = 32;
		rec.prefix.ver = LRTR_IPV4;
		rec.prefix.u.addr4.addr = htonl(i);
		pfx_table_validate(pfxt, (tid % 2), &rec.prefix, rec.min_len, &res);
		pfx_table_validate(pfxt, (tid % 2) + 1, &rec.prefix, rec.min_len, &res);

		rec.min_len = 128;
		rec.max_len = 128;
		rec.prefix.ver = LRTR_IPV6;
		rec.prefix.u.addr6.addr[1] = min_i + 0xFFFFFFFF;
		rec.prefix.u.addr6.addr[0] = htonl(i) + 0xFFFFFFFF;

		pfx_table_validate(pfxt, (tid % 2) + 1, &rec.prefix, rec.min_len, &res);
		usleep(rand() / (RAND_MAX / 20));
	}

	return NULL;
}

/**
 * @brief Delete records from prefix table
 */
static void *rec_del(struct pfx_table *pfxt)
{
	const int tid = getpid();
	struct pfx_record rec;

	rec.min_len = 32;
	rec.max_len = 32;
	rec.prefix.ver = LRTR_IPV4;
	rec.prefix.u.addr4.addr = 0;
	printf("removing records\n");
	for (uint32_t i = max_i; i >= min_i; i--) {
		rec.socket = NULL;
		rec.min_len = 32;
		rec.max_len = 32;
		rec.asn = tid % 2;
		rec.prefix.ver = LRTR_IPV4;
		rec.prefix.u.addr4.addr = htonl(i);
		pfx_table_remove(pfxt, &rec);
		rec.asn = (tid % 2) + 1;
		pfx_table_remove(pfxt, &rec);

		rec.prefix.ver = LRTR_IPV6;
		rec.min_len = 128;
		rec.max_len = 128;
		rec.prefix.u.addr6.addr[1] = min_i + 0xFFFFFFFF;
		rec.prefix.u.addr6.addr[0] = htonl(i) + 0xFFFFFFFF;

		pfx_table_remove(pfxt, &rec);
		usleep(rand() / (RAND_MAX / 20));
	}
	printf("Done\n");

	return NULL;
}

/**
 * @brief Test concurrent operations on pfx_table
 * This test creates 15 (random) threads that add, delete, or validate huge
 * numbers of entries in/to/from a pfx_table.
 *
 * NOTE: this test takes a (very) long time to complete, use with care!
 */
int main(void)
{
	unsigned int max_threads = 15;
	struct pfx_table pfxt;
	pthread_t threads[max_threads];

	pfx_table_init(&pfxt, NULL);
	srand(time(NULL));

	for (unsigned int i = 0; i < max_threads; i++) {
		int r = rand() / (RAND_MAX / 3);

		if (r == 0)
			pthread_create(&threads[i], NULL, (void *(*)(void *))rec_add, &pfxt);
		else if (r == 1)
			pthread_create(&threads[i], NULL, (void *(*)(void *))rec_del, &pfxt);
		else if (r == 2)
			pthread_create(&threads[i], NULL, (void *(*)(void *))rec_val, &pfxt);
		printf("Started Thread %d\n", i);
		usleep(200);
	}
	for (unsigned int i = 0; i < max_threads; i++) {
		pthread_join(threads[i], NULL);
		printf("Thread %i returned\n", i);
	}

	return EXIT_SUCCESS;
}
