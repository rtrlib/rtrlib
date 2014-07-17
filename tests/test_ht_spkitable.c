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


struct spki_record *create_record(int ASN, int ski_offset, int spki_offset, struct rtr_socket *socket);

struct spki_record *create_record(int ASN, int ski_offset, int spki_offset, struct rtr_socket *socket) {
    struct spki_record *record = malloc(sizeof(struct spki_record));
    record->asn = ASN;
    uint32_t i;

    for(i = 0; i < sizeof(record->ski); i++){
        record->ski[i] = i + ski_offset;
    }

    for(i = 0; i < sizeof(record->spki); i++){
        record->spki[i] = i + spki_offset;
    }
    record->socket = socket;
    return record;
}

static void test_add(){
    struct spki_table table;
    spki_table_init(&table,NULL);

    struct rtr_socket *socket_one = malloc(sizeof(struct rtr_socket));
    struct rtr_socket *socket_two = malloc(sizeof(struct rtr_socket));
    //Create 255 records with same ASN and SKI, different SPKI. Add them
    struct spki_record *record;
    for(int i = 0; i< 255;i++){
        if(i%2 != 0)
            record = create_record(1,0,i,socket_one);
        if(i%2 == 0)
            record = create_record(1,0,i,socket_two);

        assert(spki_table_add_entry(&table, record) == SPKI_SUCCESS);

    }



    struct spki_record *result;
    spki_table_get_all(&table, record->asn, record->ski,&result);

    int count = 0;
    while(result){
        assert(result->asn == record->asn);
        assert(memcmp(result->ski,record->ski,SKI_SIZE) == 0);
        result = result->next;
        count++;
    }

    assert(count == 255);


    spki_table_src_remove(&table,socket_one);
    spki_table_get_all(&table, record->asn, record->ski,&result);

    while(result){
        assert(result->socket == socket_two);
        result = result->next;
    }


    spki_table_free(&table);

    free(result);


}

int main(){
    test_add();
    return EXIT_SUCCESS;
}
