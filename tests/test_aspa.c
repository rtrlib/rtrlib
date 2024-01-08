/*
* This file is part of RTRlib.
*
* This file is subject to the terms and conditions of the MIT license.
* See the file LICENSE in the top level directory for more details.
*
* Website; http://rtrlib.realmv6.org/
*/


#include "rtrlib/aspa/aspa_array/aspa_array.h"
#include "rtrlib/aspa/aspa.h"
#include "rtrlib/aspa/aspa_private.h"
#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/lib/convert_byte_order_private.h"
#include "rtrlib/rtr/rtr_pdus.h"
#include "rtrlib/transport/transport.h"
#include "rtrlib/rtr/packets_private.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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


char *data;
size_t data_size;
size_t data_index;

#define c16(X) lrtr_convert_short(TO_HOST_HOST_BYTE_ORDER, X)
#define c32(X) lrtr_convert_long(TO_HOST_HOST_BYTE_ORDER, X)

static int custom_send(const struct tr_socket *socket, const void *pdu,
		const size_t len, const time_t timeout) {
	printf("sent %lu bytes\n", len);

	const struct pdu_error *err = pdu;
	if (err->type == 10) {
		uint32_t* errlen = (uint32_t*)((char*)err->rest + err->len_enc_pdu);
		if ((char*)errlen < (char*)err + c32(err->len)) {
			printf("err msg: %.*s\n", *errlen, (char*)(errlen+1));
		}
	}
	return len;
}

static int custom_recv(const struct tr_socket *socket, const void *buf,
		const size_t len, const time_t timeout) {
	size_t rlen = len;
	if (data_index + rlen > data_size)
		rlen = data_size-data_index;

	memcpy((char*)buf, data+data_index, rlen);
	data_index += rlen;
	printf("read %lu bytes\n", rlen);
	return rlen;
}

static void test_socket_aspa_pdu_parse() {
	struct tr_socket *tr_socket = lrtr_malloc(sizeof(struct tr_socket));
	tr_socket->recv_fp = (tr_recv_fp)&custom_recv;
	tr_socket->send_fp = (tr_send_fp)&custom_send;

	struct rtr_socket *socket = lrtr_malloc(sizeof(struct rtr_socket));
	socket->version = 2;
	socket->state = RTR_SYNC;
	socket->tr_socket = tr_socket;

	data = lrtr_malloc(
			sizeof(struct pdu_cache_response) +
			sizeof(struct pdu_end_of_data_v1_v2) +
			sizeof(struct pdu_aspa));

	struct aspa_table *aspa_table = lrtr_malloc(sizeof(*aspa_table));
	assert(aspa_table);
	aspa_table_init(aspa_table, NULL);
	socket->aspa_table = aspa_table;

	struct pdu_cache_response *cache_response = (struct pdu_cache_response*)data;
	cache_response->ver = 2;
	cache_response->type = 3;
	cache_response->session_id = 0;
	cache_response->len = c32(8);

	struct pdu_aspa *aspa = (struct pdu_aspa*)(data + sizeof(*cache_response));
	aspa->ver = 2;
	aspa->type = 11;
	aspa->zero = 0;
	aspa->len = c32(20);
	aspa->flags = 0x1;
	aspa->afi_flags = 0x3;
	aspa->provider_count = c16(1);
	aspa->customer_asn = c32(100);
	aspa->provider_asns[0] = c32(200);

	struct pdu_end_of_data_v1_v2 *eod = (struct pdu_end_of_data_v1_v2*)((char*)(aspa) + c32(aspa->len));
	eod->ver = 2;
	eod->type = 7;
	eod->session_id = 0;
	eod->len = c32(24);
	eod->sn = 2;
	eod->refresh_interval = 0;
	eod->retry_interval = 0;
	eod->expire_interval = 0;

	data_size = ((char*)eod-data)+sizeof(*eod);
	
	rtr_sync(socket);
}

int main() {
	test_socket_aspa_pdu_parse();
	return 0;
}
