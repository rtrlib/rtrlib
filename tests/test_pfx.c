/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include "rtrlib/lib/ip.h"
#include "rtrlib/lib/utils.h"

#ifdef RTRLIB_USE_LPFST
#include "rtrlib/pfx/lpfst/lpfst-pfx.h"
#else
#include "rtrlib/pfx/trie/trie-pfx.h"
#endif



#ifdef RTRLIB_USE_LPFST
/**
 * @brief remove_src_test_lpfst
 * This test verfies pfx_table_src_remove function. It first adds certain
 * records with different sockets into a pfx_table. Afterwards entries with
 * socket tr1 are removed, and the remaining records are verfified.
 */
static void remove_src_test_lpfst(void)
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
	struct lpfst_node **array = NULL;
	enum pfxv_state res;
	/* verify that table has 3 distinct prefix entries */
	assert(lpfst_get_children(pfxt.ipv4, &array, &len) != -1);
	free(array);
	array = NULL;
	assert((len + 1) == 3);

	/* remove entries with socket tr1, verify remaining 2 records */
	pfx_table_src_remove(&pfxt, &tr1);
	len = 0;
	assert(lpfst_get_children(pfxt.ipv4, &array, &len) != -1);
	free(array);
	assert((len + 1) == 2);

	/* verify validation of prefixes */
	assert(pfx_table_validate(&pfxt, 90,
				  &pfx.prefix, 8,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_NOT_FOUND);
	lrtr_ip_str_to_addr("10.11.10.0", &pfx.prefix);

	assert(pfx_table_validate(&pfxt, 90,
				  &pfx.prefix, 32,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_VALID);
	assert(pfx_table_validate(&pfxt, 80,
				  &pfx.prefix, 32,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_INVALID);

	/* cleanup: free table */
	pfx_table_free(&pfxt);
	printf("remove_src_test successful\n");
}
#else
/**
 * @brief remove_src_test_trie
 * This test verfies pfx_table_src_remove function. It first adds certain
 * records with different sockets into a pfx_table. Afterwards entries with
 * socket tr1 are removed, and the remaining records are verfified.
 */
static void remove_src_test_trie(void)
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
	enum pfxv_state res;
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
	assert(pfx_table_validate(&pfxt, 90,
				  &pfx.prefix, 8,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_NOT_FOUND);
	lrtr_ip_str_to_addr("10.11.10.0", &pfx.prefix);

	assert(pfx_table_validate(&pfxt, 90,
				  &pfx.prefix, 32,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_VALID);
	assert(pfx_table_validate(&pfxt, 80,
				  &pfx.prefix, 32,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_INVALID);

	/* cleanup: free table */
	pfx_table_free(&pfxt);
	printf("remove_src_test successful\n");
}
#endif

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
		assert(pfx_table_validate(&pfxt, i,
					  &pfx.prefix, pfx.min_len,
					  &res) == PFX_SUCCESS);
		assert(res == BGP_PFXV_STATE_VALID);
		assert(pfx_table_validate(&pfxt, i + 1,
					  &pfx.prefix, pfx.min_len,
					  &res) == PFX_SUCCESS);
		assert(res == BGP_PFXV_STATE_VALID);

		pfx.min_len = 128;
		pfx.max_len = 128;
		pfx.prefix.ver = LRTR_IPV6;
		((uint64_t *)pfx.prefix.u.addr6.addr)[1] = max_i;
		((uint64_t *)pfx.prefix.u.addr6.addr)[0] = min_i + i;

		assert(pfx_table_validate(&pfxt, i + 1,
					  &pfx.prefix, pfx.min_len,
					  &res) == PFX_SUCCESS);
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
	printf("mass_test successful\n");
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
	assert(pfx_table_validate(&pfxt, 123,
				  &pfx.prefix, 16,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_VALID);

	assert(pfx_table_validate(&pfxt, 124,
				  &pfx.prefix, 16,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_INVALID);

	assert(pfx_table_validate(&pfxt, 123,
				  &pfx.prefix, 24,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_VALID);

	lrtr_ip_str_to_addr("10.10.10.0", &pfx.prefix);
	assert(pfx_table_validate(&pfxt, 123,
				  &pfx.prefix, 20,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_VALID);

	assert(pfx_table_validate(&pfxt, 123,
				  &pfx.prefix, 25,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_INVALID);

	lrtr_ip_str_to_addr("10.11.10.0", &pfx.prefix);
	assert(pfx_table_validate(&pfxt, 123,
				  &pfx.prefix, 16,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_NOT_FOUND);

	lrtr_ip_str_to_addr("10.10.0.0", &pfx.prefix);
	pfx.asn = 122;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	assert(pfx_table_validate(&pfxt, 122,
				  &pfx.prefix, 18,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_VALID);

	lrtr_ip_str_to_addr("11.10.0.0", &pfx.prefix);
	pfx.asn = 22;
	pfx.min_len = 17;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	assert(pfx_table_validate(&pfxt, 22,
				  &pfx.prefix, 17,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_VALID);

	lrtr_ip_str_to_addr("2a01:4f8:131::", &pfx.prefix);
	pfx.prefix.ver = LRTR_IPV6;
	pfx.min_len = 48;
	pfx.max_len = 48;
	pfx.asn = 124;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	assert(pfx_table_validate(&pfxt, 124,
				  &pfx.prefix, 48,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_VALID);

	lrtr_ip_str_to_addr("2a01:4f8:131:15::", &pfx.prefix);
	assert(pfx_table_validate(&pfxt, 124,
				  &pfx.prefix, 56,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_INVALID);

	assert(lrtr_ip_str_to_addr("1.0.4.0", &pfx.prefix) == 0);
	pfx.min_len = 22;
	pfx.max_len = 22;
	pfx.asn = 56203;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	assert(pfx_table_validate(&pfxt, pfx.asn,
				  &pfx.prefix, pfx.min_len,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_VALID);

	assert(lrtr_ip_str_to_addr("1.8.1.0", &pfx.prefix) == 0);
	pfx.min_len = 24;
	pfx.max_len = 24;
	pfx.asn = 38345;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	assert(pfx_table_validate(&pfxt, pfx.asn,
				  &pfx.prefix, pfx.min_len,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_VALID);

	assert(lrtr_ip_str_to_addr("1.8.8.0", &pfx.prefix) == 0);
	pfx.min_len = 24;
	pfx.max_len = 24;
	pfx.asn = 38345;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	assert(pfx_table_validate(&pfxt, pfx.asn,
				  &pfx.prefix, pfx.min_len,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_VALID);
	pfx_table_free(&pfxt);

	assert(lrtr_ip_str_to_addr("1.0.65.0", &pfx.prefix) == 0);
	pfx.min_len = 18;
	pfx.max_len = 18;
	pfx.asn = 18144;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	assert(pfx_table_validate(&pfxt, pfx.asn,
				  &pfx.prefix, pfx.min_len,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_VALID);
	pfx_table_free(&pfxt);

	assert(lrtr_ip_str_to_addr("10.0.0.0", &pfx.prefix) == 0);
	pfx.min_len = 16;
	pfx.max_len = 16;
	pfx.asn = 123;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	assert(pfx_table_validate(&pfxt, pfx.asn,
				  &pfx.prefix, pfx.min_len,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_VALID);

	assert(lrtr_ip_str_to_addr("10.0.5.0", &pfx.prefix) == 0);
	pfx.min_len = 24;
	pfx.max_len = 24;
	pfx.asn = 124;
	assert(pfx_table_validate(&pfxt, pfx.asn,
				  &pfx.prefix, pfx.min_len,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_INVALID);

	pfx_table_free(&pfxt);
	assert(lrtr_ip_str_to_addr("109.105.96.0", &pfx.prefix) == 0);
	pfx.min_len = 19;
	pfx.max_len = 19;
	pfx.asn = 123;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	assert(lrtr_ip_str_to_addr("109.105.128.0", &pfx.prefix) == 0);
	assert(pfx_table_validate(&pfxt, 456,
				  &pfx.prefix, 20,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_NOT_FOUND);

	pfx_table_free(&pfxt);

	assert(lrtr_ip_str_to_addr("190.57.224.0", &pfx.prefix) == 0);
	pfx.min_len = 19;
	pfx.max_len = 24;
	pfx.asn = 123;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	assert(lrtr_ip_str_to_addr("190.57.72.0", &pfx.prefix) == 0);
	assert(pfx_table_validate(&pfxt, 123,
				  &pfx.prefix, 21,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_NOT_FOUND);

	pfx_table_free(&pfxt);

	assert(lrtr_ip_str_to_addr("80.253.128.0", &pfx.prefix) == 0);
	pfx.min_len = 19;
	pfx.max_len = 19;
	pfx.asn = 123;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	assert(lrtr_ip_str_to_addr("80.253.144.0", &pfx.prefix) == 0);
	assert(pfx_table_validate(&pfxt, 123,
				  &pfx.prefix, 20,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_INVALID);

	assert(lrtr_ip_str_to_addr("10.10.0.0", &pfx.prefix) == 0);
	pfx.min_len = 16;
	pfx.max_len = 16;
	pfx.asn = 0;
	assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
	assert(pfx_table_validate(&pfxt, 123,
				  &pfx.prefix, 16,
				  &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_INVALID);

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

	struct pfx_record *r = NULL;
	unsigned int r_len = 0;

	assert(lrtr_ip_str_to_addr("10.1.0.0", &pfx.prefix) == 0);
	assert(pfx_table_validate_r(&pfxt, &r, &r_len, 123,
				    &pfx.prefix, 16,
				    &res) == PFX_SUCCESS);
	assert(res == BGP_PFXV_STATE_INVALID);
	assert(r_len == 3);

	/* cleanup: free record and table */
	free(r);
	pfx_table_free(&pfxt);
	printf("pfx_table_test successful\n");
}

int main(void)
{
	pfx_table_test();
	#ifdef RTRLIB_USE_LPFST
	remove_src_test_lpfst();
	#else
	remove_src_test_trie();
	#endif
	mass_test();

	return EXIT_SUCCESS;
}
