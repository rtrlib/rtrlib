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
 * INET group, Hamburg University of Applied Sciences,
 * CST group, Freie Universitaet Berlin
 * Website: http://rpki.realmv6.org/
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include "rtrlib/lib/ip.h"
#include "rtrlib/pfx/lpfst/lpfst-pfx.h"

uint32_t min_i = 0xFF000000;
uint32_t max_i = 0xFFFFFFF0;

static void rec_insert(struct pfx_table* pfxt)
{
    const int tid = getpid();
    struct pfx_record rec;
    rec.min_len = 32;
    rec.max_len = 32;
    rec.prefix.ver = LRTR_IPV4;
    rec.prefix.u.addr4.addr = 0;


    printf("Inserting %u records\n", (max_i - min_i) * 3);
    for(uint32_t i = max_i; i >= min_i; i--) {

        rec.min_len = 32;
        rec.max_len = 32;
        rec.socket = NULL;
        rec.asn = tid % 2;
        rec.prefix.u.addr4.addr = htonl(i);
        rec.prefix.ver = LRTR_IPV4;
        pfx_table_add(pfxt, &rec);
        //printf("%i: Record inserted\n", tid);
        rec.asn = (tid % 2) + 1;
        pfx_table_add(pfxt, &rec);
        //printf("%i: Record inserted\n", tid);

        rec.min_len = 128;
        rec.max_len = 128;
        rec.prefix.ver = LRTR_IPV6;
        rec.prefix.u.addr6.addr[1] = min_i + 0xFFFFFFFF;
        rec.prefix.u.addr6.addr[0] = htonl(i) + 0xFFFFFFFF;
        pfx_table_add(pfxt, &rec);
        usleep(rand() / (RAND_MAX / 20));
        //printf("%i: Record inserted\n", tid);
    }
}
static void rec_validate(struct pfx_table* pfxt)
{
    const int tid = getpid();
    struct pfx_record rec;
    rec.min_len = 32;
    rec.max_len = 32;
    rec.prefix.ver = LRTR_IPV4;
    rec.prefix.u.addr4.addr = 0;
    enum pfxv_state res;
    printf("validating..\n");
    for(uint32_t i = max_i; i >= min_i; i--) {
        rec.min_len = 32;
        rec.max_len = 32;
        rec.prefix.ver = LRTR_IPV4;
        rec.prefix.u.addr4.addr = htonl(i);
        pfx_table_validate(pfxt, (tid % 2), &(rec.prefix), rec.min_len, &res);
        //printf("%i: Record validated,status: ", tid);
        //print_state(res);

        pfx_table_validate(pfxt, (tid % 2) + 1, &(rec.prefix), rec.min_len, &res);
        //printf("%i: Record validated,status: ", tid);
        //print_state(res);

        rec.min_len = 128;
        rec.max_len = 128;
        rec.prefix.ver = LRTR_IPV6;
        rec.prefix.u.addr6.addr[1] = min_i + 0xFFFFFFFF;
        rec.prefix.u.addr6.addr[0] = htonl(i) + 0xFFFFFFFF;

        pfx_table_validate(pfxt, (tid % 2) + 1, &(rec.prefix), rec.min_len, &res);
        //printf("%i: Record validated,status: ", tid);
        //print_state(res);
        usleep(rand() / (RAND_MAX / 20));
    }
}
static void rec_remove(struct pfx_table* pfxt)
{
    const int tid = getpid();
    struct pfx_record rec;
    rec.min_len = 32;
    rec.max_len = 32;
    rec.prefix.ver = LRTR_IPV4;
    rec.prefix.u.addr4.addr = 0;
    printf("removing records\n");
    for(uint32_t i = max_i; i >= min_i; i--) {
        rec.socket = NULL;
        rec.min_len = 32;
        rec.max_len = 32;
        rec.asn = tid %2;
        rec.prefix.ver = LRTR_IPV4;
        rec.prefix.u.addr4.addr = htonl(i);
        pfx_table_remove(pfxt, &rec);
        //printf("%i: Record removed, rtval: ", tid);
        //print_pfx_rtval(rtval);

        rec.asn = (tid % 2) + 1;
        pfx_table_remove(pfxt, &rec);
        //printf("%i: Record removed, rtval: ", tid);
        //print_pfx_rtval(rtval);

        rec.prefix.ver = LRTR_IPV6;
        rec.min_len = 128;
        rec.max_len = 128;
        rec.prefix.u.addr6.addr[1] = min_i + 0xFFFFFFFF;
        rec.prefix.u.addr6.addr[0] = htonl(i) + 0xFFFFFFFF;
        pfx_table_remove(pfxt, &rec);
        //printf("%i: Record removed, rtval: ", tid);
        //print_pfx_rtval(rtval);
        usleep(rand() / (RAND_MAX / 20));
    }
    printf("Done\n");
}

int main()
{
    unsigned int max_threads = 15;
    struct pfx_table pfxt;
    pfx_table_init(&pfxt, NULL);
    pthread_t threads[max_threads];
    srand(time(NULL));
    for(unsigned int i=0; i<max_threads; i++) {
        int r = rand() / (RAND_MAX / 3);
        if(r == 0)
            pthread_create(&(threads[i]), NULL, (void * (*)(void *)) rec_insert, &pfxt);
        else if(r == 1)
            pthread_create(&(threads[i]), NULL, (void * (*)(void *)) rec_remove, &pfxt);
        else if(r == 2)
            pthread_create(&(threads[i]), NULL, (void * (*)(void *)) rec_validate, &pfxt);
        printf("Started Thread %d\n",i);
        usleep(200);

    }
    for(unsigned int i=0; i<max_threads; i++) {
        pthread_join(threads[i], NULL);
        printf("Thread %i returned\n", i);
    }
}
