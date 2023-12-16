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
#include "rtrlib/aspa/aspa_store/aspa_store.h"
#include <stdbool.h>

#include <string.h>
#include <assert.h>

static void update_cb(struct aspa_table *t __attribute__((unused)), const struct aspa_record rec, const bool added)
{
	return;
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
	(*socket2)->aspa_table = NULL;
	return;
}

static void test_add_empty_providers(struct aspa_table *table, struct rtr_socket *socket)
{
	struct aspa_update_operation ops[] = {
		(struct aspa_update_operation) {
			.type = ASPA_ADD,
			.record = (struct aspa_record) {
				.customer_asn = 123,
				.provider_count = 0,
				.provider_asns = (uint32_t[]){ 23,25,89,2,123,2323 }
			}
		},
		(struct aspa_update_operation) {
			.type = ASPA_ADD,
			.record = (struct aspa_record) {
				.customer_asn = 456,
				.provider_count = 4,
				.provider_asns = (uint32_t[]){ 111,222,333,444 }
			}
		}
	};
		
	struct aspa_update *update = lrtr_malloc(sizeof(struct aspa_update));
	struct aspa_update_operation *failed_op = NULL;
	
	assert(aspa_table_compute_update(table, ops, sizeof(ops) / sizeof(ops[0]), socket, update, &failed_op) == ASPA_SUCCESS);
	assert(failed_op == NULL);
	
	assert(aspa_table_apply_update(update) == ASPA_SUCCESS);
	
	// check fast lookup
	assert(table == socket->aspa_table);
	
	assert(table->store->rtr_socket == socket);
	assert(table->store->aspa_array == socket->aspa_array);
	assert(socket->aspa_array->size == 2);
	assert(socket->aspa_array->data[0].customer_asn == 123);
	assert(socket->aspa_array->data[0].provider_count == 0);
	assert(socket->aspa_array->data[0].provider_asns == NULL);
	
	assert(socket->aspa_array->data[1].customer_asn == 456);
	assert(socket->aspa_array->data[1].provider_count == 4);
	assert(socket->aspa_array->data[1].provider_asns[0] == 111);
	assert(socket->aspa_array->data[1].provider_asns[3] == 444);
	return;
}

static void test_nullifying(struct aspa_table *table, struct rtr_socket *socket)
{
	return;
}

static void test_remove_existing(struct aspa_table *table, struct rtr_socket *socket)
{
	return;
}

static void test_nullifying_and_remove_existing(struct aspa_table *table, struct rtr_socket *socket)
{
	return;
}

static void test_unknown_withdrawal(struct aspa_table *table, struct rtr_socket *socket)
{
	return;
}

static void test_add_existing(struct aspa_table *table, struct rtr_socket *socket)
{
	return;
}

static void test_add_duplicates(struct aspa_table *table, struct rtr_socket *socket)
{
	return;
}

static void test_both_errors(struct aspa_table *table, struct rtr_socket *socket)
{
	return;
}

int main(void)
{
	struct aspa_table *table;
	struct rtr_socket *socket1;
	struct rtr_socket *socket2;
	
	setup(&table, &socket1, &socket2);
	
	test_add_empty_providers(table, socket1);

	return EXIT_SUCCESS;
}
