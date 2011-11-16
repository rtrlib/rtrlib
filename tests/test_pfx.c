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
#include "rtrlib/lib/utils.h"
#include "rtrlib/pfx/lpfst/lpfst-pfx.h"
void print_bytes(void* buf, size_t len){
    for(unsigned int i = 0;i < len; i++){
        if(len != 0)
            printf(":");
        printf("%02x", *((uint8_t*) ((char*) buf + i)));
    }
    printf("\n");
}

void print_state(const pfxv_state s){
    if(s == BGP_PFXV_STATE_VALID)
        printf("VALID\n");
    else if(s == BGP_PFXV_STATE_NOT_FOUND)
        printf("NOT FOUND\n");
    else if(s == BGP_PFXV_STATE_INVALID)
        printf("INVALID\n");
}

void remove_src_test(){
    pfx_table pfxt;
    pfx_table_init(&pfxt, NULL);

    pfx_record pfx;
    pfx.min_len = 32;
    pfx.max_len = 32;

    pfx.asn = 80;
    pfx.socket_id = 1;
    ip_str_to_addr("10.11.10.0", &(pfx.prefix), IPV4);
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

    pfx.asn = 90;
    pfx.socket_id = 2;
    ip_str_to_addr("10.11.10.0", &(pfx.prefix), IPV4);
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

    pfx.socket_id = 2;
    pfx.min_len = 24;
    ip_str_to_addr("192.168.0.0", &(pfx.prefix), IPV4);
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

    pfx.socket_id = 1;
    pfx.min_len = 8;
    ip_str_to_addr("10.0.0.0", &(pfx.prefix), IPV4);
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

    unsigned int len = 0;
    lpfst_node** array = NULL;
    assert(lpfst_get_children(pfxt.ipv4, &array, &len) != -1);
    free(array);
    array = NULL;
    assert((len + 1) == 3);

    pfx_table_src_remove(&pfxt, 1);
    len=0;
    assert(lpfst_get_children(pfxt.ipv4, &array, &len) != -1);

    free(array);
    assert((len + 1) == 2);

    pfxv_state res;
    assert(pfx_table_validate(&pfxt, 90, &(pfx.prefix), 8, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_NOT_FOUND);
    ip_str_to_addr("10.11.10.0", &(pfx.prefix), IPV4);

    assert(pfx_table_validate(&pfxt, 90, &(pfx.prefix), 32, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);
    assert(pfx_table_validate(&pfxt, 80, &(pfx.prefix), 32, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_INVALID);

    printf("remove_src_test successfull\n");

    pfx_table_free(&pfxt);
}

void mass_test(){
    pfx_table pfxt;
    pfx_table_init(&pfxt, NULL);

    pfx_record rec;
    pfxv_state res;
    const uint32_t min_i = 0xFFFF0000;
    const uint32_t max_i = 0xFFFFFFF0;

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
        assert(pfx_table_validate(&pfxt, i, &(rec.prefix), rec.min_len, &res) == PFX_SUCCESS);
        assert(res == BGP_PFXV_STATE_VALID);
        assert(pfx_table_validate(&pfxt, i + 1, &(rec.prefix), rec.min_len, &res) == PFX_SUCCESS);
        assert(res == BGP_PFXV_STATE_VALID);


        rec.min_len = 128;
        rec.max_len = 128;
        rec.prefix.ver = IPV6;
        rec.prefix.u.addr6.addr[1] = min_i + 0xFFFFFFFF;
        rec.prefix.u.addr6.addr[0] = htonl(i) + 0xFFFFFFFF;

        assert(pfx_table_validate(&pfxt, i + 1, &(rec.prefix), rec.min_len, &res) == PFX_SUCCESS);
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
    pfx_table_init(&pfxt, NULL);

    pfx_record pfx;
    pfx.asn = 123;
    pfx.prefix.ver = IPV4;
    ip_str_to_addr("10.10.0.0", &(pfx.prefix), IPV4);
    pfx.min_len = 16;
    pfx.max_len = 24;

    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

    pfxv_state res;
    assert(pfx_table_validate(&pfxt, 123, &(pfx.prefix), 16, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);
    assert(pfx_table_validate(&pfxt, 124, &(pfx.prefix), 16, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_INVALID);

    assert(pfx_table_validate(&pfxt, 123, &(pfx.prefix), 24, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);

    ip_str_to_addr("10.10.10.0", &(pfx.prefix), IPV4);
    assert(pfx_table_validate(&pfxt, 123, &(pfx.prefix), 20, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);

    assert(pfx_table_validate(&pfxt, 123, &(pfx.prefix), 25, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_INVALID);

    ip_str_to_addr("10.11.10.0", &(pfx.prefix), IPV4);
    assert(pfx_table_validate(&pfxt, 123, &(pfx.prefix), 16, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_NOT_FOUND);



    ip_str_to_addr("2a01:4f8:131::", &(pfx.prefix), IPV6);
    pfx.prefix.ver = IPV6;
    pfx.min_len = 48;
    pfx.max_len = 48;
    pfx.asn = 124;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(pfx_table_validate(&pfxt, 124, &(pfx.prefix), 48, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);

    ip_str_to_addr("2a01:4f8:131:15::", &(pfx.prefix), IPV6);
    assert(pfx_table_validate(&pfxt, 124, &(pfx.prefix), 56, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_INVALID);


    assert(ip_str_to_addr("1.0.4.0", &(pfx.prefix), IPV4) == 0);
    pfx.min_len=22;
    pfx.max_len=22;
    pfx.asn=56203;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(pfx_table_validate(&pfxt, pfx.asn, &(pfx.prefix), pfx.min_len, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);

    assert(ip_str_to_addr("1.8.1.0", &(pfx.prefix), IPV4) == 0);
    pfx.min_len=24;
    pfx.max_len=24;
    pfx.asn=38345;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(pfx_table_validate(&pfxt, pfx.asn, &(pfx.prefix), pfx.min_len, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);

    assert(ip_str_to_addr("1.8.8.0", &(pfx.prefix), IPV4) == 0);
    pfx.min_len=24;
    pfx.max_len=24;
    pfx.asn=38345;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(pfx_table_validate(&pfxt, pfx.asn, &(pfx.prefix), pfx.min_len, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);
    pfx_table_free(&pfxt);

    assert(ip_str_to_addr("1.0.65.0", &(pfx.prefix), IPV4) == 0);
    pfx.min_len=18;
    pfx.max_len=18;
    pfx.asn=18144;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(pfx_table_validate(&pfxt, pfx.asn, &(pfx.prefix), pfx.min_len, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);
    pfx_table_free(&pfxt);

    assert(ip_str_to_addr("10.0.0.0", &(pfx.prefix), IPV4) == 0);
    pfx.min_len=16;
    pfx.max_len=16;
    pfx.asn=123;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(pfx_table_validate(&pfxt, pfx.asn, &(pfx.prefix), pfx.min_len, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);

    assert(ip_str_to_addr("10.0.5.0", &(pfx.prefix), IPV4) == 0);
    pfx.min_len=24;
    pfx.max_len=24;
    pfx.asn=124;
    assert(pfx_table_validate(&pfxt, pfx.asn, &(pfx.prefix), pfx.min_len, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_INVALID);

    pfx_table_free(&pfxt);
    assert(ip_str_to_addr("109.105.96.0", &(pfx.prefix), IPV4) == 0);
    //memcpy(&(pfx.prefix.u.addr4.addr),  &i, sizeof(i));
    pfx.min_len=19;
    pfx.max_len=19;
    pfx.asn=123;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(ip_str_to_addr("109.105.128.0", &(pfx.prefix), IPV4) == 0);
    assert(pfx_table_validate(&pfxt, 456, &(pfx.prefix), 20, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_NOT_FOUND);

    pfx_table_free(&pfxt);
    assert(ip_str_to_addr("190.57.224.0", &(pfx.prefix), IPV4) == 0);
    pfx.min_len=19;
    pfx.max_len=24;
    pfx.asn=123;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(ip_str_to_addr("190.57.72.0", &(pfx.prefix), IPV4) == 0);
    assert(pfx_table_validate(&pfxt, 123, &(pfx.prefix), 21, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_NOT_FOUND);


    pfx_table_free(&pfxt);

    assert(ip_str_to_addr("80.253.128.0", &(pfx.prefix), IPV4) == 0);
    pfx.min_len=19;
    pfx.max_len=19;
    pfx.asn=123;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(ip_str_to_addr("80.253.144.0", &(pfx.prefix), IPV4) == 0);
    assert(pfx_table_validate(&pfxt, 123, &(pfx.prefix), 20, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_INVALID);

    char tmp[512];
    ipv6_addr_to_str(&(pfx.prefix.u.addr6), tmp, sizeof(tmp));

    pfx_table_free(&pfxt);
    remove_src_test();
    mass_test();
}

