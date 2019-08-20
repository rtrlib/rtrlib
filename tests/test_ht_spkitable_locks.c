/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "rtrlib/spki/hashtable/ht-spkitable_private.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

struct add_records_args {
	struct spki_table *table;
	int start_asn;
	int count;
};

struct remove_records_args {
	struct spki_table *table;
	int start_asn;
	int count;
};

static struct spki_record *create_record(int ASN, int ski_offset, int spki_offset, struct rtr_socket *socket);

/**
 * @brief Compare SPKI records for equality
 *
 * @return true if r1 == r2, false otherwise
 */
static bool compare_spki_records(struct spki_record *r1, struct spki_record *r2)
{
	if (r1->asn != r2->asn)
		return false;
	if (r1->socket != r2->socket)
		return false;
	if (memcmp(r1->ski, r2->ski, SKI_SIZE) != 0)
		return false;
	if (memcmp(r1->spki, r2->spki, SPKI_SIZE) != 0)
		return false;

	return true;
}

/**
 * @brief Create a SPKI record
 *
 * @return new SPKI record
 */
static struct spki_record *create_record(int ASN, int ski_offset, int spki_offset, struct rtr_socket *socket)
{
	struct spki_record *record = malloc(sizeof(struct spki_record));
	uint32_t i;

	record->asn = ASN;

	for (i = 0; i < sizeof(record->ski); i++)
		record->ski[i] = i + ski_offset;

	for (i = 0; i < sizeof(record->spki); i++)
		record->spki[i] = i + spki_offset;

	record->socket = socket;
	return record;
}

/**
 * @brief Add records to spki table
 * Add 'args->count' records to the spki table 'args->table', start with
 * ASN 'args->start_asn'.
 */
static void *add_records(struct add_records_args *args)
{
	printf("Add %i records: ASN [%i..%i]\n", args->count, args->start_asn, args->count + args->start_asn - 1);
	for (int i = args->start_asn; i < args->count + args->start_asn; i++) {
		struct spki_record *record = create_record(i, i, i, NULL);
		int ret = spki_table_add_entry(args->table, record);

		assert(ret == SPKI_SUCCESS);
		free(record);
	}

	return NULL;
}

/**
 * @brief remove records from spki table
 * Remove 'args->count' records from the spki table 'args->table'.
 */
static void *remove_records(struct remove_records_args *args)
{
	printf("Remove %i records: ASN [%i..%i]\n", args->count, args->start_asn, args->count + args->start_asn - 1);
	for (int i = args->start_asn; i < args->count + args->start_asn; i++) {
		struct spki_record *record = create_record(i, i, i, NULL);
		int ret = spki_table_remove_entry(args->table, record);

		assert(ret == SPKI_SUCCESS);
		free(record);
	}

	return NULL;
}

/**
 * @brief Test concurrent add and delete operations on spki table
 */
static void lock_test1(void)
{
	unsigned int max_threads = 20;
	unsigned int records_per_thread = 10000;
	struct spki_table spkit;
	struct add_records_args args[max_threads];

	spki_table_init(&spkit, NULL);
	pthread_t threads[max_threads];

	/* Concurrently add SPKI records to the table */
	for (unsigned int i = 0; i < max_threads; i++) {
		args[i].table = &spkit;
		args[i].start_asn = i * records_per_thread;
		args[i].count = records_per_thread;
		pthread_create(&threads[i], NULL, (void *(*)(void *))add_records, &args[i]);
	}

	/* Wait for parallel add operations to finish */
	for (unsigned int i = 0; i < max_threads; i++) {
		pthread_join(threads[i], NULL);
		printf("Thread %i returned\n", i);
	}

	/* Check all records for successful add */
	struct spki_record *result;
	unsigned int result_size = 0;

	for (unsigned int i = 0; i < records_per_thread * max_threads; i++) {
		struct spki_record *record = create_record(i, i, i, NULL);

		spki_table_get_all(&spkit, record->asn, record->ski, &result, &result_size);
		assert(result_size == 1);
		assert(compare_spki_records(record, &result[0]));
		free(result);
		free(record);
	}

	struct remove_records_args remove_args[max_threads];
	/* Concurrently delete SPKI records */
	for (unsigned int i = 0; i < max_threads; i++) {
		remove_args[i].table = &spkit;
		remove_args[i].start_asn = i * records_per_thread;
		remove_args[i].count = records_per_thread;
		pthread_create(&threads[i], NULL, (void *(*)(void *))remove_records, &remove_args[i]);
	}

	/* Wait for parallel delete operation to finish */
	for (unsigned int i = 0; i < max_threads; i++) {
		pthread_join(threads[i], NULL);
		printf("Thread %i returned\n", i);
	}

	/* cleanup: free spki_table */
	spki_table_free(&spkit);
	printf("%s() complete\n", __func__);
}

int main(void)
{
	lock_test1();
	return EXIT_SUCCESS;
}
