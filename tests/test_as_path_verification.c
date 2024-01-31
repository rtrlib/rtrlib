/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website; http://rtrlib.realmv6.org/
 */

/*
 * AS_PATH verification tests are derived from
 * https://github.com/netd-tud/IETF-ASPA/blob/master/ietf-hackathon/tests.py
 */

#include "rtrlib/aspa/aspa_array/aspa_array.h"
#include "rtrlib/aspa/aspa_private.h"
#include "rtrlib/lib/alloc_utils_private.h"

#include <assert.h>
#include <string.h>

#define ASNS(...) ((uint32_t[]){__VA_ARGS__})

// clang-format off

#define RECORD(cas, providers) ((struct aspa_record) { \
	.customer_asn = cas, \
	.provider_count = (size_t)(sizeof(providers) / sizeof(uint32_t)), \
	.provider_asns = sizeof(providers) == 0 ? NULL : providers \
})

#define ADD_OPERATION(idx, rec) ((struct aspa_update_operation) { \
	.index = idx, \
	.record = rec, \
	.type = ASPA_ADD, \
	.is_no_op = false \
})

#define BUILD_ASPA_TABLE(tablename, socketname, ...) \
	struct aspa_table *(tablename) = lrtr_malloc(sizeof(*tablename)); \
	assert((tablename)); \
	aspa_table_init((tablename), NULL); \
	\
	NEW_SOCKET_ADD_RECORDS((tablename), (socketname), __VA_ARGS__) \

#define NEW_SOCKET_ADD_RECORDS(tablename, socketname, ...) \
	struct rtr_socket *socketname = lrtr_malloc(sizeof(struct rtr_socket)); \
	assert(socketname); \
	socketname->aspa_table = (tablename); \
	{ \
		struct aspa_record records[] = { __VA_ARGS__ }; \
		size_t len = sizeof(records) / sizeof(struct aspa_record); \
		\
		if (len) { \
			struct aspa_update *update = NULL; \
			struct aspa_update_operation *operations = \
				lrtr_malloc(len * sizeof(struct aspa_update_operation)); \
			for (size_t i = 0; i < len; i++) \
				operations[i] = ADD_OPERATION(i, records[i]); \
			\
			assert(aspa_table_update_swap_in_compute((tablename), \
				(socketname), operations, len, &update) \
				== ASPA_SUCCESS); \
			aspa_table_update_swap_in_apply(&update); \
		} \
	}

#define VERIFY_AS_PATH(aspa_table, direction, result, asns) \
	assert((result) == aspa_verify_as_path(aspa_table, asns, sizeof(asns) / sizeof(uint32_t), direction))

static struct aspa_table *test_create_aspa_table(void)
{
	struct aspa_table *aspa_table = lrtr_malloc(sizeof(*aspa_table));

	assert(aspa_table);
	aspa_table_init(aspa_table, NULL);

	NEW_SOCKET_ADD_RECORDS(aspa_table, rtr_socket_1,
		RECORD(100, ASNS(200, 201)),
		RECORD(200, ASNS(300)),
		RECORD(300, ASNS(400)),
		RECORD(400, ASNS(500)),

		RECORD(501, ASNS(601)),
		RECORD(401, ASNS(501)),
		RECORD(301, ASNS(401)),
		RECORD(201, ASNS(301)),

		RECORD(502, ASNS(602)),
		RECORD(402, ASNS(502)),
		RECORD(302, ASNS(402)),
		RECORD(202, ASNS(302)),

		// 103 --> 203 <--> 303 <--> 403 <-- 304
		RECORD(103, ASNS(203)),
		RECORD(203, ASNS(303)),
		RECORD(303, ASNS(203, 403)),
		RECORD(403, ASNS(303)),
		RECORD(304, ASNS(403)),
	);

	NEW_SOCKET_ADD_RECORDS(aspa_table, rtr_socket_2,
		RECORD(100, ASNS(200, 202))
	);

	return aspa_table;
}

// clang-format on

static void test_hopping(struct aspa_table *aspa_table)
{
	// check that provider and not provider holds
	assert(aspa_check_hop(aspa_table, 100, 200) == ASPA_PROVIDER_PLUS);
	assert(aspa_check_hop(aspa_table, 200, 100) == ASPA_NOT_PROVIDER_PLUS);

	assert(aspa_check_hop(aspa_table, 200, 300) == ASPA_PROVIDER_PLUS);
	assert(aspa_check_hop(aspa_table, 500, 999) == ASPA_NO_ATTESTATION);

	assert(aspa_check_hop(aspa_table, 999, 999) == ASPA_NO_ATTESTATION);

	// multiple dissimilar aspas
	assert(aspa_check_hop(aspa_table, 100, 201) == ASPA_PROVIDER_PLUS);
	assert(aspa_check_hop(aspa_table, 100, 202) == ASPA_PROVIDER_PLUS);
}

// test multiple routes
// - upstream (only customer-provider-hops)
//   - one not provider and one not attested: invalid
//   - one not attested: unknown
//   - all attested: valid

static void test_upstream(struct aspa_table *aspa_table)
{
	// empty paths are valid
	assert(aspa_verify_as_path(aspa_table, NULL, 0, ASPA_UPSTREAM) == ASPA_AS_PATH_VALID);

	// paths of length 1 are valid
	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_VALID, ASNS(100));

	// valid upstream paths
	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_VALID, ASNS(200, 100));
	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_VALID, ASNS(300, 200));
	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_VALID, ASNS(300, 200, 100));

	// single not-provider hop (nP)

	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_INVALID, ASNS(999, 100));
	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_INVALID, ASNS(300, 999, 100));
	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_INVALID, ASNS(999, 999, 100));
	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_INVALID, ASNS(999, 100, 999));

	// single unattested hop (nA)
	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_UNKNOWN, ASNS(999, 500, 400, 300));
}

static void test_downstream(struct aspa_table *aspa_table)
{
	// paths of length 1 <= N <= 2 are valid
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_VALID, ASNS(999));
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_VALID, ASNS(998, 999));

	// either up- or down-ramp is valid, not both
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_VALID, ASNS(300, 400, 500));
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_VALID, ASNS(500, 400, 300));

	// w/o customer-provider gap
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_VALID, ASNS(300, 400, 500, 400, 300));

	// single not-provider (nP) in between
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_VALID, ASNS(302, 402, 502, 500, 400, 300));

	// two highest-level hops are nP
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_INVALID, ASNS(301, 401, 501, 502, 502, 402, 302));
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_UNKNOWN, ASNS(302, 402, 502, 999, 500, 400, 300));

	// single nA at highest level is valid
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_VALID, ASNS(999, 500, 400, 300));
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_VALID, ASNS(300, 400, 500, 999));

	// single nP at highest level is valid
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_VALID, ASNS(999, 502, 402, 302));
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_VALID, ASNS(302, 402, 502, 999));

	// the last hop in the down ramp must be valid
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_UNKNOWN, ASNS(999, 300, 400, 500));
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_INVALID, ASNS(100, 300, 400, 500));

	// the first hop in the up ramp must be valid
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_UNKNOWN, ASNS(500, 400, 300, 999));
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_INVALID, ASNS(500, 400, 300, 100));

	// consecutive up-ramps are invalid
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_INVALID, ASNS(400, 300, 200, 502, 402, 302));

	// consecutive down-ramps are invalid
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_INVALID, ASNS(200, 300, 400, 302, 402, 502));

	// both down- and up-ramp are invalid
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_INVALID, ASNS(400, 300, 200, 302, 402, 502));

	// overlapping customer-provider-relationships
	// 103 --> 203 <--> 303 <--> 403 <-- 304
	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_VALID, ASNS(304, 403, 303, 203, 103));

	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_UNKNOWN, ASNS(20, 30, 90, 40, 70, 80));
}

// clang-format off

/**
 * Example 1 (downstream) (valid)
 *
 * as_path: 20, 30, 40, 70, 80
 *
 *          30   40
 *  10  20           70
 *                       80 (origin)
 *
 * customer-providers:
 *   80: 70
 *   70: 40
 *   20: 30
 *
 */
static void test_verify_example_1(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(80, ASNS(70)),
		RECORD(70, ASNS(40)),
		RECORD(20, ASNS(30)),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_VALID,
		ASNS(20, 30, 40, 70, 80));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 2 (downstream) (unknown)
 *
 * as_path: 20, 30, 90, 40, 70, 80
 *
 *          30       40
 *  10   20       90      70
 *                           80 (origin)
 *
 * customer-providers:
 *   80: 70
 *   70: 40
 *   20: 30
 *   90: 30, 40
 *
 */
static void test_verify_example_2(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(80, ASNS(70)),
		RECORD(70, ASNS(40)),
		RECORD(20, ASNS(30)),
		RECORD(90, ASNS(30, 40)),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_UNKNOWN,
		ASNS(20, 30, 90, 40, 70, 80));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 2b (downstream) (invalid)
 *
 * as_path: 20, 30, 90, 40, 70, 80
 *
 *          30*      40*
 *  10  20       90      70
 *                           80 (origin)
 *
 * customer-providers:
 *   80: 70
 *   70: 40
 *   20: 30
 *   90: 30, 40
 *   30:   (none)
 *   40:   (none)
 *
 */
static void test_verify_example_2b(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(80, ASNS(70)),
		RECORD(70, ASNS(40)),
		RECORD(20, ASNS(30)),
		RECORD(90, ASNS(30, 40)),
		RECORD(30, ASNS()),
		RECORD(40, ASNS()),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_INVALID,
		ASNS(20, 30, 90, 40, 70, 80));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 3a (downstream) (unknown)
 *
 * as_path: 20, 30, 90, 40, 70, 80
 *
 *          30   90  40
 *      20               70
 *  10                       80 (origin)
 *
 * customer-providers:
 *   80: 70
 *   70: 40
 *   20: 30
 *
 */
static void test_verify_example_3a(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(80, ASNS(70)),
		RECORD(70, ASNS(40)),
		RECORD(20, ASNS(30)),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_UNKNOWN,
		ASNS(20, 30, 90, 40, 70, 80));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 3b (downstream) (unknown)
 *
 * as_path: 20, 30, 90, 100, 40, 70, 80
 *
 *          30   90  100 40
 *  10  20                 70
 *                            80 (origin)
 *
 * customer-providers:
 *   80: 70
 *   70: 40
 *   20: 30
 *
 */
static void test_verify_example_3b(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(80, ASNS(70)),
		RECORD(70, ASNS(40)),
		RECORD(20, ASNS(30)),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_UNKNOWN,
		ASNS(20, 30, 90, 100, 40, 70, 80));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 3c (downstream) (invalid)
 *
 * as_path: 20, 30, 90, 100, 40, 70, 80
 *
 *       30*  90  100  40*
 *     20                 70
 *  10                      80 (origin)
 *
 * customer-providers:
 *   80: 70
 *   70: 40
 *   20: 30
 *   30:   (none)
 *   40:   (none)
 *
 */
static void test_verify_example_3c(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(80, ASNS(70)),
		RECORD(70, ASNS(40)),
		RECORD(20, ASNS(30)),
		RECORD(30, ASNS()),
		RECORD(40, ASNS()),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_INVALID,
		ASNS(20, 30, 90, 100, 40, 70, 80));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 3d (downstream) (unknown)
 *
 * as_path: 20, 30, 40, 100, 90, 70, 80
 *
 *         30*  40* 100? 90?
 *  10  20                  70
 *                            80 (origin)
 *
 * customer-providers:
 *   80: 70
 *   70: 90
 *   20: 30
 *   30:   (none)
 *   40:   (none)
 *
 */
static void test_verify_example_3d(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(80, ASNS(70)),
		RECORD(70, ASNS(90)),
		RECORD(20, ASNS(30)),
		RECORD(30, ASNS()),
		RECORD(40, ASNS()),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_UNKNOWN,
		ASNS(20, 30, 40, 100, 90, 70, 80));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 3f (downstream) (unknown)
 *
 * as_path: 20, 30, 40, 100, 90, 70, 80
 *
 *          30   40  100 90
 *  10  20                 70
 *                            80 (origin)
 *
 * customer-providers:
 *   80: 70
 *   70: 90
 *   20: 30
 *   100:   (none)
 *   40:   (none)
 *
 */
static void test_verify_example_3f(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(80, ASNS(70)),
		RECORD(70, ASNS(90)),
		RECORD(20, ASNS(30)),
		RECORD(100, ASNS()),
		RECORD(40, ASNS()),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_UNKNOWN,
		ASNS(20, 30, 40, 100, 90, 70, 80));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 4 (upstream) (invalid)
 *
 * as_path: 20, 30, 40, 50, 60, 70, 80
 *
 *  10                               80 (origin)
 *    20   30    40    50   60   70
 *
 * customer-providers:
 *   70: 80
 *
 */
static void test_verify_example_4(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(70, ASNS(80)),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_INVALID,
		ASNS(20, 30, 40, 50, 60, 70, 80));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 4-fixed (upstream) (invalid)
 *
 * as_path: 20, 30, 40, 50, 60, 70, 80
 *
 *  10                      80 (origin)
 *    20                70
 *      30  40  50  60
 *
 * customer-providers:
 *   70: 80
 *   60: 70
 *   30: 20
 *
 */
static void test_verify_example_4_fixed(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(70, ASNS(80)),
		RECORD(60, ASNS(70)),
		RECORD(30, ASNS(20)),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_INVALID,
		ASNS(20, 30, 40, 50, 60, 70, 80));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 5 (upstream) (valid)
 *
 * as_path: 20, 30, 40
 *
 * 10  20
 *        30
 *           40 (origin)
 *
 * customer-providers:
 *   40: 30
 *   30: 20
 *
 */
static void test_verify_example_5(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(40, ASNS(30)),
		RECORD(30, ASNS(20)),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_VALID,
		ASNS(20, 30, 40));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 6 (downstream) (invalid)
 *
 * as_path: 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120
 *
 *         50         90
 *       40  60 70 80   100
 *     30                  110
 *   20                       120
 * 10
 *
 * customer-providers:
 *   120: 110
 *   110: 100
 *   100: 90
 *   80: 90
 *   60: 50
 *   40: 50
 *   30: 40
 *   20: 30
 *
 */
static void test_verify_example_6(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(120, ASNS(110)),
		RECORD(110, ASNS(100)),
		RECORD(100, ASNS(90)),
		RECORD(80, ASNS(90)),
		RECORD(60, ASNS(50)),
		RECORD(40, ASNS(50)),
		RECORD(30, ASNS(40)),
		RECORD(20, ASNS(30)),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_INVALID,
		ASNS(20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 7 (downstream) (unknown)
 *
 * as_path: 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140
 *
 * read from right: 100 -U-> 90 -U-> 80 -U-> 70 -U-> 60 -U-> 50
 * read from left: 50 -U-> 60 -U-> 70 -U-> 80 -P+-> 90 -P+-> 100
 *
 *                        100
 *                     90     110
 *         50 60 70 80           120
 *       40                         130
 *     30                              140
 *   20
 * 10
 *
 * customer-providers:
 *   20: 30
 *   30: 40
 *   40: 50
 *   80: 90
 *   90: 100
 *   110: 100
 *   120: 110
 *   130: 120
 *   140: 130
 *
 */
static void test_verify_example_7(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(20, ASNS(30)),
		RECORD(30, ASNS(40)),
		RECORD(40, ASNS(50)),
		RECORD(80, ASNS(90)),
		RECORD(90, ASNS(100)),
		RECORD(110, ASNS(100)),
		RECORD(120, ASNS(110)),
		RECORD(130, ASNS(120)),
		RECORD(140, ASNS(130)),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_UNKNOWN,
		ASNS(20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 8 (downstream) (valid)
 *
 * as_path: 20
 *
 *   20
 * 10
 *
 * customer-providers:
 * (none)
 *
 */
static void test_verify_example_8(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
	)

	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_VALID,
		ASNS(20));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 9 (upstream) (valid)
 *
 * as_path: 20
 *
 * 10
 *   20
 *
 * customer-providers:
 * (none)
 *
 */
static void test_verify_example_9(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
	)

	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_VALID,
		ASNS(20));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 11 (downstream) (valid)
 *
 * as_path: 20, 30
 *
 *   20 30
 *
 * customer-providers:
 *   20:   (none)
 *   30:   (none)
 *
 */
static void test_verify_example_11(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(20, ASNS()),
		RECORD(30, ASNS()),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_DOWNSTREAM, ASPA_AS_PATH_VALID,
		ASNS(20, 30));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 12 (upstream) (unknown)
 *
 * as_path: 20, 30
 *
 *   20 30
 *
 * customer-providers:
 * (none)
 *
 */
static void test_verify_example_12(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
	)

	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_UNKNOWN,
		ASNS(20, 30));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 13 (upstream) (invalid)
 *
 * as_path: 20, 30, 40, 50, 60
 *
 *   20
 *      30
 *         40  50
 *               60
 *
 * customer-providers:
 *   60: 50
 *   50:   (none)
 *   40: 30
 *   30: 20
 *   20:   (none)
 *
 */
static void test_verify_example_13(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(60, ASNS(50)),
		RECORD(50, ASNS()),
		RECORD(40, ASNS(30)),
		RECORD(30, ASNS(20)),
		RECORD(20, ASNS()),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_INVALID,
		ASNS(20, 30, 40, 50, 60));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 14 (upstream) (invalid)
 *
 * as_path: 20, 30, 40, 50, 60
 *
 *     30 <> 40 <> 50 <> 60
 *  20
 *
 * customer-providers:
 *   60: 50
 *   50: 40, 60
 *   40: 30, 50
 *   30: 40
 *   20: 30
 *
 */
static void test_verify_example_14(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(60, ASNS(50)),
		RECORD(50, ASNS(40, 60)),
		RECORD(40, ASNS(30, 50)),
		RECORD(30, ASNS(40)),
		RECORD(20, ASNS(30)),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_INVALID,
		ASNS(20, 30, 40, 50, 60));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 15 (upstream) (invalid)
 *
 * as_path: 20, 30, 40, 50, 60
 *
 *       30 <> 40 <> 50 <> 60
 *   20
 *
 * customer-providers:
 *   60: 50, 20
 *   50: 40, 60
 *   40: 30, 50
 *   30: 40
 *   20: 30
 *
 */
static void test_verify_example_15(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(60, ASNS(50, 20)),
		RECORD(50, ASNS(40, 60)),
		RECORD(40, ASNS(30, 50)),
		RECORD(30, ASNS(40)),
		RECORD(20, ASNS(30)),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_INVALID,
		ASNS(20, 30, 40, 50, 60));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 16 (upstream) (invalid)
 *
 * as_path: 10, 20, 30, 40
 *
 *     20   30
 * 10           40
 *
 * customer-providers:
 *   10: 20
 *   20: 100
 *   40: 30
 *
 */
static void test_verify_example_16(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(10, ASNS(20)),
		RECORD(20, ASNS(100)),
		RECORD(40, ASNS(30)),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_INVALID,
		ASNS(10, 20, 30, 40));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 17 (upstream) (invalid)
 *
 * as_path: 10, 20, 30, 40
 *
 *                 X
 *     20      40
 * 10      30
 *
 * customer-providers:
 *   10: 20
 *   20: 100
 *   40: 30, 50
 *   50: 40
 *
 */
static void test_verify_example_17(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(10, ASNS(20)),
		RECORD(20, ASNS(100)),
		RECORD(40, ASNS(30, 50)),
		RECORD(50, ASNS(40)),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_INVALID,
		ASNS(10, 20, 30, 40));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

/**
 * Example 18 (upstream) (invalid)
 *
 * as_path: 30, 20, 40
 *
 *  30  20*  40
 *
 * customer-providers:
 *   20:   (none)
 *
 */
static void test_verify_example_18(void)
{
	BUILD_ASPA_TABLE(aspa_table, rtr_socket,
		RECORD(20, ASNS()),
	)

	VERIFY_AS_PATH(aspa_table, ASPA_UPSTREAM, ASPA_AS_PATH_INVALID,
		ASNS(30, 20, 40));

	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);
	lrtr_free(rtr_socket);
}

// clang-format on

static void test_single_collapse(uint32_t input[], size_t input_len, uint32_t output[], size_t output_len)
{
	size_t retlen = aspa_collapse_as_path(input, input_len);

	assert(retlen == output_len);

	for (size_t i = 0; i < output_len; i++)
		assert(input[i] == output[i]);
}

static void test_collapse(void)
{
	test_single_collapse(NULL, 0, NULL, 0);
	test_single_collapse((uint32_t[]){1}, 1, (uint32_t[]){1}, 1);
	test_single_collapse((uint32_t[]){1, 1}, 2, (uint32_t[]){1}, 1);
	test_single_collapse((uint32_t[]){1, 2}, 2, (uint32_t[]){1, 2}, 2);
	test_single_collapse((uint32_t[]){1, 1, 1}, 3, (uint32_t[]){1}, 1);
	test_single_collapse((uint32_t[]){1, 1, 2}, 3, (uint32_t[]){1, 2}, 2);
	test_single_collapse((uint32_t[]){1, 2, 2}, 3, (uint32_t[]){1, 2}, 2);
	test_single_collapse((uint32_t[]){1, 2, 2, 2}, 4, (uint32_t[]){1, 2}, 2);
	test_single_collapse((uint32_t[]){1, 2, 2, 3}, 4, (uint32_t[]){1, 2, 3}, 3);
}

int main(void)
{
	struct aspa_table *aspa_table = test_create_aspa_table();

	test_hopping(aspa_table);
	test_upstream(aspa_table);
	test_downstream(aspa_table);

	lrtr_free(aspa_table->store->next->rtr_socket);
	lrtr_free(aspa_table->store->rtr_socket);
	aspa_table_free(aspa_table, false);
	lrtr_free(aspa_table);

	test_verify_example_1();
	test_verify_example_2();
	test_verify_example_2b();
	test_verify_example_3a();
	test_verify_example_3b();
	test_verify_example_3c();
	test_verify_example_3d();
	test_verify_example_3f();
	test_verify_example_4();
	test_verify_example_4_fixed();
	test_verify_example_5();
	test_verify_example_6();
	test_verify_example_7();
	test_verify_example_8();
	test_verify_example_9();
	test_verify_example_11();
	test_verify_example_12();
	test_verify_example_13();
	test_verify_example_14();
	test_verify_example_15();
	test_verify_example_16();
	test_verify_example_17();
	test_verify_example_18();

	test_collapse();
}
