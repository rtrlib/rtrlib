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
#include <pthread.h>
#include <stdio.h>
#include <string.h>


struct add_records_args {
    struct spki_table *table;
    int start_asn;
    int count;
};

struct remove_records_args {
    struct spki_table *table;
    int start_asn;
    int count;
};


static struct spki_record *create_record(int ASN, int ski_offset, int spki_offset, struct rtr_socket *socket);



static bool compare_spki_records(struct spki_record *r1, struct spki_record *r2)
{
    if(r1->asn != r2->asn) {
        return false;
    }
    if(r1->socket != r2->socket) {
        return false;
    }
    if(memcmp(r1->ski, r2->ski, SKI_SIZE) != 0) {
        return false;
    }
    if(memcmp(r1->spki, r2->spki, SPKI_SIZE) != 0) {
        return false;
    }
    return true;
}

static struct spki_record *create_record(int ASN, int ski_offset, int spki_offset, struct rtr_socket *socket)
{
    struct spki_record *record = malloc(sizeof(struct spki_record));
    record->asn = ASN;
    uint32_t i;

    for(i = 0; i < sizeof(record->ski); i++) {
        record->ski[i] = i + ski_offset;
    }

    for(i = 0; i < sizeof(record->spki); i++) {
        record->spki[i] = i + spki_offset;
    }
    record->socket = socket;
    return record;
}



/**
 * Add 'args->count' records to the 'args->table', start with ASN '->start_asn'.
 */
static void add_records(struct add_records_args *args)
{
    printf("Add %i records: ASN [%i..%i]\n",args->count, args->start_asn, args->count+args->start_asn-1);
    for(int i = args->start_asn; i < args->count+args->start_asn; i++) {
        struct spki_record *record = create_record(i,i,i,NULL);
        int ret = spki_table_add_entry(args->table, record);
        assert(ret == SPKI_SUCCESS);
        free(record);
    }
}

static void remove_records(struct remove_records_args *args)
{
    printf("Remove %i records: ASN [%i..%i]\n",args->count, args->start_asn, args->count+args->start_asn-1);
    for(int i = args->start_asn; i < args->count+args->start_asn; i++) {
        struct spki_record *record = create_record(i,i,i,NULL);
        int ret = spki_table_remove_entry(args->table, record);
        assert(ret == SPKI_SUCCESS);
        free(record);
    }
}

/**
 * @brief lock_test1
 * Test concurrent add operations and concurrent delete operations.
 */
static void lock_test1()
{
    unsigned int max_threads = 20;
    unsigned int records_per_thread = 10000;

    struct spki_table spkit;
    spki_table_init(&spkit, NULL);
    pthread_t threads[max_threads];

    //Test 1: Add concurrently spki_records to the table and test
    //        if nothing get lost.
    struct add_records_args args[max_threads];
    for(unsigned int i = 0; i < max_threads; i++) {
        args[i].table = &spkit;
        args[i].start_asn = i * records_per_thread;
        args[i].count = records_per_thread;
        pthread_create(&(threads[i]), NULL, (void * (*)(void *)) add_records, &args[i]);
    }

    //Wait for add operation to finish
    for(unsigned int i=0; i<max_threads; i++) {
        pthread_join(threads[i], NULL);
        printf("Thread %i returned\n", i);
    }

    //Test if alle records are okay
    struct spki_record *result;
    unsigned int result_size = 0;
    for(unsigned int i = 0; i < records_per_thread * max_threads; i++) {
        struct spki_record *record = create_record(i,i,i,NULL);
        spki_table_get_all(&spkit, record->asn, record->ski, &result, &result_size);
        assert(result_size == 1);
        assert(compare_spki_records(record, &result[0]));
        free(result);
        free(record);
    }

    //Delete concurrently the spki_records which we have added.
    struct remove_records_args remove_args[max_threads];
    for(unsigned int i = 0; i < max_threads; i++) {
        remove_args[i].table = &spkit;
        remove_args[i].start_asn = i * records_per_thread;
        remove_args[i].count = records_per_thread;
        pthread_create(&(threads[i]), NULL, (void * (*)(void *)) remove_records, &remove_args[i]);
    }

    //Wait for remove operation to finish
    for(unsigned int i=0; i<max_threads; i++) {
        pthread_join(threads[i], NULL);
        printf("Thread %i returned\n", i);
    }

    spki_table_free(&spkit);
    printf("lock_test1() complete\n");
}

int main()
{
    lock_test1();


}
