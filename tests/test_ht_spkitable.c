/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "rtrlib/rtr/rtr_private.h"
#include "rtrlib/spki/hashtable/ht-spkitable_private.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* test_ht_4 */
#define NUM_TABLE_X (50)
#define NUM_TABLE_Y (50)
/* test_ht_6 */
#define NUM_SKIS (10)
#define NUM_SKIS_RECORDS (10)
/* test_ht_7 */
#define NUM_OF_SKI (200)
#define NUM_OF_RECORDS (2000)

/**
 * @brief Compare SPKI records for equality
 *
 * @return true if r1 == r2, false otherwise
 */
static bool spki_records_are_equal(struct spki_record *r1, struct spki_record *r2)
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

	memset(record, 0, sizeof(*record));
	record->asn = ASN;

	for (i = 0; i < sizeof(record->ski) / sizeof(uint32_t); i++)
		((uint32_t *)record->ski)[i] = i + ski_offset;

	for (i = 0; i < sizeof(record->spki) / sizeof(uint32_t); i++)
		((uint32_t *)record->spki)[i] = i + spki_offset;

	record->socket = socket;
	return record;
}

/**
 * @brief Helper shortcut, to assert insert of table entry
 * Assert fails if memory allocation for new entry fails OR
 * if the entry already exists.
 */
static void _spki_table_add_assert(struct spki_table *table, struct spki_record *record)
{
	assert(spki_table_add_entry(table, record) == SPKI_SUCCESS);
}

/**
 * @brief Helper shortcut, to assert remove of table entry
 * Assert fails if entry does not exist in table, or any search error occurs.
 */
static void _spki_table_remove_assert(struct spki_table *table, struct spki_record *record)
{
	assert(spki_table_remove_entry(table, record) == SPKI_SUCCESS);
}

/**
 * @brief Helper shortcut, to assert search of table entry
 * Assert fails if a matching enrty is found, but memory allocation fails OR
 * if number of results does not match reference value: NUM_SKIS_RECORDS.
 */
static void _spki_table_search_assert(struct spki_table *table, uint8_t *ski)
{
	struct spki_record *result;
	unsigned int result_len;

	assert(spki_table_search_by_ski(table, ski, &result, &result_len) == SPKI_SUCCESS);
	assert(result_len == NUM_SKIS_RECORDS);
	free(result);
}

/* ----- TESTS FUNCTIONS ----- */

/**
 * @brief Test of spki_table_src_remove function
 * Test if the spki_table_src_remove function is working correctly by adding
 * spki_record associated with different rtr_socket.
 * Then call spki_table_src_remove with one of the sockets as argument and
 * validate that all records associated with different sockets are still there.
 */
static void test_ht_1(void)
{
	struct spki_table table;
	struct rtr_socket *socket_one = malloc(sizeof(struct rtr_socket));
	struct rtr_socket *socket_two = malloc(sizeof(struct rtr_socket));
	uint8_t ski[SKI_SIZE];
	uint32_t asn = 1;

	/* test create record */
	struct spki_record *record = create_record(1, 0, 0, NULL);

	spki_table_init(&table, NULL);
	memcpy(ski, record->ski, 20);
	free(record);

	/* create and add records with either socket_one or socket_two */
	for (int i = 0; i < 255; i++) {
		if (i % 2)
			record = create_record(1, 0, i, socket_one);
		else
			record = create_record(1, 0, i, socket_two);

		_spki_table_add_assert(&table, record);
		free(record);
	}

	struct spki_record *result;
	unsigned int result_len;
	int count = 0;
	/* verify all (255) records for asn and ski size */
	spki_table_get_all(&table, asn, ski, &result, &result_len);
	for (unsigned int i = 0; i < result_len; i++) {
		assert(result->asn == asn);
		assert(memcmp(&result[i].ski, ski, SKI_SIZE) == 0);
		count++;
	}
	assert(count == 255);
	free(result);

	/* remove all records with socket_one */
	spki_table_src_remove(&table, socket_one);
	/* verify remaining records have socket_two */
	spki_table_get_all(&table, asn, ski, &result, &result_len);
	for (unsigned int i = 0; i < result_len; i++) {
		assert(result[i].asn == asn);
		assert(memcmp(&result[i].ski, ski, SKI_SIZE) == 0);
		assert(result[i].socket == socket_two);
	}

	/* cleanup: free memory */
	spki_table_free(&table);
	free(result);
	free(socket_one);
	free(socket_two);
	printf("%s() complete\n", __func__);
}

/**
 * @brief Test of spki_table_get_all with equal records
 * Check the behaviour of spki_table_get_all if we add spki_record with
 * different SPKI values but same SKI values (Hash collision).
 */
static void test_ht_2(void)
{
	struct spki_table table;
	struct spki_record *result;
	unsigned int result_len;
	/* create 2 distinct records */
	struct spki_record *record1 = create_record(10, 20, 30, NULL);
	struct spki_record *record2 = create_record(10, 20, 40, NULL);
	/* TEST1: verify 2 diff SPKIs hash to the same SKI */
	spki_table_init(&table, NULL);
	_spki_table_add_assert(&table, record1);
	_spki_table_add_assert(&table, record2);
	spki_table_get_all(&table, 10, record1->ski, &result, &result_len);
	assert(result_len == 2);

	/* TEST2: check that returned records are the same we added. */
	assert(spki_records_are_equal(&result[0], record1) != spki_records_are_equal(&result[0], record2));
	assert(spki_records_are_equal(&result[1], record1) != spki_records_are_equal(&result[1], record2));
	free(result);

	/* TEST3: remove record1 and verify result is record2 */
	_spki_table_remove_assert(&table, record1);
	spki_table_get_all(&table, 10, record1->ski, &result, &result_len);
	assert(result_len == 1);
	assert(spki_records_are_equal(&result[0], record2));
	free(result);

	/*TEST4: remove record2 and verify search result is empty */
	_spki_table_remove_assert(&table, record2);
	spki_table_get_all(&table, 10, record1->ski, &result, &result_len);
	assert(result_len == 0);
	assert(!result);
	spki_table_get_all(&table, 10, record2->ski, &result, &result_len);
	assert(result_len == 0);
	assert(!result);

	/* cleanup: free memory */
	free(record1);
	free(record2);
	spki_table_free(&table);
	printf("%s() complete\n", __func__);
}

/**
 * @brief Test spki_table_get_all with records, differing in one attribute
 * Test if the compare function for spki_record work correctly:
 * Add spki_records which only differ in one attribute and delete one
 * of the records, then check if the other records are still there.
 */
static void test_ht_3(void)
{
	struct spki_table table;
	struct spki_record *result;
	unsigned int result_len;

	/* create 4 distinct SPKI records, (at least) one parameter different */
	struct spki_record *record1 = create_record(10, 10, 10, NULL);
	struct spki_record *record2 = create_record(10, 10, 11, NULL);
	struct spki_record *record3 = create_record(10, 11, 10, NULL);
	struct spki_record *record4 = create_record(11, 10, 10, NULL);

	/* TEST0:init table and add records -> checks compare function */
	spki_table_init(&table, NULL);
	_spki_table_add_assert(&table, record1);
	_spki_table_add_assert(&table, record2);
	_spki_table_add_assert(&table, record3);
	_spki_table_add_assert(&table, record4);

	/* TEST1: remove record1, check others -------------------------------*/
	_spki_table_remove_assert(&table, record1);
	/* Check if other records are still there */
	assert(spki_table_get_all(&table, record2->asn, record2->ski, &result, &result_len) == SPKI_SUCCESS);
	assert(result_len == 1);
	free(result);

	assert(spki_table_get_all(&table, record3->asn, record3->ski, &result, &result_len) == SPKI_SUCCESS);
	assert(result_len == 1);
	free(result);

	assert(spki_table_get_all(&table, record4->asn, record4->ski, &result, &result_len) == SPKI_SUCCESS);
	assert(result_len == 1);
	free(result);
	/* (re)add record1 */
	_spki_table_add_assert(&table, record1);

	/* TEST2: remove record2, check others -------------------------------*/
	_spki_table_remove_assert(&table, record2);
	/* Check if other records are still there */
	assert(spki_table_get_all(&table, record1->asn, record1->ski, &result, &result_len) == SPKI_SUCCESS);
	assert(result_len == 1);
	free(result);

	assert(spki_table_get_all(&table, record3->asn, record3->ski, &result, &result_len) == SPKI_SUCCESS);
	assert(result_len == 1);
	free(result);

	assert(spki_table_get_all(&table, record4->asn, record4->ski, &result, &result_len) == SPKI_SUCCESS);
	assert(result_len == 1);
	free(result);
	/* (re)add record2 */
	_spki_table_add_assert(&table, record2);

	/* TEST3: remove record3, check others -------------------------------*/
	_spki_table_remove_assert(&table, record3);
	/* Check if other records are still there */
	assert(spki_table_get_all(&table, record1->asn, record1->ski, &result, &result_len) == SPKI_SUCCESS);
	assert(result_len == 2);
	free(result);

	assert(spki_table_get_all(&table, record2->asn, record2->ski, &result, &result_len) == SPKI_SUCCESS);
	assert(result_len == 2);
	free(result);

	assert(spki_table_get_all(&table, record4->asn, record4->ski, &result, &result_len) == SPKI_SUCCESS);
	assert(result_len == 1);
	free(result);
	/* (re)add record3 */
	_spki_table_add_assert(&table, record3);

	/* TEST4: remove record4, check others -------------------------------*/
	_spki_table_remove_assert(&table, record4);
	/* Check if other records are still there */
	assert(spki_table_get_all(&table, record1->asn, record1->ski, &result, &result_len) == SPKI_SUCCESS);
	assert(result_len == 2);
	free(result);

	assert(spki_table_get_all(&table, record2->asn, record2->ski, &result, &result_len) == SPKI_SUCCESS);
	assert(result_len == 2);
	free(result);

	assert(spki_table_get_all(&table, record3->asn, record3->ski, &result, &result_len) == SPKI_SUCCESS);
	assert(result_len == 1);
	free(result);

	/* cleanup: free memory */
	spki_table_free(&table);
	free(record1);
	free(record2);
	free(record3);
	free(record4);
	printf("%s complete\n", __func__);
}

/**
 * @brief Test remove records one by one until spki table is empty
 * Test if all added records can be deleted and test if any of the added
 * records retain in the table although they got deleted.
 */
static void test_ht_4(void)
{
	struct spki_table table;
	struct spki_record *result;
	unsigned int result_len;
	struct spki_record *records[NUM_TABLE_X][NUM_TABLE_Y];

	spki_table_init(&table, NULL);
	/* Add 50 * 50 entries */
	for (int i = 0; i < NUM_TABLE_X; i++) {
		for (int j = 0; j < NUM_TABLE_Y; j++) {
			records[i][j] = create_record(i, j, j, NULL);
			_spki_table_add_assert(&table, records[i][j]);
		}
	}

	/* Check if every record is there and then delete it */
	for (int i = 0; i < NUM_TABLE_X; i++) {
		for (int j = 0; j < NUM_TABLE_Y; j++) {
			assert(spki_table_get_all(&table, records[i][j]->asn, records[i][j]->ski, &result,
						  &result_len) == SPKI_SUCCESS);
			assert(result_len == 1);
			_spki_table_remove_assert(&table, records[i][j]);
			free(result);
			free(records[i][j]);
		}
	}

	/* Add all record again and look for SPKI_DUPLICATE_RECORD */
	for (int i = 0; i < NUM_TABLE_X; i++) {
		for (int j = 0; j < NUM_TABLE_Y; j++) {
			records[i][j] = create_record(i, j, j, NULL);
			_spki_table_add_assert(&table, records[i][j]);
			free(records[i][j]);
		}
	}

	/* cleanup: free memory */
	spki_table_free(&table);
	printf("%s() complete\n", __func__);
}

/**
 * @brief Test for SPKI_DUPLICATE_RECORD
 * Test the behavior if equal spki_records get added to the table.
 */
static void test_ht_5(void)
{
	struct spki_table table;
	/* create 2 equal records */
	struct spki_record *record1 = create_record(10, 10, 10, NULL);
	struct spki_record *record2 = create_record(10, 10, 10, NULL);

	/* create table and (try) add records -> check for duplicates */
	spki_table_init(&table, NULL);
	assert(spki_table_add_entry(&table, record1) == SPKI_SUCCESS);
	assert(spki_table_add_entry(&table, record2) == SPKI_DUPLICATE_RECORD);
	assert(spki_table_add_entry(&table, record1) == SPKI_DUPLICATE_RECORD);

	struct spki_record *result;
	unsigned int result_len;

	/* check that only record1 is in table and matches query */
	spki_table_get_all(&table, 10, record1->ski, &result, &result_len);
	assert(result_len == 1);
	free(result);

	/* cleanup: free memory */
	spki_table_free(&table);
	free(record1);
	free(record2);
	printf("%s() complete\n", __func__);
}

/**
 * @brief Test spki_table_search_by_ski
 * Test if all spki_records with the same SKI get returned.
 */
static void test_ht_6(void)
{
	struct spki_table table;
	struct spki_record *records[NUM_SKIS][NUM_SKIS_RECORDS];
	/* Add the records to the table */
	spki_table_init(&table, NULL);
	for (unsigned int i = 0; i < NUM_SKIS; i++) {
		for (unsigned int j = 0; j < NUM_SKIS_RECORDS; j++) {
			records[i][j] = create_record(j, i, j, NULL);
			_spki_table_add_assert(&table, records[i][j]);
		}
	}

	/* Search for records by SKI and check result */
	for (unsigned int i = 0; i < NUM_SKIS; i++) {
		for (unsigned int j = 0; j < NUM_SKIS_RECORDS; j++) {
			_spki_table_search_assert(&table, records[i][j]->ski);
			free(records[i][j]);
		}
	}

	/* cleanup: free memory */
	spki_table_free(&table);
	printf("%s() complete\n", __func__);
}

/**
 * @brief Test releation between ASN and SKI
 * This tests has 3 parts, in each part spki records are created and validated:
 *      a) spki records with same ASN but different SKI: ASN(1) <---> SKI(n)
 *      b) spki records with different ASN but same SKI: ASN(n) <---> SKI(1)
 *      c) spki records with different ASN and diff SKI: ASN(n) <---> SKI(n)
 */
static void test_ht_7(void)
{
	struct spki_table table;
	struct spki_record *records[NUM_OF_RECORDS];
	/* ASN(1) <---> SKI(n) ---------------------------------------------- */
	spki_table_init(&table, NULL);
	/* Create NUM_OF_RECORDS spki_records with same ASN but different SKI */
	for (unsigned int i = 0; i < NUM_OF_RECORDS; i++) {
		records[i] = create_record(2555, i, i, NULL);
		_spki_table_add_assert(&table, records[i]);
	}

	/* Validate */
	for (unsigned int i = 0; i < NUM_OF_RECORDS; i++) {
		struct spki_record *result;
		unsigned int size = 0;

		spki_table_get_all(&table, 2555, records[i]->ski, &result, &size);

		assert(size == 1);
		assert(spki_records_are_equal(records[i], &result[0]));
		free(result);
		free(records[i]);
	}
	spki_table_free(&table);

	/* ASN(n) <---> SKI(1) ---------------------------------------------- */
	spki_table_init(&table, NULL);
	/* Create NUM_OF_RECORDS spki_records with same SKI but different ASN */
	for (unsigned int i = 0; i < NUM_OF_RECORDS; i++) {
		records[i] = create_record(i, 100, 100, NULL);
		_spki_table_add_assert(&table, records[i]);
	}

	/* Validate */
	for (unsigned int i = 0; i < NUM_OF_RECORDS; i++) {
		struct spki_record *result;
		unsigned int size = 0;

		spki_table_get_all(&table, i, records[NUM_OF_RECORDS - 1]->ski, &result, &size);

		assert(size == 1);
		assert(spki_records_are_equal(records[i], &result[0]));
		free(result);
		free(records[i]);
	}
	spki_table_free(&table);

	struct spki_record *records_n_n[NUM_OF_RECORDS][NUM_OF_SKI];
	/* ASN(n) <---> SKI(n) ---------------------------------------------- */
	spki_table_init(&table, NULL);
	/* Create: {NUM_OF_RECORDS} x {NUM_OF_SKI} spki_records */
	/* {ASN_0,ASN_1...} x {SKI_0, SKI_1...} */
	for (unsigned int i = 0; i < NUM_OF_RECORDS; i++) {
		for (unsigned int j = 0; j < NUM_OF_SKI; j++) {
			records_n_n[i][j] = create_record(i, j, j, NULL);
			_spki_table_add_assert(&table, records_n_n[i][j]);
		}
	}

	/* Validate */
	for (unsigned int i = 0; i < NUM_OF_RECORDS; i++) {
		for (unsigned int j = 0; j < NUM_OF_SKI; j++) {
			struct spki_record *result;
			unsigned int size = 0;

			spki_table_get_all(&table, i, records_n_n[i][j]->ski, &result, &size);

			assert(size == 1);
			assert(spki_records_are_equal(records_n_n[i][j], &result[0]));
			free(result);
			free(records_n_n[i][j]);
		}
	}

	/* cleanup: free memory */
	spki_table_free(&table);
	printf("%s() complete\n", __func__);
}

static void test_table_swap(void)
{
	struct spki_table table1;
	struct spki_table table2;

	struct spki_record *test_record1 = create_record(1, 10, 100, NULL);
	struct spki_record *test_record2 = create_record(2, 20, 200, NULL);

	spki_table_init(&table1, NULL);
	spki_table_init(&table2, NULL);

	_spki_table_add_assert(&table1, test_record1);
	_spki_table_add_assert(&table2, test_record2);

	struct spki_record *result;
	unsigned int result_len;

	assert(spki_table_search_by_ski(&table1, test_record1->ski, &result, &result_len) == SPKI_SUCCESS);
	assert(result_len == 1);
	free(result);
	result = NULL;

	assert(spki_table_search_by_ski(&table2, test_record2->ski, &result, &result_len) == SPKI_SUCCESS);
	assert(result_len == 1);
	free(result);
	result = NULL;

	spki_table_swap(&table1, &table2);

	assert(spki_table_search_by_ski(&table1, test_record2->ski, &result, &result_len) == SPKI_SUCCESS);
	assert(result_len == 1);
	free(result);
	result = NULL;

	assert(spki_table_search_by_ski(&table2, test_record1->ski, &result, &result_len) == SPKI_SUCCESS);
	assert(result_len == 1);
	free(result);
	result = NULL;

	spki_table_free(&table1);
	spki_table_free(&table2);
	free(test_record1);
	free(test_record2);

	printf("%s() complete\n", __func__);
}

static void update_spki(struct spki_table *s __attribute__((unused)), const struct spki_record record, const bool added)
{
	printf("%c ASN: %u\n", (added ? '+' : '-'), record.asn);

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

	if (record.asn == 1)
		assert(!added);
	if (record.asn == 2)
		assert(false);
	if (record.asn == 3)
		assert(added);
}

static void test_table_diff(void)
{
	struct spki_table table1;
	struct spki_table table2;
	struct rtr_socket *socket = (struct rtr_socket *)1;

	struct spki_record *test_record1 = create_record(1, 10, 100, socket);
	struct spki_record *test_record2 = create_record(2, 20, 200, socket);
	struct spki_record *test_record3 = create_record(3, 30, 300, socket);

	spki_table_init(&table1, NULL);
	spki_table_init(&table2, NULL);

	printf("Adding to table 1\n");
	_spki_table_add_assert(&table1, test_record1);
	_spki_table_add_assert(&table1, test_record2);

	printf("Adding to table 2\n");
	_spki_table_add_assert(&table2, test_record2);
	_spki_table_add_assert(&table2, test_record3);

	printf("Calculating diff\n");
	table1.update_fp = update_spki;
	table2.update_fp = update_spki;
	spki_table_notify_diff(&table2, &table1, socket);
	table1.update_fp = NULL;
	table2.update_fp = NULL;

	printf("Freeing tables\n");
	spki_table_free(&table1);
	spki_table_free(&table2);
	free(test_record1);
	free(test_record2);
	free(test_record3);

	printf("%s() complete\n", __func__);
}

int main(void)
{
	test_ht_1();
	test_ht_2();
	test_ht_3();
	test_ht_4();
	test_ht_5();
	test_ht_6();
	test_ht_7();
	test_table_swap();
	test_table_diff();
	return EXIT_SUCCESS;
}
