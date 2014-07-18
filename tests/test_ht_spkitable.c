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
#include <time.h>
#include <stdlib.h>


static struct spki_record *create_record(int ASN, int ski_offset, int spki_offset, struct rtr_socket *socket);





static bool compare_spki_records(struct spki_record *r1, struct spki_record *r2){
    if(r1->asn != r2->asn){
        return false;
    }
    if(r1->socket != r2->socket){
        return false;
    }
    if(memcmp(r1->ski, r2->ski, SKI_SIZE) != 0){
        return false;
    }
    if(memcmp(r1->spki, r2->spki, SPKI_SIZE) != 0){
        return false;
    }
    return true;
}

static struct spki_record *create_record(int ASN, int ski_offset, int spki_offset, struct rtr_socket *socket) {
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

static void test_add_1(){
    struct spki_table table;
    spki_table_init(&table,NULL);

    struct rtr_socket *socket_one = malloc(sizeof(struct rtr_socket));
    struct rtr_socket *socket_two = malloc(sizeof(struct rtr_socket));
    //Create 255 records with same ASN and SKI, different SPKI. Add them
    struct spki_record *record;
    for(unsigned int i = 0; i< 255;i++){
        if(i%2 != 0)
            record = create_record(1,0,i,socket_one);
        if(i%2 == 0)
            record = create_record(1,0,i,socket_two);

        assert(spki_table_add_entry(&table, record) == SPKI_SUCCESS);

    }

    struct spki_record *result;
    unsigned int result_size;
    spki_table_get_all(&table, record->asn, record->ski,&result, &result_size);

    int count = 0;

    for(unsigned int i = 0; i < result_size; i++){
        assert(result->asn == record->asn);
        assert(memcmp(&result[i].ski,record->ski,SKI_SIZE) == 0);
        count++;
    }
    assert(count == 255);

    spki_table_src_remove(&table,socket_one);
    spki_table_get_all(&table, record->asn, record->ski,&result,&result_size);

    for(unsigned int i = 0; i < result_size; i++){
        assert(result->asn == record->asn);
        assert(memcmp(&result[i].ski,record->ski,SKI_SIZE) == 0);
        count++;
    }

    spki_table_free(&table);
    free(result);

    printf("test_add_1() complete\n");
}

static void test_add_2(){
    struct spki_table table;
    spki_table_init(&table,NULL);

    //Test: Two diff SPKIs hash to the same SKI
    struct spki_record *record1 = create_record(10,20,30,NULL);
    struct spki_record *record2 = create_record(10,20,40,NULL);


    assert(spki_table_add_entry(&table, record1) == SPKI_SUCCESS);
    assert(spki_table_add_entry(&table, record2) == SPKI_SUCCESS);

    struct spki_record *result;
    unsigned int result_size;
    spki_table_get_all(&table, 10, record1->ski, &result, &result_size);
    assert(result_size == 2);

    assert(compare_spki_records(&result[0], record1) != compare_spki_records(&result[0], record2));
    assert(compare_spki_records(&result[1], record1) != compare_spki_records(&result[1], record2));
    free(result);

    spki_table_remove_entry(&table, record1);
    spki_table_get_all(&table, 10, record1->ski, &result, &result_size);
    assert(result_size == 1);

    assert(compare_spki_records(&result[0], record2));
    free(result);

    spki_table_remove_entry(&table, record2);
    spki_table_get_all(&table, 10, record1->ski, &result, &result_size);
    assert(result_size == 0);
    assert(result == NULL);

    spki_table_free(&table);

    //Test: Add equal records
    spki_table_init(&table,NULL);
    record1 = create_record(10,10,10,NULL);
    record2 = create_record(10,10,10,NULL);

    assert(spki_table_add_entry(&table, record1) == SPKI_SUCCESS);
    assert(spki_table_add_entry(&table, record2) == SPKI_DUPLICATE_RECORD);
    assert(spki_table_add_entry(&table, record1) == SPKI_DUPLICATE_RECORD);

    spki_table_get_all(&table, 10, record1->ski, &result, &result_size);
    assert(result_size == 1);

    spki_table_free(&table);
    printf("test_add_2() complete\n");
}

static void test_remove_1(){
    struct spki_table table;
    spki_table_init(&table,NULL);

    struct spki_record *result;
    unsigned int result_size;

    //Test: Add 4 records with only one field differ from the first record.
    //-> Checks the compare function
    struct spki_record *record1 = create_record(10,10,10,NULL);
    struct spki_record *record2 = create_record(10,10,11,NULL);
    struct spki_record *record3 = create_record(10,11,10,NULL);
    struct spki_record *record4 = create_record(11,10,10,NULL);

    assert(spki_table_add_entry(&table, record1) == SPKI_SUCCESS);
    assert(spki_table_add_entry(&table, record2) == SPKI_SUCCESS);
    assert(spki_table_add_entry(&table, record3) == SPKI_SUCCESS);
    assert(spki_table_add_entry(&table, record4) == SPKI_SUCCESS);

    assert(spki_table_remove_entry(&table, record1) == SPKI_SUCCESS);
    //Check if other records are still there
    assert(spki_table_get_all(&table, record2->asn, record2->ski, &result, &result_size) == SPKI_SUCCESS);
    assert(result_size == 1);
    free(result);

    assert(spki_table_get_all(&table, record2->asn, record2->ski, &result, &result_size) == SPKI_SUCCESS);
    assert(result_size == 1);
    free(result);

    assert(spki_table_get_all(&table, record2->asn, record2->ski, &result, &result_size) == SPKI_SUCCESS);
    assert(result_size == 1);
    free(result);

    spki_table_free(&table);
        printf("test_remove_1() complete\n");
}

int main(){
    test_add_1();
    test_add_2();

    test_remove_1();

    return EXIT_SUCCESS;
}
