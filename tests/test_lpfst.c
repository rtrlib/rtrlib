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
 * Webpage: https://rpki.realmv6.org/
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>
#include <arpa/inet.h>
#include "rtrlib/lib/ipv6.h"
#include "rtrlib/lib/log.h"
#include "rtrlib/lib/test.h"
#include "rtrlib/pfx/lpfst/lpfst.c"


void get_bits_testv4(){
    ip_addr addr;
    addr.ver = IPV4;

    ip_addr result;
    addr.u.addr4.addr=0xAABBCC22;
    result = ip_addr_get_bits(&addr, 0, 0);
    assert(result.u.addr4.addr == 0);
    result = ip_addr_get_bits(&addr, 1, 1);
    assert(result.u.addr4.addr == 2);

    result = ip_addr_get_bits(&addr, 0, 7);
    assert(result.u.addr4.addr == 0x22);

    result = ip_addr_get_bits(&addr, 8, 15);
    assert(result.u.addr4.addr == 0xCC00);


    put_ipv4(&addr, "10.10.10.0");

    result = ip_addr_get_bits(&addr, 0, 8);
    assert(cmp_ipv4(&result, "10.0.0.0"));

    result = ip_addr_get_bits(&addr, 0, 15);
    assert(cmp_ipv4(&result, "10.10.0.0"));

    result = ip_addr_get_bits(&addr, 8, 15);
    assert(cmp_ipv4(&result, "0.10.0.0"));

    result = ip_addr_get_bits(&addr, 8, 23);
    assert(cmp_ipv4(&result, "0.10.10.0"));

    result = ip_addr_get_bits(&addr, 31, 31);
    assert(result.u.addr4.addr == 0);

    result = ip_addr_get_bits(&addr, 0, 0);
    assert(result.u.addr4.addr == 0);

    result = ip_addr_get_bits(&addr, 3, 3);
    assert(cmp_ipv4(&result, "8.0.0.0"));

}

void get_bits_testv6(){
    ip_addr addr;
    addr.ver = IPV6;
    addr.u.addr6.addr[0] = 0x22AABBCCDDEEFF99;
    addr.u.addr6.addr[1] = 0x3300112233445566;

    ip_addr result;
    result = ip_addr_get_bits(&addr, 0, 63);
    assert(result.u.addr6.addr[0] == addr.u.addr6.addr[0] && result.u.addr6.addr[1] == 0);

    result = ip_addr_get_bits(&addr, 64, 127);
    assert(result.u.addr6.addr[1] == addr.u.addr6.addr[1] && result.u.addr6.addr[0] == 0);

    result = ip_addr_get_bits(&addr, 0, 127);
    assert(result.u.addr6.addr[0] == addr.u.addr6.addr[0] && result.u.addr6.addr[1] == addr.u.addr6.addr[1]);


    result = ip_addr_get_bits(&addr, 0, 7);
    assert(result.u.addr6.addr[0] == 0x99 && result.u.addr6.addr[1] == 0);

    result = ip_addr_get_bits(&addr, 64, 71);
    assert(result.u.addr6.addr[0] == 0 && result.u.addr6.addr[1] == 0x66);

    result = ip_addr_get_bits(&addr, 8, 15);
    assert(result.u.addr6.addr[0] == 0xFF00 && result.u.addr6.addr[1] == 0);

    result = ip_addr_get_bits(&addr, 68, 75);
    assert(result.u.addr6.addr[0] == 0 && result.u.addr6.addr[1] == 0x560);

    put_ipv6(&addr, "2001:0db8:85a3:08d3:1319:8a2e:0370:7344");

    result = ip_addr_get_bits(&addr, 0, 15);
    assert(cmp_ipv6(&result, "2001::"));

    result = ip_addr_get_bits(&addr, 16, 31);
    assert(cmp_ipv6(&result, "0:db8::"));
    result = ip_addr_get_bits(&addr, 0, 0);
    assert(cmp_ipv6(&result, "::"));
    result = ip_addr_get_bits(&addr, 5, 5);
    assert(cmp_ipv6(&result, "2000::"));
}

void lpfst_test(){
    ip_addr addr;
    addr.ver = IPV4;
    lpfst_node* result;
    unsigned int lvl = 0;
    //n1
    lpfst_node n1;
    n1.len = 16;
    n1.lchild = NULL;
    n1.rchild = NULL;
    n1.parent = NULL;
    n1.data = NULL;
    put_ipv4(&(n1.prefix), "100.200.0.0"); 

    put_ipv4(&(addr), "100.200.0.0"); 
    lvl = 0;
    result = lpfst_lookup(&n1, &addr, 16, &lvl);
    assert(result != NULL);
    assert(cmp_ipv4(&(result->prefix), "100.200.0.0"));

    put_ipv4(&(addr), "100.200.30.0"); 
    lvl = 0;
    result = lpfst_lookup(&n1, &addr, 16, &lvl);
    assert(result != NULL);
    assert(cmp_ipv4(&(result->prefix), "100.200.0.0"));


    //n2
    lpfst_node n2;
    n2.len = 16;
    n2.lchild = NULL;
    n2.rchild = NULL;
    n2.parent = NULL;
    n2.data = NULL;
    put_ipv4(&(n2.prefix), "132.200.0.0"); 
    lpfst_insert(&n1, &n2, 0);

    put_ipv4(&(addr), "132.200.0.0"); 
    lvl = 0;
    result = lpfst_lookup(&n1, &addr, 16, &lvl);
    assert(result != NULL);
    assert(cmp_ipv4(&(result->prefix), "132.200.0.0"));
    assert(n1.lchild == &n2);


    //n3
    lpfst_node n3;
    n3.len = 16;
    n3.lchild = NULL;
    n3.rchild = NULL;
    n3.parent = NULL;
    n3.data = NULL;
    put_ipv4(&(n3.prefix), "101.200.0.0"); 
    lpfst_insert(&n1, &n3, 0);

    put_ipv4(&(addr), "101.200.0.0"); 
    lvl = 0;
    result = lpfst_lookup(&n1, &addr, 16, &lvl);
    assert(result != NULL);
    assert(cmp_ipv4(&(result->prefix), "101.200.0.0"));
    assert(n1.rchild == &n3);

    //n4
    lpfst_node n4;
    n4.len = 24;
    n4.lchild = NULL;
    n4.rchild = NULL;
    n4.parent = NULL;
    n4.data = NULL;
    put_ipv4(&(n4.prefix), "132.200.3.0"); 
    lpfst_insert(&n1, &n4, 0);

    put_ipv4(&(addr), "132.200.3.0"); 
    lvl = 0;
    result = lpfst_lookup(&n1, &addr, 24, &lvl);
    assert(result != NULL);
    assert(cmp_ipv4(&(result->prefix), "132.200.3.0"));
    assert(cmp_ipv4(&(n1.prefix), "132.200.3.0"));
    assert(cmp_ipv4(&(n1.lchild->prefix), "132.200.0.0"));
    assert(cmp_ipv4(&(n1.rchild->prefix), "101.200.0.0"));
    assert(cmp_ipv4(&(n1.lchild->lchild->prefix), "100.200.0.0"));

    put_ipv4(&(addr), "132.0.0.0"); 
    lvl = 0;
    result = lpfst_lookup(&n1, &addr, 16, &lvl);
    assert(result == NULL);
    //assert(cmp_ipv4(&(result->prefix), "132.200.3.0"));

    lvl = 0;
    put_ipv4(&(addr), "132.0.0.0"); 
    result = lpfst_lookup_exact(&n1, &addr, 16, &lvl);
    assert(result == NULL);

    lvl = 0;
    put_ipv4(&(addr), "132.200.3.0"); 
    result = lpfst_lookup_exact(&n1, &addr, 24, &lvl);
    assert(cmp_ipv4(&(result->prefix), "132.200.3.0"));

    result = lpfst_remove(&n1, &addr, 0);
    assert(result != NULL);
    assert(cmp_ipv4(&(n1.prefix), "101.200.0.0"));
    assert(cmp_ipv4(&(n1.lchild->prefix), "132.200.0.0"));
    assert(cmp_ipv4(&(n1.lchild->lchild->prefix), "100.200.0.0"));
}


int main(){
    get_bits_testv4();
    get_bits_testv6();
    lpfst_test();
    printf("Test successfull\n");
}
