/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */
#ifndef RTR_HT_SPKITABLE_PRIVATE_H
#define RTR_HT_SPKITABLE_PRIVATE_H

#include "rtrlib/spki/spkitable_private.h"

#include "third-party/tommyds/tommyhashlin.h"
#include "third-party/tommyds/tommylist.h"

typedef int (*hash_cmp_fp)(const void *arg, const void *obj);

/**
 * @brief spki_table.
 * @param hashtable Linear hashtable
 * @param list List that holds the same entries as hashtable, used to iterate.
 * @param cmp_fp Compare function used to find entries in the hashtable
 * @param update_fp Update function, called when the hashtable changes
 * @param lock Read-Write lock to prevent data races
 */
struct spki_table {
	tommy_hashlin hashtable;
	tommy_list list;
	hash_cmp_fp cmp_fp;
	spki_update_fp update_fp;
	pthread_rwlock_t lock;
};

#endif
