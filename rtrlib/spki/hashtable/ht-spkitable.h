/*
 * This file is part of RTRlib.
 *
 * RTRlib is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * RTRlib is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with RTRlib; see the file COPYING.LESSER.
 *
 * written by Andreas Reuter and Nils Bars, in cooperation with:
 * INET group, Hamburg University of Applied Sciences,
 * CST group, Freie Universitaet Berlin
 * Website: http://rpki.realmv6.org/
 */
#ifndef RTR_HT_SPKITABLE_H
#define RTR_HT_SPKITABLE_H

#include "rtrlib/spki/spkitable.h"
#include "rtrlib/spki/hashtable/tommyds-1.8/tommy.h"



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
