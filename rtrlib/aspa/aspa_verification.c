/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "rtrlib/aspa/aspa_private.h"
#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/rtrlib_export_private.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

static void *binary_search_asns(const uint32_t cas, uint32_t *array, size_t len)
{
	size_t mid, top;
	int val;
	uint32_t *pivot, *base = array;

	mid = len;
	top = len;

	while (mid) {
		mid = top / 2;
		pivot = base + mid;

		val = cas - *pivot;

		if (val == 0)
			return pivot;

		if (val >= 0)
			base = pivot;

		top -= mid;
	}
	return NULL;
}

enum aspa_hop_result aspa_check_hop(struct aspa_table *aspa_table, uint32_t customer_asn, uint32_t provider_asn)
{
	bool customer_found = false;

	for (struct aspa_store_node *node = aspa_table->store; node != NULL; node = node->next) {
		struct aspa_record *aspa_record = aspa_array_search(node->aspa_array, customer_asn);

		if (!aspa_record)
			continue;

		customer_found = true;

		// Provider ASNs are sorted in ascending order.
		// We consider this an implementation detail, callers must not make any assumptions on the
		// ordering of provider ASNs.
		uint32_t *provider =
			binary_search_asns(provider_asn, aspa_record->provider_asns, aspa_record->provider_count);

		if (provider)
			return ASPA_PROVIDER_PLUS;
	}

	return customer_found ? ASPA_NOT_PROVIDER_PLUS : ASPA_NO_ATTESTATION;
}

static enum aspa_verification_result aspa_verify_as_path_upstream(struct aspa_table *aspa_table, uint32_t as_path[],
								  size_t len)
{
	// Optimized AS_PATH verification algorithm using zero based array
	// where the origin AS has index N - 1 and the latest AS in the AS_PATH
	// has index 0.
	// Doesn't check any hop twice.
	if (len <= 1)
		// Trivially VALID AS_PATH
		return ASPA_AS_PATH_VALID;

	pthread_rwlock_rdlock(&aspa_table->lock);

	// Find apex of up-ramp
	size_t r = len - 1;
	enum aspa_hop_result last_hop_right;

	while (r > 0 && (last_hop_right = aspa_check_hop(aspa_table, as_path[r], as_path[r - 1])) == ASPA_PROVIDER_PLUS)
		r -= 1;

	if (r == 0) {
		// Complete customer-provider chain, VALID upstream AS_PATH
		pthread_rwlock_unlock(&aspa_table->lock);
		return ASPA_AS_PATH_VALID;
	}

	bool found_nP_from_right = false;

	/*
	 * Look for nP+ in the right-to-left/upwards direction
	 * Check if there's a nP+ hop in the gap from the right (facing left/up).
	 * a, The next hop right after the up-ramp was already retrieved from the database,
	 *    so just check if that hop was nP+.
	 * b, Also, don't check last hop before down-ramp starts
	 *    because there must be a hop of space in order for two
	 *    nP+ hops to oppose each other.
	 * c, RR points to the left end of the hop last checked.
	 * d, Checking stops if the hop is nP+.
	 *
	 *        Last chance of finding a relevant nP+ hop
	 *           /
	 *   L      /\                  R
	 *   * -- * -- * . . . . . * -- *
	 *   0   L+1              R-1    \
	 *         |<------------------|  *
	 *                               N-1
	 *
	 */

	size_t rr = r;

	if (last_hop_right == ASPA_NOT_PROVIDER_PLUS) {
		found_nP_from_right = true;
	} else {
		while (rr > 0) {
			size_t c = rr;

			rr--;
			if (aspa_check_hop(aspa_table, as_path[c], as_path[rr]) == ASPA_NOT_PROVIDER_PLUS) {
				found_nP_from_right = true;
				break;
			}
		}
	}

	pthread_rwlock_unlock(&aspa_table->lock);

	// If nP+ occurs upstream customer-provider chain, return INVALID.
	if (found_nP_from_right) {
		return ASPA_AS_PATH_INVALID;
	}

	return ASPA_AS_PATH_UNKNOWN;
}

static enum aspa_verification_result aspa_verify_as_path_downstream(struct aspa_table *aspa_table, uint32_t as_path[],
								    size_t len)
{
	// Optimized AS_PATH verification algorithm using zero based array
	// where the origin AS has index N - 1 and the latest AS in the AS_PATH
	// has index 0.
	// Doesn't check any hop twice.
	if (len <= 2)
		// Trivially VALID AS_PATH
		return ASPA_AS_PATH_VALID;

	pthread_rwlock_rdlock(&aspa_table->lock);

	// Find apex of up-ramp
	size_t r = len - 1;
	enum aspa_hop_result last_hop_right;

	while (r > 0 && (last_hop_right = aspa_check_hop(aspa_table, as_path[r], as_path[r - 1])) == ASPA_PROVIDER_PLUS)
		r--;

	bool found_nP_from_right = false;
	bool found_nP_from_left = false;

	size_t l = 0;
	enum aspa_hop_result last_hop_left;

	// Find down-ramp end
	while (l < r && (last_hop_left = aspa_check_hop(aspa_table, as_path[l], as_path[l + 1])) == ASPA_PROVIDER_PLUS)
		l++;
	assert(l <= r);

	// If gap does not exist (sharp tip) or is just a single hop wide,
	// there's no way to create a route leak, return VALID.
	if (r - l <= 1) {
		pthread_rwlock_unlock(&aspa_table->lock);
		return ASPA_AS_PATH_VALID;
	}

	/*
	 * I. Look for nP+ in the right-to-left/upwards direction
	 * Check if there's a nP+ hop in the gap from the right (facing left/up).
	 * a, The next hop right after the up-ramp was already retrieved from the database,
	 *    so just check if that hop was nP+.
	 * b, Also, don't check last hop before down-ramp starts
	 *    because there must be a hop of space in order for two
	 *    nP+ hops to oppose each other.
	 * c, RR points to the left end of the hop last checked.
	 * d, Checking stops if the hop is nP+.
	 *
	 *        Last chance of finding a relevant nP+ hop
	 *           /
	 *   L      /\                  R
	 *   * -- * -- * . . . . . * -- *
	 *  /    L+1              R-1    \
	 * *       |<------------------|  *
	 * 0                             N-1
	 *
	 */

	size_t rr = r;

	if (last_hop_right == ASPA_NOT_PROVIDER_PLUS) {
		found_nP_from_right = true;
	} else {
		while (rr > l + 1) {
			size_t c = rr;

			rr--;
			if (aspa_check_hop(aspa_table, as_path[c], as_path[rr]) == ASPA_NOT_PROVIDER_PLUS) {
				found_nP_from_right = true;
				break;
			}
		}
	}

	// II. Look for nP+ in the left-to-right/down direction
	// Check if there's a nP+ hop in the gap from the right (facing left/down).
	if (found_nP_from_right) {
		/*
		 * a, There's no need to check for an nP+ from the left if we
		 *    didn't find an nP+ from the right before.
		 * b, The next hop right after the down-ramp was already retrieved from the database,
		 *    so just check if that hop was nP+.
		 * c, LL points to the right end of the hop last checked.
		 * d, Checking stops if the hop is nP+.
		 *
		 *                    Last chance of finding a relevant nP+ hop
		 *                       /
		 *  L    LL             /\
		 *   * -- * . . . . . * -- * . . . . .
		 *  /    L+1               RR
		 * *       |------------->|
		 * 0
		 *
		 */
		size_t ll = l + 1;

		if (last_hop_left == ASPA_NOT_PROVIDER_PLUS) {
			found_nP_from_left = true;
		} else {
			while (ll < rr) {
				size_t c = ll;

				ll++;
				if (aspa_check_hop(aspa_table, as_path[c], as_path[ll]) == ASPA_NOT_PROVIDER_PLUS) {
					found_nP_from_left = true;
					break;
				}
			}
		}
	}

	// If two nP+ occur in opposing directions, return INVALID.
	if (found_nP_from_right && found_nP_from_left) {
		pthread_rwlock_unlock(&aspa_table->lock);
		return ASPA_AS_PATH_INVALID;
	}

	pthread_rwlock_unlock(&aspa_table->lock);
	return ASPA_AS_PATH_UNKNOWN;
}

RTRLIB_EXPORT enum aspa_verification_result aspa_verify_as_path(struct aspa_table *aspa_table, uint32_t as_path[],
								size_t len, enum aspa_direction direction)
{
	switch (direction) {
	case ASPA_UPSTREAM:
		return aspa_verify_as_path_upstream(aspa_table, as_path, len);
	case ASPA_DOWNSTREAM:
		return aspa_verify_as_path_downstream(aspa_table, as_path, len);
	}

	return ASPA_AS_PATH_UNKNOWN;
}

RTRLIB_EXPORT size_t aspa_collapse_as_path(uint32_t as_path[], size_t len)
{
	if (len == 0)
		return 0;

	size_t i = 1;

	while (i < len && as_path[i - 1] != as_path[i])
		i++;

	if (i == len)
		return len;

	size_t j = i;

	i++;

	while (true) { // equivalent to while (i < len)
		while (i < len && as_path[i - 1] == as_path[i])
			i++;

		if (i == len)
			break;

		as_path[j++] = as_path[i++];
	}

	return j;
}
