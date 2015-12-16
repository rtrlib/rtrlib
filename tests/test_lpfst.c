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
#include <arpa/inet.h>
#include "rtrlib/lib/ipv6.h"
#include "rtrlib/lib/log.h"
#include "rtrlib/lib/utils.h"
#include "rtrlib/pfx/lpfst/lpfst.c"


static void get_bits_testv4()
{
    struct lrtr_ip_addr addr;
    addr.ver = LRTR_IPV4;

    struct lrtr_ip_addr result;
    addr.u.addr4.addr=0xAABBCC22;

    result = lrtr_ip_addr_get_bits(&addr, 0, 32);
    assert(result.u.addr4.addr == 0xAABBCC22);

    result = lrtr_ip_addr_get_bits(&addr, 0, 1);
    assert(result.u.addr4.addr == 0x80000000);

    result = lrtr_ip_addr_get_bits(&addr, 1, 1);
    assert(result.u.addr4.addr == 0);

    result = lrtr_ip_addr_get_bits(&addr, 2, 1);
    assert(result.u.addr4.addr == 0x20000000);

    result = lrtr_ip_addr_get_bits(&addr, 0, 8);
    assert(result.u.addr4.addr == 0xAA000000);

    result = lrtr_ip_addr_get_bits(&addr, 8, 8);
    assert(result.u.addr4.addr == 0x00BB0000);


    lrtr_ip_str_to_addr("10.10.10.0", &addr);

    result = lrtr_ip_addr_get_bits(&addr, 0, 8);
    assert(lrtr_ip_str_cmp(&result, "10.0.0.0"));

    result = lrtr_ip_addr_get_bits(&addr, 0, 16);
    assert(lrtr_ip_str_cmp(&result, "10.10.0.0"));

    result = lrtr_ip_addr_get_bits(&addr, 8, 8);
    assert(lrtr_ip_str_cmp(&result, "0.10.0.0"));

    result = lrtr_ip_addr_get_bits(&addr, 8, 24);
    assert(lrtr_ip_str_cmp(&result, "0.10.10.0"));

    result = lrtr_ip_addr_get_bits(&addr, 31, 1);
    assert(result.u.addr4.addr == 0);

    result = lrtr_ip_addr_get_bits(&addr, 0, 1);
    assert(result.u.addr4.addr == 0);

    result = lrtr_ip_addr_get_bits(&addr, 3, 3);
    assert(lrtr_ip_str_cmp(&result, "8.0.0.0"));

    assert(lrtr_ip_str_to_addr("132.200.0.0", &addr) == 0);
    result = lrtr_ip_addr_get_bits(&addr, 0, 1);
    assert(result.u.addr4.addr == 0x80000000);

    assert(lrtr_ip_str_to_addr("101.200.0.0", &addr) == 0);
    result = lrtr_ip_addr_get_bits(&addr, 0, 1);
    assert(result.u.addr4.addr == 0);

    addr.u.addr4.addr = 0x6D698000;
    result = lrtr_ip_addr_get_bits(&addr, 0, 19);
    assert(result.u.addr4.addr == 0x6D698000);
    /*
    ip_str_to_addr("109.105.128.0", &addr);
    result = ip_addr_get_bits(&addr, 0, 8);
    printf("%u\n", result.u.addr4.addr);
    */

    char buf[INET_ADDRSTRLEN];
    assert(lrtr_ip_str_to_addr("10.10.10.5", &addr) == 0);
    assert(lrtr_ip_addr_to_str(&addr, buf, sizeof(buf)) == 0);
    assert(strcmp("10.10.10.5", buf) == 0);

}

static void get_bits_testv6()
{
    struct lrtr_ip_addr addr;
    addr.ver = LRTR_IPV6;
    addr.u.addr6.addr[0] = 0x22AABBCC;
    addr.u.addr6.addr[1] = 0xDDEEFF99;
    addr.u.addr6.addr[2] = 0x33001122;
    addr.u.addr6.addr[3] = 0x33445566;

    struct lrtr_ip_addr result;

    result = lrtr_ip_addr_get_bits(&addr, 0, 128);
    assert(result.u.addr6.addr[0] == addr.u.addr6.addr[0] && result.u.addr6.addr[1] == addr.u.addr6.addr[1] && result.u.addr6.addr[2] == addr.u.addr6.addr[2] && result.u.addr6.addr[3] == addr.u.addr6.addr[3]);

    result = lrtr_ip_addr_get_bits(&addr, 0, 64);
    assert(result.u.addr6.addr[0] == addr.u.addr6.addr[0] && result.u.addr6.addr[1] == addr.u.addr6.addr[1] && result.u.addr6.addr[2] == 0 && result.u.addr6.addr[3] == 0);

    bzero(&result, sizeof(result));
    result = lrtr_ip_addr_get_bits(&addr, 64, 64);
    assert(result.u.addr6.addr[0] == 0);
    assert(result.u.addr6.addr[1] == 0);
    assert(result.u.addr6.addr[2] == addr.u.addr6.addr[2]);
    assert(result.u.addr6.addr[3] == addr.u.addr6.addr[3]);


    result = lrtr_ip_addr_get_bits(&addr, 0, 8);
    assert(result.u.addr6.addr[0] == 0x22000000 && result.u.addr6.addr[1] == 0);

    result = lrtr_ip_addr_get_bits(&addr, 64, 8);
    assert(result.u.addr6.addr[1] == 0 && result.u.addr6.addr[2] == 0x33000000);

    result = lrtr_ip_addr_get_bits(&addr, 7, 8);
    assert(result.u.addr6.addr[0] == 0xAA0000 && result.u.addr6.addr[1] == 0);

    result = lrtr_ip_addr_get_bits(&addr, 68, 7);
    assert(result.u.addr6.addr[0] == 0 && result.u.addr6.addr[2] == 0x03000000);

    char buf[INET6_ADDRSTRLEN];

    lrtr_ip_str_to_addr("fe80::862b:2bff:fe9a:f50f", &addr);
    addr.ver = LRTR_IPV6;
    assert(addr.u.addr6.addr[0] == 0xfe800000);
    assert(addr.u.addr6.addr[1] == 0);
    assert(addr.u.addr6.addr[2] == 0x862b2bff);
    assert(addr.u.addr6.addr[3] == 0xfe9af50f);

    assert(lrtr_ip_str_to_addr("2001::", &addr) == 0);
    assert(addr.u.addr6.addr[0] == 0x20010000);
    assert(addr.u.addr6.addr[1] == 0);
    assert(addr.u.addr6.addr[2] == 0);
    assert(addr.u.addr6.addr[3] == 0);

    assert(lrtr_ip_addr_to_str(&addr, buf, sizeof(buf)) == 0);
    assert(strcmp("2001::", buf) == 0);

    lrtr_ip_str_to_addr("2001:0db8:85a3:08d3:1319:8a2e:0370:7344", &addr);
    assert(lrtr_ip_addr_to_str(&addr, buf, sizeof(buf)) == 0);
    assert(strcmp("2001:db8:85a3:8d3:1319:8a2e:370:7344", buf) == 0);

    result = lrtr_ip_addr_get_bits(&addr, 0, 16);
    assert(lrtr_ip_addr_to_str(&result, buf, sizeof(buf)) == 0);
    assert(lrtr_ip_str_cmp(&result, "2001::"));

    result = lrtr_ip_addr_get_bits(&addr, 16, 16);
    assert(lrtr_ip_addr_to_str(&result, buf, sizeof(buf)) == 0);
    assert(lrtr_ip_str_cmp(&result, "0:db8::"));
    result = lrtr_ip_addr_get_bits(&addr, 0, 1);
    assert(lrtr_ip_str_cmp(&result, "::"));

    result = lrtr_ip_addr_get_bits(&addr, 126, 1);
    assert(lrtr_ip_addr_to_str(&result, buf, sizeof(buf)) == 0);
    assert(lrtr_ip_str_cmp(&result, "::"));
}

static void lpfst_test()
{
    struct lrtr_ip_addr addr;
    addr.ver = LRTR_IPV4;
    struct lpfst_node* result;
    unsigned int lvl = 0;
    //n1
    struct lpfst_node n1;
    n1.len = 16;
    n1.lchild = NULL;
    n1.rchild = NULL;
    n1.parent = NULL;
    n1.data = NULL;
    lrtr_ip_str_to_addr( "100.200.0.0", &(n1.prefix));

    lrtr_ip_str_to_addr( "100.200.0.0", &(addr));
    lvl = 0;
    result = lpfst_lookup(&n1, &addr, 16, &lvl);
    assert(result != NULL);
    assert(lrtr_ip_str_cmp(&(result->prefix), "100.200.0.0"));

    lrtr_ip_str_to_addr( "100.200.30.0", &(addr));
    lvl = 0;
    result = lpfst_lookup(&n1, &addr, 16, &lvl);
    assert(result != NULL);
    assert(lrtr_ip_str_cmp(&(result->prefix), "100.200.0.0"));


    //n2
    struct lpfst_node n2;
    n2.len = 16;
    n2.lchild = NULL;
    n2.rchild = NULL;
    n2.parent = NULL;
    n2.data = NULL;
    lrtr_ip_str_to_addr("132.200.0.0", &(n2.prefix));
    lpfst_insert(&n1, &n2, 0);

    lrtr_ip_str_to_addr("132.200.0.0", &(addr));
    lvl = 0;
    result = lpfst_lookup(&n1, &addr, 16, &lvl);
    assert(result != NULL);
    assert(lrtr_ip_str_cmp(&(result->prefix), "132.200.0.0"));
    assert(n1.rchild == &n2);


    //n3
    struct lpfst_node n3;
    n3.len = 16;
    n3.lchild = NULL;
    n3.rchild = NULL;
    n3.parent = NULL;
    n3.data = NULL;
    lrtr_ip_str_to_addr("101.200.0.0", &(n3.prefix));
    lpfst_insert(&n1, &n3, 0);

    lrtr_ip_str_to_addr("101.200.0.0", &(addr));
    lvl = 0;
    result = lpfst_lookup(&n1, &addr, 16, &lvl);
    assert(result != NULL);
    assert(lrtr_ip_str_cmp(&(result->prefix), "101.200.0.0"));
    assert(n1.lchild == &n3);

    //n4
    struct lpfst_node n4;
    n4.len = 24;
    n4.lchild = NULL;
    n4.rchild = NULL;
    n4.parent = NULL;
    n4.data = NULL;
    lrtr_ip_str_to_addr("132.200.3.0", &(n4.prefix));
    lpfst_insert(&n1, &n4, 0);

    lrtr_ip_str_to_addr("132.200.3.0", &(addr));
    lvl = 0;
    result = lpfst_lookup(&n1, &addr, 24, &lvl);
    assert(result != NULL);
    assert(lrtr_ip_str_cmp(&(result->prefix), "132.200.3.0"));
    assert(lrtr_ip_str_cmp(&(n1.prefix), "132.200.3.0"));
    assert(n1.len == 24);
    assert(lrtr_ip_str_cmp(&(n1.lchild->prefix), "101.200.0.0"));
    assert(n1.lchild->len == 16);
    assert(lrtr_ip_str_cmp(&(n1.rchild->prefix), "132.200.0.0"));
    assert(n1.rchild->len == 16);
    assert(lrtr_ip_str_cmp(&(n1.lchild->rchild->prefix), "100.200.0.0"));
    assert(n1.lchild->rchild->len == 16);

    lrtr_ip_str_to_addr("132.200.0.0", &(addr));
    lvl = 0;
    result = lpfst_lookup(&n1, &addr, 16, &lvl);
    assert(result != NULL);
    assert(lrtr_ip_str_cmp(&(result->prefix), "132.200.0.0"));

    lrtr_ip_str_to_addr("132.200.3.0", &(addr));
    lvl = 0;
    result = lpfst_lookup(&n1, &addr, 16, &lvl);
    assert(result != NULL);
    assert(lrtr_ip_str_cmp(&(result->prefix), "132.200.0.0"));


    lvl = 0;
    lrtr_ip_str_to_addr("132.0.0.0", &(addr));
    bool found;
    result = lpfst_lookup_exact(&n1, &addr, 16, &lvl, &found);
    assert(!found);

    lvl = 0;
    lrtr_ip_str_to_addr("132.200.3.0", &(addr));
    result = lpfst_lookup_exact(&n1, &addr, 24, &lvl, &found);
    assert(found);
    assert(lrtr_ip_str_cmp(&(result->prefix), "132.200.3.0"));

    result = lpfst_remove(&n1, &addr, 24, 0);
    assert(result != NULL);
    assert(lrtr_ip_str_cmp(&(n1.prefix), "132.200.0.0"));
    assert(lrtr_ip_str_cmp(&(n1.lchild->prefix), "101.200.0.0"));
    assert(lrtr_ip_str_cmp(&(n1.lchild->rchild->prefix), "100.200.0.0"));
    assert(n1.rchild == NULL);

    lrtr_ip_str_to_addr("101.200.0.0", &(addr));
    result = lpfst_remove(&n1, &addr, 16, 0);
    assert(result != NULL);
    assert(lrtr_ip_str_cmp(&(n1.lchild->prefix), "100.200.0.0"));
    assert(n1.rchild == NULL);

    lrtr_ip_str_to_addr("100.200.0.0", &(addr));
    result = lpfst_remove(&n1, &addr, 16, 0);
    assert(result != NULL);
    assert(n1.lchild == NULL);
    assert(n1.rchild == NULL);
}


int main()
{
    get_bits_testv4();
    get_bits_testv6();
    lpfst_test();
    printf("Test successfull\n");
}
