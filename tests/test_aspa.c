/*
* This file is part of RTRlib.
*
* This file is subject to the terms and conditions of the MIT license.
* See the file LICENSE in the top level directory for more details.
*
* Website; http://rtrlib.realmv6.org/
*/

#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/aspa/aspa_private.h"
#include "rtrlib/aspa/aspa_array/aspa_array.h"
#include <stdbool.h>

#include <string.h>
#include <assert.h>

static void update_cb(struct aspa_table *t __attribute__((unused)), const struct aspa_record rec, const bool added)
{
	
}

static void setup(struct aspa_table **table, struct rtr_socket **socket1, struct rtr_socket **socket2)
{
	*table = lrtr_malloc(sizeof(struct aspa_table));
	assert(table != NULL);
	aspa_table_init(*table, NULL);
	
	*socket1 = lrtr_malloc(sizeof(struct rtr_socket));
	assert(*socket1 != NULL);
	(*socket1)->aspa_table = *table;
	
	*socket2 = lrtr_malloc(sizeof(struct rtr_socket));
	assert(*socket2 != NULL);
	(*socket2)->aspa_table = *table;
}

int main(void)
{
	struct aspa_table *table;
	struct rtr_socket *socket1;
	struct rtr_socket *socket2;
	
	setup(&table, &socket1, &socket2);
	

	return EXIT_SUCCESS;
}
