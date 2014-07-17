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
#include "rtrlib/spki/hashtable/ht-spkitable.h"
#include <stdio.h>
#include <string.h>

static void test_add(){
    struct spki_table table;
    spki_table_init(&table);

    struct spki_record record;
    record.asn = 1;
    record.socket = NULL;

    for(int i = 0; i < sizeof(record.ski); i++){
        record.ski[i] = i;
    }

    for(int i = 0; i < sizeof(record.spki); i++){
        record.spki[i] = i;
    }

    assert(spki_table_add_entry(&table, &record) == SPKI_SUCCESS);

    struct spki_record *result;
    spki_table_get_all(&table, 1, record.ski,&result);

    assert(result->asn == 1);
    assert(result->next == NULL);

    assert(memcmp(record.ski, result->ski, SKI_SIZE) == 0);
    assert(memcmp(record.spki, result->spki, SKI_SIZE) == 0);

    free(result);


}

int main(){
    test_add();
    return EXIT_SUCCESS;
}
