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
 */
#ifndef RTR_HT_SPKITABLE_H
#define RTR_HT_SPKITABLE_H

#include "rtrlib/spki/spkitable.h"
#include "rtrlib/spki/hashtable/tommyds-1.8/tommy.h"



typedef int (*hash_cmp_fp)(const void *arg, const void *obj);

/**
 * @brief spki_table.
 * @param hashtable Linear hashtable
 * @param lock
 */
struct spki_table {
    tommy_hashlin hashtable;
    tommy_list list;
    hash_cmp_fp cmp_fp;
    pthread_rwlock_t lock;
};


 #endif
