/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "rtrlib/lib/ip_private.h"
#include "rtrlib/lib/utils_private.h"
#include "rtrlib/pfx/pfx.h"
#include "rtrlib/pfx/pfx_private.h"
#include "rtrlib/pfx/trie/trie_private.h"
#include "rtrlib/rtr/rtr.h"

#include <arpa/inet.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

static void validate(struct pfx_table *pfxt, uint32_t asn, const char *prefix, uint8_t prefix_len,
		     enum pfxv_state expected_result)
{
	struct lrtr_ip_addr ip;
	enum pfxv_state val_res;

	assert(!lrtr_ip_str_to_addr(prefix, &ip));

	assert(pfx_table_validate(pfxt, asn, &ip, prefix_len, &val_res) == PFX_SUCCESS);

	assert(val_res == expected_result);
}

/**
 * @brief remove_src_test
 * This test verfies pfx_table_src_remove function. It first adds certain
 * records with different sockets into a pfx_table. Afterwards entries with
 * socket tr1 are removed, and the remaining records are verfified.
 */
static void remove_src_test(void)
{
	struct pfx_table pfxt;
	struct rtr_socket tr1;
	struct pfx_record pfx;
	/* TEST1: init prefix table ----------------------------------------- */
	pfx_table_init(&pfxt, NULL);
	pfx.min_len = 32;
	pfx.max_len = 32;
	/* TEST2: add and verify differnt prefixes -------------------------- */
	pfx.asn = 80;
	pfx.socket = &tr1;
	lrtr_ip_str_to_addr("10.11.10.0", &pfx.prefix);
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

	pfx.asn = 90;
	pfx.socket = NULL;
	lrtr_ip_str_to_addr("10.11.10.0", &pfx.prefix);
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

	pfx.socket = NULL;
	pfx.min_len = 24;
	lrtr_ip_str_to_addr("192.168.0.0", &pfx.prefix);
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

	pfx.socket = &tr1;
	pfx.min_len = 8;
	lrtr_ip_str_to_addr("10.0.0.0", &pfx.prefix);
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

	unsigned int len = 0;
	struct trie_node **array = NULL;
	/* verify that table has 3 distinct prefix entries */
	assert(trie_get_children(pfxt.ipv4, &array, &len) != -1);
	free(array);
	array = NULL;
	assert((len + 1) == 3);

	/* remove entries with socket tr1, verify remaining 2 records */
	pfx_table_src_remove(&pfxt, &tr1);
	len = 0;
	assert(trie_get_children(pfxt.ipv4, &array, &len) != -1);
	free(array);
	assert((len + 1) == 2);

	/* verify validation of prefixes */
	validate(&pfxt, 90, "10.0.0.0", 8, BGP_PFXV_STATE_NOT_FOUND);

	validate(&pfxt, 90, "10.11.10.0", 32, BGP_PFXV_STATE_VALID);

	validate(&pfxt, 80, "10.11.10.0", 32, BGP_PFXV_STATE_INVALID);

	/* cleanup: free table */
	pfx_table_free(&pfxt);
	printf("%s() successful\n", __func__);
}

/**
 * @brief Test pfx_table operations with many records
 * This test adds, validates, and removes a huge number of records in/to/from
 * a pfx_table.
 */
static void mass_test(void)
{
	struct pfx_table pfxt;
	struct pfx_record pfx;
	enum pfxv_state res;
	const uint32_t min_i = 0xFFFF0000;
	const uint32_t max_i = 0xFFFFFFF0;
	/* init table with huge number of records */
	pfx_table_init(&pfxt, NULL);
	printf("Inserting %u records\n", (max_i - min_i) * 3);
	for (uint32_t i = max_i; i >= min_i; i--) {
		pfx.min_len = 32;
		pfx.max_len = 32;
		pfx.socket = NULL;
		pfx.asn = i;
		pfx.prefix.u.addr4.addr = htonl(i);
		pfx.prefix.ver = LRTR_IPV4;
		assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
		/* add same prefix, with diff asn */
		pfx.asn = i + 1;
		assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
		/* add same prefix, with diff len */
		pfx.min_len = 128;
		pfx.max_len = 128;
		pfx.prefix.ver = LRTR_IPV6;
		((uint64_t *)pfx.prefix.u.addr6.addr)[1] = max_i;
		((uint64_t *)pfx.prefix.u.addr6.addr)[0] = min_i + i;
		assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	}

	/* verify validation of huge number of records */
	printf("validating..\n");
	for (uint32_t i = max_i; i >= min_i; i--) {
		pfx.min_len = 32;
		pfx.max_len = 32;
		pfx.prefix.ver = LRTR_IPV4;
		pfx.prefix.u.addr4.addr = htonl(i);
		assert(pfx_table_validate(&pfxt, i, &pfx.prefix, pfx.min_len, &res) == PFX_SUCCESS);
		assert(res == BGP_PFXV_STATE_VALID);
		assert(pfx_table_validate(&pfxt, i + 1, &pfx.prefix, pfx.min_len, &res) == PFX_SUCCESS);
		assert(res == BGP_PFXV_STATE_VALID);

		pfx.min_len = 128;
		pfx.max_len = 128;
		pfx.prefix.ver = LRTR_IPV6;
		((uint64_t *)pfx.prefix.u.addr6.addr)[1] = max_i;
		((uint64_t *)pfx.prefix.u.addr6.addr)[0] = min_i + i;

		assert(pfx_table_validate(&pfxt, i + 1, &pfx.prefix, pfx.min_len, &res) == PFX_SUCCESS);
		assert(res == BGP_PFXV_STATE_VALID);
	}

	/* verify removal of huge number of records */
	printf("removing records\n");
	for (uint32_t i = max_i; i >= min_i; i--) {
		pfx.socket = NULL;
		pfx.min_len = 32;
		pfx.max_len = 32;
		pfx.asn = i;
		pfx.prefix.ver = LRTR_IPV4;
		pfx.prefix.u.addr4.addr = htonl(i);
		assert(pfx_table_remove(&pfxt, &pfx) == PFX_SUCCESS);

		pfx.asn = i + 1;
		assert(pfx_table_remove(&pfxt, &pfx) == PFX_SUCCESS);

		pfx.prefix.ver = LRTR_IPV6;
		pfx.min_len = 128;
		pfx.max_len = 128;
		((uint64_t *)pfx.prefix.u.addr6.addr)[1] = max_i;
		((uint64_t *)pfx.prefix.u.addr6.addr)[0] = min_i + i;
		assert(pfx_table_remove(&pfxt, &pfx) == PFX_SUCCESS);
	}

	/* cleanup: free table */
	pfx_table_free(&pfxt);
	printf("%s() successful\n", __func__);
}

/**
 * @brief Test of pfx_table functions
 * This test verifies pfx_table and its core functions, namely
 * pfx_table_add and pfx_table_validate.
 */
static void pfx_table_test(void)
{
	struct pfx_table pfxt;
	struct pfx_record pfx;
	enum pfxv_state res;

	pfx_table_init(&pfxt, NULL);

	pfx.asn = 123;
	pfx.prefix.ver = LRTR_IPV4;
	lrtr_ip_str_to_addr("10.10.0.0", &pfx.prefix);
	pfx.min_len = 16;
	pfx.max_len = 24;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

	validate(&pfxt, 123, "10.10.0.0", 16, BGP_PFXV_STATE_VALID);
	validate(&pfxt, 124, "10.10.0.0", 16, BGP_PFXV_STATE_INVALID);
	validate(&pfxt, 123, "10.10.0.0", 24, BGP_PFXV_STATE_VALID);
	validate(&pfxt, 123, "10.10.10.0", 20, BGP_PFXV_STATE_VALID);
	validate(&pfxt, 123, "10.10.10.0", 25, BGP_PFXV_STATE_INVALID);
	validate(&pfxt, 123, "10.11.10.0", 16, BGP_PFXV_STATE_NOT_FOUND);

	lrtr_ip_str_to_addr("10.10.0.0", &pfx.prefix);
	pfx.asn = 122;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	validate(&pfxt, 122, "10.10.0.0", 18, BGP_PFXV_STATE_VALID);

	lrtr_ip_str_to_addr("11.10.0.0", &pfx.prefix);
	pfx.asn = 22;
	pfx.min_len = 17;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	validate(&pfxt, 22, "11.10.0.0", 17, BGP_PFXV_STATE_VALID);

	lrtr_ip_str_to_addr("2a01:4f8:131::", &pfx.prefix);
	pfx.prefix.ver = LRTR_IPV6;
	pfx.min_len = 48;
	pfx.max_len = 48;
	pfx.asn = 124;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	assert(pfx_table_validate(&pfxt, 124, &pfx.prefix, 48, &res) == PFX_SUCCESS);
	validate(&pfxt, 124, "2a01:4f8:131::", 48, BGP_PFXV_STATE_VALID);

	validate(&pfxt, 124, "2a01:4f8:131:15::", 56, BGP_PFXV_STATE_INVALID);

	assert(lrtr_ip_str_to_addr("1.0.4.0", &pfx.prefix) == 0);
	pfx.min_len = 22;
	pfx.max_len = 22;
	pfx.asn = 56203;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	validate(&pfxt, pfx.asn, "1.0.4.0", pfx.min_len, BGP_PFXV_STATE_VALID);

	assert(lrtr_ip_str_to_addr("1.8.1.0", &pfx.prefix) == 0);
	pfx.min_len = 24;
	pfx.max_len = 24;
	pfx.asn = 38345;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	validate(&pfxt, pfx.asn, "1.8.1.0", pfx.min_len, BGP_PFXV_STATE_VALID);

	assert(lrtr_ip_str_to_addr("1.8.8.0", &pfx.prefix) == 0);
	pfx.min_len = 24;
	pfx.max_len = 24;
	pfx.asn = 38345;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	validate(&pfxt, pfx.asn, "1.8.8.0", pfx.min_len, BGP_PFXV_STATE_VALID);
	pfx_table_free(&pfxt);

	assert(lrtr_ip_str_to_addr("1.0.65.0", &pfx.prefix) == 0);
	pfx.min_len = 18;
	pfx.max_len = 18;
	pfx.asn = 18144;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	validate(&pfxt, pfx.asn, "1.0.65.0", pfx.min_len, BGP_PFXV_STATE_VALID);
	pfx_table_free(&pfxt);

	assert(lrtr_ip_str_to_addr("10.0.0.0", &pfx.prefix) == 0);
	pfx.min_len = 16;
	pfx.max_len = 16;
	pfx.asn = 123;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	validate(&pfxt, pfx.asn, "10.0.0.0", pfx.min_len, BGP_PFXV_STATE_VALID);
	validate(&pfxt, 124, "10.0.5.0", 24, BGP_PFXV_STATE_INVALID);
	pfx_table_free(&pfxt);

	assert(lrtr_ip_str_to_addr("109.105.96.0", &pfx.prefix) == 0);
	pfx.min_len = 19;
	pfx.max_len = 19;
	pfx.asn = 123;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	validate(&pfxt, 456, "109.105.128.0", 20, BGP_PFXV_STATE_NOT_FOUND);
	pfx_table_free(&pfxt);

	assert(lrtr_ip_str_to_addr("190.57.224.0", &pfx.prefix) == 0);
	pfx.min_len = 19;
	pfx.max_len = 24;
	pfx.asn = 123;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	validate(&pfxt, 123, "190.57.72.0", 21, BGP_PFXV_STATE_NOT_FOUND);
	pfx_table_free(&pfxt);

	assert(lrtr_ip_str_to_addr("80.253.128.0", &pfx.prefix) == 0);
	pfx.min_len = 19;
	pfx.max_len = 19;
	pfx.asn = 123;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	validate(&pfxt, 123, "80.253.144.0", 20, BGP_PFXV_STATE_INVALID);

	assert(lrtr_ip_str_to_addr("10.10.0.0", &pfx.prefix) == 0);
	pfx.min_len = 16;
	pfx.max_len = 16;
	pfx.asn = 0;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	validate(&pfxt, 123, "10.10.0.0", 16, BGP_PFXV_STATE_INVALID);

	assert(lrtr_ip_str_to_addr("10.0.0.0", &pfx.prefix) == 0);
	pfx.min_len = 8;
	pfx.max_len = 15;
	pfx.asn = 6;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

	assert(lrtr_ip_str_to_addr("10.0.0.0", &pfx.prefix) == 0);
	pfx.min_len = 8;
	pfx.max_len = 15;
	pfx.asn = 5;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

	assert(lrtr_ip_str_to_addr("10.1.0.0", &pfx.prefix) == 0);
	pfx.min_len = 16;
	pfx.max_len = 16;
	pfx.asn = 5;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

	validate(&pfxt, 123, "10.1.0.0", 16, BGP_PFXV_STATE_INVALID);

	/* cleanup: free record and table */
	pfx_table_free(&pfxt);
	printf("%s() successful\n", __func__);
}

static void create_ip4_pfx_record(struct pfx_record *pfx, uint32_t asn, const char *ip, uint8_t min_mask_len,
				  uint8_t max_mask_len)
{
	pfx->asn = asn;
	pfx->min_len = min_mask_len;
	pfx->max_len = max_mask_len;
	pfx->socket = (struct rtr_socket *)1;
	assert(!lrtr_ip_str_to_addr(ip, &pfx->prefix));
}

static void add_ip4_pfx_record(struct pfx_table *pfxt, uint32_t asn, const char *ip, uint8_t min_mask_len,
			       uint8_t max_mask_len)
{
	struct pfx_record pfx;
	enum pfxv_state val_res;

	create_ip4_pfx_record(&pfx, asn, ip, min_mask_len, max_mask_len);

	assert(pfx_table_add(pfxt, &pfx) == PFX_SUCCESS);

	assert(pfx_table_validate(pfxt, pfx.asn, &pfx.prefix, pfx.min_len, &val_res) == PFX_SUCCESS);
	assert(val_res == BGP_PFXV_STATE_VALID);
}

static void test_issue99(void)
{
	struct pfx_table pfxt;

	pfx_table_init(&pfxt, NULL);

	add_ip4_pfx_record(&pfxt, 200, "10.100.255.0", 24, 24);
	add_ip4_pfx_record(&pfxt, 300, "255.0.0.0", 24, 24);
	add_ip4_pfx_record(&pfxt, 400, "128.0.0.0", 1, 24);

	validate(&pfxt, 400, "255.0.0.0", 24, BGP_PFXV_STATE_VALID);
	pfx_table_free(&pfxt);
}

static void test_issue152(void)
{
	struct pfx_table pfxt;
	struct pfx_record *records = calloc(6, sizeof(struct pfx_record));

	pfx_table_init(&pfxt, NULL);
	create_ip4_pfx_record(&records[0], 1, "89.18.183.0", 24, 24);
	create_ip4_pfx_record(&records[1], 2, "109.164.0.0", 17, 25);
	create_ip4_pfx_record(&records[2], 3, "185.131.60.0", 22, 24);
	create_ip4_pfx_record(&records[3], 4, "185.146.28.0", 22, 22);
	create_ip4_pfx_record(&records[4], 5, "212.5.51.0", 24, 24);
	create_ip4_pfx_record(&records[5], 6, "213.175.86.0", 24, 24);

	for (size_t i = 0; i < 6; i++)
		assert(pfx_table_add(&pfxt, &records[i]) == PFX_SUCCESS);

	for (size_t i = 0; i < 6; i++)
		assert(pfx_table_remove(&pfxt, &records[i]) == PFX_SUCCESS);

	free(records);
}

static void update_cb1(struct pfx_table *p __attribute__((unused)), const struct pfx_record rec, const bool added)
{
	char ip[INET6_ADDRSTRLEN];

	if (added)
		printf("+ ");
	else
		printf("- ");
	lrtr_ip_addr_to_str(&rec.prefix, ip, sizeof(ip));
	printf("%-40s   %3u - %3u   %10u\n", ip, rec.min_len, rec.max_len, rec.asn);

	if (rec.asn == 1)
		assert(!added);
	if (rec.asn == 2)
		assert(false);
	if (rec.asn == 3)
		assert(added);
}

static void test_pfx_merge(void)
{
	struct pfx_table pfxt1;
	struct pfx_table pfxt2;
	struct pfx_record records[3];

	pfx_table_init(&pfxt1, NULL);
	pfx_table_init(&pfxt2, NULL);

	create_ip4_pfx_record(&records[0], 1, "1.1.1.1", 24, 24);
	create_ip4_pfx_record(&records[1], 2, "2.2.2.2", 24, 24);
	create_ip4_pfx_record(&records[2], 3, "3.3.3.3", 24, 24);

	printf("Adding to table one\n");
	assert(pfx_table_add(&pfxt1, &records[0]) == PFX_SUCCESS);
	assert(pfx_table_add(&pfxt1, &records[1]) == PFX_SUCCESS);
	printf("Adding to table two\n");
	assert(pfx_table_add(&pfxt2, &records[1]) == PFX_SUCCESS);
	assert(pfx_table_add(&pfxt2, &records[2]) == PFX_SUCCESS);

	printf("Computing diff\n");
	pfxt1.update_fp = update_cb1;
	pfxt2.update_fp = update_cb1;
	pfx_table_notify_diff(&pfxt2, &pfxt1, (struct rtr_socket *)1);
	pfxt1.update_fp = NULL;
	pfxt2.update_fp = NULL;

	printf("Freeing table one\n");
	pfx_table_free(&pfxt1);
	printf("Freeing table two\n");
	pfx_table_free(&pfxt2);
}

int main(void)
{
	pfx_table_test();
	remove_src_test();
	mass_test();
	test_issue99();
	test_issue152();
	test_pfx_merge();

	return EXIT_SUCCESS;
}
