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
#include "rtrlib/lib/log.h"
#include "rtrlib/lib/utils.h"
#include "rtrlib/pfx/lpfst/lpfst.c"

/*
 * @brief Test lpfst core operations such as add and remove
 * This test validates core operations of the longest prefix first search tree
 * (lpfst), namely insert, remove, lookup, and lookup_exact.
 */
static void lpfst_test(void)
{
	struct lrtr_ip_addr addr;
	struct lpfst_node *result;
	unsigned int lvl = 0;
	/* set IP to version 4 */
	addr.ver = LRTR_IPV4;

	struct lpfst_node n1;
	/* init  node n1 */
	n1.len = 16;
	n1.lchild = NULL;
	n1.rchild = NULL;
	n1.parent = NULL;
	n1.data = NULL;

	lrtr_ip_str_to_addr("100.200.0.0", &n1.prefix);
	lrtr_ip_str_to_addr("100.200.0.0", &addr);
	lvl = 0;
	result = lpfst_lookup(&n1, &addr, 16, &lvl);
	assert(result);
	assert(lrtr_ip_str_cmp(&result->prefix, "100.200.0.0"));

	lrtr_ip_str_to_addr("100.200.30.0", &addr);
	lvl = 0;
	result = lpfst_lookup(&n1, &addr, 16, &lvl);
	assert(result);
	assert(lrtr_ip_str_cmp(&result->prefix, "100.200.0.0"));

	struct lpfst_node n2;
	/* init node n2 */
	n2.len = 16;
	n2.lchild = NULL;
	n2.rchild = NULL;
	n2.parent = NULL;
	n2.data = NULL;

	lrtr_ip_str_to_addr("132.200.0.0", &n2.prefix);
	lpfst_insert(&n1, &n2, 0);
	lrtr_ip_str_to_addr("132.200.0.0", &addr);
	lvl = 0;
	result = lpfst_lookup(&n1, &addr, 16, &lvl);
	assert(result);
	assert(lrtr_ip_str_cmp(&result->prefix, "132.200.0.0"));
	assert(n1.rchild == &n2);

	struct lpfst_node n3;
	/* init node n3 */
	n3.len = 16;
	n3.lchild = NULL;
	n3.rchild = NULL;
	n3.parent = NULL;
	n3.data = NULL;

	lrtr_ip_str_to_addr("101.200.0.0", &n3.prefix);
	lpfst_insert(&n1, &n3, 0);
	lrtr_ip_str_to_addr("101.200.0.0", &addr);
	lvl = 0;
	result = lpfst_lookup(&n1, &addr, 16, &lvl);
	assert(result);
	assert(lrtr_ip_str_cmp(&result->prefix, "101.200.0.0"));
	assert(n1.lchild == &n3);

	struct lpfst_node n4;
	/* init node n4 */
	n4.len = 24;
	n4.lchild = NULL;
	n4.rchild = NULL;
	n4.parent = NULL;
	n4.data = NULL;

	lrtr_ip_str_to_addr("132.200.3.0", &n4.prefix);
	lpfst_insert(&n1, &n4, 0);
	lrtr_ip_str_to_addr("132.200.3.0", &addr);
	lvl = 0;
	result = lpfst_lookup(&n1, &addr, 24, &lvl);
	assert(result);
	assert(lrtr_ip_str_cmp(&result->prefix, "132.200.3.0"));
	assert(lrtr_ip_str_cmp(&n1.prefix, "132.200.3.0"));
	assert(n1.len == 24);
	assert(lrtr_ip_str_cmp(&n1.lchild->prefix, "101.200.0.0"));
	assert(n1.lchild->len == 16);
	assert(lrtr_ip_str_cmp(&n1.rchild->prefix, "132.200.0.0"));
	assert(n1.rchild->len == 16);
	assert(lrtr_ip_str_cmp(&n1.lchild->rchild->prefix, "100.200.0.0"));
	assert(n1.lchild->rchild->len == 16);

	lrtr_ip_str_to_addr("132.200.0.0", &addr);
	lvl = 0;
	result = lpfst_lookup(&n1, &addr, 16, &lvl);
	assert(result);
	assert(lrtr_ip_str_cmp(&result->prefix, "132.200.0.0"));

	lrtr_ip_str_to_addr("132.200.3.0", &addr);
	lvl = 0;
	result = lpfst_lookup(&n1, &addr, 16, &lvl);
	assert(result);
	assert(lrtr_ip_str_cmp(&result->prefix, "132.200.0.0"));

	lvl = 0;
	lrtr_ip_str_to_addr("132.0.0.0", &addr);

	bool found;
	/* verify prefix 132.0.0.0/16 is not found */
	result = lpfst_lookup_exact(&n1, &addr, 16, &lvl, &found);
	assert(!found);

	lvl = 0;
	lrtr_ip_str_to_addr("132.200.3.0", &addr);
	/* verify prefix 132.200.3.0/24 is found */
	result = lpfst_lookup_exact(&n1, &addr, 24, &lvl, &found);
	assert(found);
	assert(lrtr_ip_str_cmp(&result->prefix, "132.200.3.0"));

	result = lpfst_remove(&n1, &addr, 24, 0);
	assert(result);
	assert(lrtr_ip_str_cmp(&n1.prefix, "132.200.0.0"));
	assert(lrtr_ip_str_cmp(&n1.lchild->prefix, "101.200.0.0"));
	assert(lrtr_ip_str_cmp(&n1.lchild->rchild->prefix, "100.200.0.0"));
	assert(!n1.rchild);

	lrtr_ip_str_to_addr("101.200.0.0", &addr);
	result = lpfst_remove(&n1, &addr, 16, 0);
	assert(result);
	assert(lrtr_ip_str_cmp(&n1.lchild->prefix, "100.200.0.0"));
	assert(!n1.rchild);

	lrtr_ip_str_to_addr("100.200.0.0", &addr);
	result = lpfst_remove(&n1, &addr, 16, 0);
	assert(result);
	assert(!n1.lchild);
	assert(!n1.rchild);
}

int main(void)
{
	lpfst_test();
	printf("Test successful\n");
	return EXIT_SUCCESS;
}
