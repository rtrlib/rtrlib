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
#include <sys/types.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include "rtrlib/lib/ip.h"
#include "rtrlib/lib/utils.h"
#include "rtrlib/pfx/lpfst/lpfst-pfx.h"

static void remove_src_test()
{
    struct pfx_table pfxt;
    pfx_table_init(&pfxt, NULL);
    struct rtr_socket tr1;

    struct pfx_record pfx;
    pfx.min_len = 32;
    pfx.max_len = 32;

    pfx.asn = 80;
    pfx.socket = &tr1;
    lrtr_ip_str_to_addr("10.11.10.0", &(pfx.prefix));
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

    pfx.asn = 90;
    pfx.socket = NULL;
    lrtr_ip_str_to_addr("10.11.10.0", &(pfx.prefix));
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

    pfx.socket = NULL;
    pfx.min_len = 24;
    lrtr_ip_str_to_addr("192.168.0.0", &(pfx.prefix));
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

    pfx.socket = &tr1;
    pfx.min_len = 8;
    lrtr_ip_str_to_addr("10.0.0.0", &(pfx.prefix));
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

    unsigned int len = 0;
    struct lpfst_node** array = NULL;
    assert(lpfst_get_children(pfxt.ipv4, &array, &len) != -1);
    free(array);
    array = NULL;
    assert((len + 1) == 3);

    pfx_table_src_remove(&pfxt, &tr1);
    len=0;
    assert(lpfst_get_children(pfxt.ipv4, &array, &len) != -1);

    free(array);
    assert((len + 1) == 2);

    enum pfxv_state res;
    assert(pfx_table_validate(&pfxt, 90, &(pfx.prefix), 8, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_NOT_FOUND);
    lrtr_ip_str_to_addr("10.11.10.0", &(pfx.prefix));

    assert(pfx_table_validate(&pfxt, 90, &(pfx.prefix), 32, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);
    assert(pfx_table_validate(&pfxt, 80, &(pfx.prefix), 32, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_INVALID);

    printf("remove_src_test successfull\n");

    pfx_table_free(&pfxt);
}

static void mass_test()
{
    struct pfx_table pfxt;
    pfx_table_init(&pfxt, NULL);

    struct pfx_record rec;
    enum pfxv_state res;
    const uint32_t min_i = 0xFFFF0000;
    const uint32_t max_i = 0xFFFFFFF0;

    printf("Inserting %u records\n", (max_i - min_i) * 3);
    for(uint32_t i = max_i; i >= min_i; i--) {
        rec.min_len = 32;
        rec.max_len = 32;
        rec.socket = NULL;
        rec.asn = i;
        rec.prefix.u.addr4.addr = htonl(i);
        rec.prefix.ver = LRTR_IPV4;
        assert(pfx_table_add(&pfxt, &rec) == PFX_SUCCESS);
        rec.asn = i + 1;
        assert(pfx_table_add(&pfxt, &rec) == PFX_SUCCESS);

        rec.min_len = 128;
        rec.max_len = 128;
        rec.prefix.ver = LRTR_IPV6;
        ((uint64_t*) rec.prefix.u.addr6.addr)[1] = max_i;
        ((uint64_t*) rec.prefix.u.addr6.addr)[0] = min_i+i;
        assert(pfx_table_add(&pfxt, &rec) == PFX_SUCCESS);
    }

    printf("validating..\n");
    for(uint32_t i = max_i; i >= min_i; i--) {
        rec.min_len = 32;
        rec.max_len = 32;
        rec.prefix.ver = LRTR_IPV4;
        rec.prefix.u.addr4.addr = htonl(i);
        assert(pfx_table_validate(&pfxt, i, &(rec.prefix), rec.min_len, &res) == PFX_SUCCESS);
        assert(res == BGP_PFXV_STATE_VALID);
        assert(pfx_table_validate(&pfxt, i + 1, &(rec.prefix), rec.min_len, &res) == PFX_SUCCESS);
        assert(res == BGP_PFXV_STATE_VALID);


        rec.min_len = 128;
        rec.max_len = 128;
        rec.prefix.ver = LRTR_IPV6;
        ((uint64_t*) rec.prefix.u.addr6.addr)[1] = max_i;
        ((uint64_t*) rec.prefix.u.addr6.addr)[0] = min_i + i;

        assert(pfx_table_validate(&pfxt, i + 1, &(rec.prefix), rec.min_len, &res) == PFX_SUCCESS);
        assert(res == BGP_PFXV_STATE_VALID);
    }

    printf("removing records\n");
    for(uint32_t i = max_i; i >= min_i; i--) {
        rec.socket = NULL;
        rec.min_len = 32;
        rec.max_len = 32;
        rec.asn = i;
        rec.prefix.ver = LRTR_IPV4;
        rec.prefix.u.addr4.addr = htonl(i);
        assert(pfx_table_remove(&pfxt, &rec) == PFX_SUCCESS);

        rec.asn = i + 1;
        assert(pfx_table_remove(&pfxt, &rec) == PFX_SUCCESS);

        rec.prefix.ver = LRTR_IPV6;
        rec.min_len = 128;
        rec.max_len = 128;
        ((uint64_t*) rec.prefix.u.addr6.addr)[1] = max_i;
        ((uint64_t*) rec.prefix.u.addr6.addr)[0] = min_i + i;
        assert(pfx_table_remove(&pfxt, &rec) == PFX_SUCCESS);
    }
    pfx_table_free(&pfxt);
    printf("Done\n");
}

int main()
{
    struct pfx_table pfxt;
    pfx_table_init(&pfxt, NULL);

    struct pfx_record pfx;
    pfx.asn = 123;
    pfx.prefix.ver = LRTR_IPV4;
    lrtr_ip_str_to_addr("10.10.0.0", &(pfx.prefix));
    pfx.min_len = 16;
    pfx.max_len = 24;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

    enum pfxv_state res;
    assert(pfx_table_validate(&pfxt, 123, &(pfx.prefix), 16, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);
    assert(pfx_table_validate(&pfxt, 124, &(pfx.prefix), 16, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_INVALID);

    assert(pfx_table_validate(&pfxt, 123, &(pfx.prefix), 24, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);

    lrtr_ip_str_to_addr("10.10.10.0", &(pfx.prefix));
    assert(pfx_table_validate(&pfxt, 123, &(pfx.prefix), 20, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);

    assert(pfx_table_validate(&pfxt, 123, &(pfx.prefix), 25, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_INVALID);

    lrtr_ip_str_to_addr("10.11.10.0", &(pfx.prefix));
    assert(pfx_table_validate(&pfxt, 123, &(pfx.prefix), 16, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_NOT_FOUND);

    lrtr_ip_str_to_addr("10.10.0.0", &(pfx.prefix));
    pfx.asn = 122;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

    assert(pfx_table_validate(&pfxt, 122, &(pfx.prefix), 18, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);

    lrtr_ip_str_to_addr("11.10.0.0", &(pfx.prefix));
    pfx.asn = 22;
    pfx.min_len = 17;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(pfx_table_validate(&pfxt, 22, &(pfx.prefix), 17, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);



    lrtr_ip_str_to_addr("2a01:4f8:131::", &(pfx.prefix));
    pfx.prefix.ver = LRTR_IPV6;
    pfx.min_len = 48;
    pfx.max_len = 48;
    pfx.asn = 124;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(pfx_table_validate(&pfxt, 124, &(pfx.prefix), 48, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);

    lrtr_ip_str_to_addr("2a01:4f8:131:15::", &(pfx.prefix));
    assert(pfx_table_validate(&pfxt, 124, &(pfx.prefix), 56, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_INVALID);


    assert(lrtr_ip_str_to_addr("1.0.4.0", &(pfx.prefix)) == 0);
    pfx.min_len=22;
    pfx.max_len=22;
    pfx.asn=56203;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(pfx_table_validate(&pfxt, pfx.asn, &(pfx.prefix), pfx.min_len, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);

    assert(lrtr_ip_str_to_addr("1.8.1.0", &(pfx.prefix)) == 0);
    pfx.min_len=24;
    pfx.max_len=24;
    pfx.asn=38345;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(pfx_table_validate(&pfxt, pfx.asn, &(pfx.prefix), pfx.min_len, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);

    assert(lrtr_ip_str_to_addr("1.8.8.0", &(pfx.prefix)) == 0);
    pfx.min_len=24;
    pfx.max_len=24;
    pfx.asn=38345;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(pfx_table_validate(&pfxt, pfx.asn, &(pfx.prefix), pfx.min_len, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);
    pfx_table_free(&pfxt);

    assert(lrtr_ip_str_to_addr("1.0.65.0", &(pfx.prefix)) == 0);
    pfx.min_len=18;
    pfx.max_len=18;
    pfx.asn=18144;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(pfx_table_validate(&pfxt, pfx.asn, &(pfx.prefix), pfx.min_len, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);
    pfx_table_free(&pfxt);

    assert(lrtr_ip_str_to_addr("10.0.0.0", &(pfx.prefix)) == 0);
    pfx.min_len=16;
    pfx.max_len=16;
    pfx.asn=123;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(pfx_table_validate(&pfxt, pfx.asn, &(pfx.prefix), pfx.min_len, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_VALID);

    assert(lrtr_ip_str_to_addr("10.0.5.0", &(pfx.prefix)) == 0);
    pfx.min_len=24;
    pfx.max_len=24;
    pfx.asn=124;
    assert(pfx_table_validate(&pfxt, pfx.asn, &(pfx.prefix), pfx.min_len, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_INVALID);

    pfx_table_free(&pfxt);
    assert(lrtr_ip_str_to_addr("109.105.96.0", &(pfx.prefix)) == 0);
    //memcpy(&(pfx.prefix.u.addr4.addr),  &i, sizeof(i));
    pfx.min_len=19;
    pfx.max_len=19;
    pfx.asn=123;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(lrtr_ip_str_to_addr("109.105.128.0", &(pfx.prefix)) == 0);
    assert(pfx_table_validate(&pfxt, 456, &(pfx.prefix), 20, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_NOT_FOUND);

    pfx_table_free(&pfxt);
    assert(lrtr_ip_str_to_addr("190.57.224.0", &(pfx.prefix)) == 0);
    pfx.min_len=19;
    pfx.max_len=24;
    pfx.asn=123;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(lrtr_ip_str_to_addr("190.57.72.0", &(pfx.prefix)) == 0);
    assert(pfx_table_validate(&pfxt, 123, &(pfx.prefix), 21, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_NOT_FOUND);


    pfx_table_free(&pfxt);

    assert(lrtr_ip_str_to_addr("80.253.128.0", &(pfx.prefix)) == 0);
    pfx.min_len=19;
    pfx.max_len=19;
    pfx.asn=123;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(lrtr_ip_str_to_addr("80.253.144.0", &(pfx.prefix)) == 0);
    assert(pfx_table_validate(&pfxt, 123, &(pfx.prefix), 20, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_INVALID);

    assert(lrtr_ip_str_to_addr("10.10.0.0", &(pfx.prefix)) == 0);
    pfx.min_len=16;
    pfx.max_len=16;
    pfx.asn=0;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);
    assert(pfx_table_validate(&pfxt, 123, &(pfx.prefix), 16, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_INVALID);

    assert(lrtr_ip_str_to_addr("10.0.0.0", &(pfx.prefix)) == 0);
    pfx.min_len=8;
    pfx.max_len=15;
    pfx.asn=6;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

    assert(lrtr_ip_str_to_addr("10.0.0.0", &(pfx.prefix)) == 0);
    pfx.min_len=8;
    pfx.max_len=15;
    pfx.asn=5;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

    assert(lrtr_ip_str_to_addr("10.1.0.0", &(pfx.prefix)) == 0);
    pfx.min_len=16;
    pfx.max_len=16;
    pfx.asn=5;
    assert(pfx_table_add(&pfxt, &pfx) == PFX_SUCCESS);

    struct pfx_record* r = NULL;
    unsigned int r_len = 0;
    assert(lrtr_ip_str_to_addr("10.1.0.0", &(pfx.prefix)) == 0);
    assert(pfx_table_validate_r(&pfxt, &r, &r_len, 123, &(pfx.prefix), 16, &res) == PFX_SUCCESS);
    assert(res == BGP_PFXV_STATE_INVALID);
    assert(r_len == 3);
    free(r);


    pfx_table_free(&pfxt);
    remove_src_test();
    mass_test();
}

