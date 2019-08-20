/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "rtrlib/lib/log_private.h"
#include "rtrlib/lib/utils_private.h"
#include "rtrlib/pfx/trie/trie.c"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

/*
 * @brief Test trie core operations such as add and remove
 * This test validates core operations of the trie,
 * namely insert, remove, lookup, and lookup_exact.
 */
static void trie_test(void)
{
	struct lrtr_ip_addr addr;
	struct trie_node *result;
	struct trie_node n1, n2, n3, n4;
	unsigned int lvl = 0;
	bool found;

	addr.ver = LRTR_IPV4;

	/* node 1
	 * Tree after insert should be:
	 * 100.200.0.0/16
	 */

	n1.len = 16;
	n1.lchild = NULL;
	n1.rchild = NULL;
	n1.parent = NULL;
	n1.data = NULL;
	lrtr_ip_str_to_addr("100.200.0.0", &n1.prefix);
	addr = n1.prefix;
	result = trie_lookup(&n1, &addr, 16, &lvl);
	assert(result);
	assert(lrtr_ip_str_cmp(&result->prefix, "100.200.0.0"));

	lrtr_ip_str_to_addr("100.200.30.0", &addr);
	result = trie_lookup(&n1, &addr, 16, &lvl);
	assert(result);
	assert(lrtr_ip_str_cmp(&result->prefix, "100.200.0.0"));

	/* node 2
	 * Tree after insert should be:
	 * 100.200.0.0/16
	 *               \
	 *                132.200.0.0/16
	 */
	n2.len = 16;
	n2.lchild = NULL;
	n2.rchild = NULL;
	n2.parent = NULL;
	n2.data = NULL;
	lrtr_ip_str_to_addr("132.200.0.0", &n2.prefix);
	trie_insert(&n1, &n2, 0);
	lrtr_ip_str_to_addr("132.200.0.0", &addr);
	lvl = 0;
	result = trie_lookup(&n1, &addr, 16, &lvl);
	assert(result);
	assert(lrtr_ip_str_cmp(&result->prefix, "132.200.0.0"));
	assert(n1.rchild == &n2);

	/* node 3
	 * Tree after insert should be:
	 *		 100.200.0.0/16
	 *               /             \
	 * 101.200.0.0/16               132.200.0.0/16
	 */
	n3.len = 16;
	n3.lchild = NULL;
	n3.rchild = NULL;
	n3.parent = NULL;
	n3.data = NULL;

	lrtr_ip_str_to_addr("101.200.0.0", &n3.prefix);
	trie_insert(&n1, &n3, 0);
	lrtr_ip_str_to_addr("101.200.0.0", &addr);
	lvl = 0;
	result = trie_lookup(&n1, &addr, 16, &lvl);
	assert(result);
	assert(lrtr_ip_str_cmp(&result->prefix, "101.200.0.0"));
	assert(n1.lchild == &n3);

	/* node 4
	 * Tree after insert should be:
	 *		 100.200.0.0/16
	 *               /             \
	 * 101.200.0.0/16               132.200.0.0/16
	 *                              /
	 *                      132.201.3.0/24
	 */
	n4.len = 24;
	n4.lchild = NULL;
	n4.rchild = NULL;
	n4.parent = NULL;
	n4.data = NULL;

	lrtr_ip_str_to_addr("132.201.3.0", &n4.prefix);
	trie_insert(&n1, &n4, 0);
	lrtr_ip_str_to_addr("132.201.3.0", &addr);
	lvl = 0;
	result = trie_lookup(&n1, &addr, 24, &lvl);
	assert(result);
	assert(lrtr_ip_str_cmp(&result->prefix, "132.201.3.0"));

	assert(lrtr_ip_str_cmp(&n1.prefix, "100.200.0.0"));
	assert(n1.len == 16);

	/* verify tree structure */
	assert(lrtr_ip_str_cmp(&n1.lchild->prefix, "101.200.0.0"));
	assert(n1.lchild->len == 16);

	assert(lrtr_ip_str_cmp(&n1.rchild->prefix, "132.200.0.0"));
	assert(n1.rchild->len == 16);

	assert(lrtr_ip_str_cmp(&n1.rchild->lchild->prefix, "132.201.3.0"));
	assert(n1.rchild->lchild->len == 24);

	lrtr_ip_str_to_addr("132.200.0.0", &addr);
	lvl = 0;
	result = trie_lookup(&n1, &addr, 16, &lvl);
	assert(result);
	assert(lrtr_ip_str_cmp(&result->prefix, "132.200.0.0"));

	/* verify that a search for 132.200.3.0 returns 132.200/16 */
	lrtr_ip_str_to_addr("132.200.3.0", &addr);
	lvl = 0;
	result = trie_lookup(&n1, &addr, 16, &lvl);
	assert(result);
	assert(lrtr_ip_str_cmp(&result->prefix, "132.200.0.0"));

	/* verify no result for prefix 132.0.0.0/16 is found */
	lvl = 0;
	lrtr_ip_str_to_addr("132.0.0.0", &addr);
	result = trie_lookup_exact(&n1, &addr, 16, &lvl, &found);
	assert(!found);

	/* verify trie_lookup_exact for prefix 132.201.3.0/24 is found */
	lvl = 0;
	lrtr_ip_str_to_addr("132.201.3.0", &addr);
	result = trie_lookup_exact(&n1, &addr, 24, &lvl, &found);
	assert(found);
	assert(lrtr_ip_str_cmp(&result->prefix, "132.201.3.0"));

	/* remove root->rchild
	 * Tree after remove should be:
	 *		 100.200.0.0/16
	 *               /             \
	 * 101.200.0.0/16               132.201.3.0/24
	 */
	lrtr_ip_str_to_addr("132.200.0.0", &addr);
	result = trie_remove(&n1, &addr, 16, 0);
	assert(result);
	assert(lrtr_ip_str_cmp(&n1.prefix, "100.200.0.0"));
	assert(lrtr_ip_str_cmp(&n1.lchild->prefix, "101.200.0.0"));
	assert(lrtr_ip_str_cmp(&n1.rchild->prefix, "132.201.3.0"));
	assert(!n1.rchild->lchild);

	/* remove root->lchild
	 * Tree after remove should be:
	 *		 100.200.0.0/16
	 *                             \
	 *                        132.201.3.0/24
	 */
	lrtr_ip_str_to_addr("101.200.0.0", &addr);
	result = trie_remove(&n1, &addr, 16, 0);
	assert(result);
	assert(lrtr_ip_str_cmp(&n1.rchild->prefix, "132.201.3.0"));
	assert(!n1.lchild);

	/* remove root node
	 * Tree after remove should be:
	 *		 132.201.3.0/24
	 */
	lrtr_ip_str_to_addr("100.200.0.0", &addr);
	result = trie_remove(&n1, &addr, 16, 0);
	assert(lrtr_ip_str_cmp(&n1.prefix, "132.201.3.0"));
	assert(result);
	assert(!n1.lchild);
	assert(!n1.rchild);
}

int main(void)
{
	trie_test();
	printf("Test successful\n");
	return EXIT_SUCCESS;
}
