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
/*
static void update_cb(struct aspa_table *t __attribute__((unused)), const struct aspa_record rec, const struct rtr_socket *socket, const bool added)
{
	switch (rec.customer_asn) {
		case 123:
			assert(added == true);
			assert(rec.provider_count == 0);
			assert(rec.provider_asns == NULL);
			break;
		case 456:
			assert(added == true);
			assert(rec.provider_count == 4);
			assert(rec.provider_asns);
			assert(rec.provider_asns[0] == 111);
			assert(rec.provider_asns[1] == 222);
			assert(rec.provider_asns[2] == 333);
			assert(rec.provider_asns[3] == 444);
			break;
		case 777:
			// 777 should be annihilated
			abort();
	}
}

static void setup(struct aspa_table **table, struct rtr_socket **socket1, struct rtr_socket **socket2)
{
	*table = lrtr_malloc(sizeof(struct aspa_table));
	assert(table != NULL);
	aspa_table_init(*table, update_cb);
	
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
	uint32_t *providers = lrtr_malloc(4);
	providers[0] = 111;
	providers[1] = 222;
	providers[2] = 333;
	providers[3] = 444;
	
	struct aspa_update_operation ops[] = {
		(struct aspa_update_operation) {
			.type = ASPA_ADD,
			.record = (struct aspa_record) {
				.customer_asn = 123,
				.provider_count = 0,
				.provider_asns = NULL
			}
		},
		(struct aspa_update_operation) {
			.type = ASPA_ADD,
			.record = (struct aspa_record) {
				.customer_asn = 456,
				.provider_count = 4,
				.provider_asns = providers
			}
		}
	};
		
	struct aspa_update *update = lrtr_malloc(sizeof(struct aspa_update));
	struct aspa_update_operation *failed_op = NULL;
	
	assert(aspa_table_compute_update(table, ops, sizeof(ops) / sizeof(ops[0]), socket, update, &failed_op) == ASPA_SUCCESS);
	assert(failed_op == NULL);
	assert(update->stale_provider_arrays_count == 0);
	assert(update->stale_provider_arrays == NULL);
	
	assert(aspa_table_apply_update(update) == ASPA_SUCCESS);
	
	// check fast lookup
	assert(table == socket->aspa_table);
	
	assert(table->store);
	assert(table->store->next == NULL);
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

static void test_src_remove(struct aspa_table *table, struct rtr_socket *socket)
{
	assert(aspa_table_src_remove(table, socket, false) == ASPA_SUCCESS);
	assert(table->store == NULL);
	assert(socket->aspa_array == NULL);
	return;
};

static void test_nullifying(struct aspa_table *table, struct rtr_socket *socket)
{
	uint32_t *providers = lrtr_malloc(4);
	providers[0] = 55;
	providers[1] = 66;
	
	struct aspa_update_operation ops[] = {
		(struct aspa_update_operation) {
			.type = ASPA_ADD,
			.record = (struct aspa_record) {
				.customer_asn = 123,
				.provider_count = 0,
				.provider_asns = NULL
			}
		},
		(struct aspa_update_operation) {
			.type = ASPA_ADD,
			.record = (struct aspa_record) {
				.customer_asn = 777,
				.provider_count = 4,
				.provider_asns = providers
			}
		},
		(struct aspa_update_operation) {
			.type = ASPA_REMOVE,
			.record = (struct aspa_record) {
				.customer_asn = 777,
				.provider_count = 0,
				.provider_asns = NULL
			}
		}
	};
		
	struct aspa_update *update = lrtr_malloc(sizeof(struct aspa_update));
	struct aspa_update_operation *failed_op = NULL;
	
	assert(aspa_table_compute_update(table, ops, sizeof(ops) / sizeof(ops[0]), socket, update, &failed_op) == ASPA_SUCCESS);
	assert(failed_op == NULL);
	assert(update->stale_provider_arrays_count == 1);
	assert(update->stale_provider_arrays[0] == providers);
	
	assert(aspa_table_apply_update(update) == ASPA_SUCCESS);
	
	// check fast lookup
	assert(table == socket->aspa_table);
	
	assert(table->store);
	assert(table->store->next == NULL);
	assert(table->store->rtr_socket == socket);
	assert(table->store->aspa_array == socket->aspa_array);
	
	// check that the last two ops really annihilate each other
	assert(socket->aspa_array->size == 1);
	assert(socket->aspa_array->data[0].customer_asn == 123);
	assert(socket->aspa_array->data[0].provider_count == 0);
	assert(socket->aspa_array->data[0].provider_asns == NULL);
	
	return;
}

static void test_remove_existing(struct aspa_table *table, struct rtr_socket *socket)
{
	struct aspa_update_operation ops[] = {
		(struct aspa_update_operation) {
			.type = ASPA_REMOVE,
			.record = (struct aspa_record) {
				.customer_asn = 456,
				.provider_count = 0,
				.provider_asns = NULL
			}
		},
	};
		
	struct aspa_update *update = lrtr_malloc(sizeof(struct aspa_update));
	struct aspa_update_operation *failed_op = NULL;
	
	assert(aspa_table_compute_update(table, ops, sizeof(ops) / sizeof(ops[0]), socket, update, &failed_op) == ASPA_SUCCESS);
	// TODO: aspa_table_compute_update doesn't throw here, but it should based on the typo above (456 instead of 123)
	assert(failed_op == NULL);
//	assert(update->stale_provider_arrays_count == 0);
//	assert(update->stale_provider_arrays == NULL);
	
	assert(aspa_table_apply_update(update) == ASPA_SUCCESS);
	
	// check fast lookup
	assert(table == socket->aspa_table);
	
	assert(table->store);
	assert(table->store->next == NULL);
	assert(table->store->rtr_socket == socket);
	assert(table->store->aspa_array == socket->aspa_array);
	
	// check that the last two ops really annihilate each other
	assert(socket->aspa_array->size == 0);
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
	test_src_remove(table, socket1);
	test_nullifying(table, socket1);
	test_remove_existing(table, socket1);

	return EXIT_SUCCESS;
}
*/

int main() { return 0; }
