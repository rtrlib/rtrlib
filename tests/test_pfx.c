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
 * written by Fabian Holler, in cooperation with:
 * INET group, Hamburg University of Applied Sciences,
 * CST group, Freie Universitaet Berlin
 * Website: http://rpki.realmv6.org/
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include "rtrlib/lib/ip.h"
#include "rtrlib/pfx/lpfst/lpfst-pfx.h"
#include "rtrlib/lib/test.h"

void print_state(const pfxv_state s){
    if(s == BGP_PFXV_STATE_VALID)
        printf("VALID\n");
    else if(s == BGP_PFXV_STATE_NOT_FOUND)
        printf("NOT FOUND\n");
    else if(s == BGP_PFXV_STATE_INVALID)
        printf("INVALID\n");
}

void mass_test(){
    pfx_table pfxt;
    pfx_table_init(&pfxt, NULL, 0);

    pfx_record rec;
    rec.min_len = 32;
    rec.max_len = 32;
    rec.prefix.ver = IPV4;
    rec.prefix.u.addr4.addr = 0;
    pfxv_state res;


    uint32_t min_i = 0xFFFF0000;
    uint32_t max_i = 0xFFFFFFF0;

    printf("Inserting %u records\n", (max_i - min_i) * 3);
    for(uint32_t i = max_i; i >= min_i; i--){

        rec.min_len = 32;
        rec.max_len = 32;
        rec.socket_id = i;
        rec.asn = i;
        rec.prefix.u.addr4.addr = htonl(i);
        rec.prefix.ver = IPV4;
        assert(pfx_table_add(&pfxt, &rec) == PFX_SUCCESS);
        rec.asn = i + 1;
        assert(pfx_table_add(&pfxt, &rec) == PFX_SUCCESS);

        rec.min_len = 128;
        rec.max_len = 128;
        rec.prefix.ver = IPV6;
        rec.prefix.u.addr6.addr[1] = min_i + 0xFFFFFFFF;
        rec.prefix.u.addr6.addr[0] = htonl(i) + 0xFFFFFFFF;
        assert(pfx_table_add(&pfxt, &rec) == PFX_SUCCESS);
    }

    printf("validating..\n");
    for(uint32_t i = max_i; i >= min_i; i--){
        rec.min_len = 32;
        rec.max_len = 32;
        rec.prefix.ver = IPV4;
        rec.prefix.u.addr4.addr = htonl(i);
        assert(pfx_validate_origin(&pfxt, i, &(rec.prefix), rec.min_len, &res) == PFX_SUCCESS);
        assert(res == BGP_PFXV_STATE_VALID);
        assert(pfx_validate_origin(&pfxt, i + 1, &(rec.prefix), rec.min_len, &res) == PFX_SUCCESS);
        assert(res == BGP_PFXV_STATE_VALID);


        rec.min_len = 128;
        rec.max_len = 128;
        rec.prefix.ver = IPV6;
        rec.prefix.u.addr6.addr[1] = min_i + 0xFFFFFFFF;
        rec.prefix.u.addr6.addr[0] = htonl(i) + 0xFFFFFFFF;

        assert(pfx_validate_origin(&pfxt, i + 1, &(rec.prefix), rec.min_len, &res) == PFX_SUCCESS);
        assert(res == BGP_PFXV_STATE_VALID);
    }

    printf("removing records\n");
    for(uint32_t i = max_i; i >= min_i; i--){
        rec.socket_id = i;
        rec.min_len = 32;
        rec.max_len = 32;
        rec.asn = i;
        rec.prefix.ver = IPV4;
        rec.prefix.u.addr4.addr = htonl(i);
        assert(pfx_table_remove(&pfxt, &rec) == PFX_SUCCESS);

        rec.asn = i + 1;
        assert(pfx_table_remove(&pfxt, &rec) == PFX_SUCCESS);

        rec.prefix.ver = IPV6;
        rec.min_len = 128;
        rec.max_len = 128;
        rec.prefix.u.addr6.addr[1] = min_i + 0xFFFFFFFF;
        rec.prefix.u.addr6.addr[0] = htonl(i) + 0xFFFFFFFF;
        assert(pfx_table_remove(&pfxt, &rec) == PFX_SUCCESS);
    }
    pfx_table_free(&pfxt);
    printf("Done\n");
}

int main(){
    pfx_table pfxt;
    pfx_table_init(&pfxt, NULL, 0);

    pfx_record pfx;
    pfx.asn = 123;
    pfx.prefix.ver = IPV4;
    put_ipv4(&(pfx.prefix), "10.10.0.0");
    pfx.min_len = 16;
    pfx.max_len = 24;

    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

    pfxv_state res;
    assert(pfx_validate_origin(&pfxt, 123, &(pfx.prefix), 16, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);
    assert(pfx_validate_origin(&pfxt, 124, &(pfx.prefix), 16, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_INVALID);

    assert(pfx_validate_origin(&pfxt, 123, &(pfx.prefix), 24, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);

    put_ipv4(&(pfx.prefix), "10.10.10.0");
    assert(pfx_validate_origin(&pfxt, 123, &(pfx.prefix), 20, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);

    assert(pfx_validate_origin(&pfxt, 123, &(pfx.prefix), 25, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_INVALID);

    put_ipv4(&(pfx.prefix), "10.11.10.0");
    assert(pfx_validate_origin(&pfxt, 123, &(pfx.prefix), 16, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_NOT_FOUND);



    put_ipv6(&(pfx.prefix), "2a01:4f8:131::");
    pfx.prefix.ver = IPV6;
    pfx.min_len = 48;
    pfx.max_len = 48;
    pfx.asn = 124;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(pfx_validate_origin(&pfxt, 124, &(pfx.prefix), 48, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);

    put_ipv6(&(pfx.prefix), "2a01:4f8:131:15::");
    assert(pfx_validate_origin(&pfxt, 124, &(pfx.prefix), 56, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_INVALID);

    pfx_table_free(&pfxt);
    mass_test();
}
