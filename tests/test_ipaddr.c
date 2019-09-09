
/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "rtrlib/lib/ip.h"

#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * @brief test ipv4 parsing
 */
static void test_v4(void)
{
	struct lrtr_ip_addr addr;
	char buf[INET_ADDRSTRLEN];

	lrtr_ip_str_to_addr("0.0.0.0", &addr);
	assert(addr.ver == LRTR_IPV4);
	assert(addr.u.addr4.addr == 0);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("0.0.0.0", buf) == 0);

	lrtr_ip_str_to_addr("255.255.255.255", &addr);
	assert(addr.ver == LRTR_IPV4);
	assert(addr.u.addr4.addr == 0xffffffff);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("255.255.255.255", buf) == 0);

	lrtr_ip_str_to_addr("0.2.6.7", &addr);
	assert(addr.ver == LRTR_IPV4);
	assert(addr.u.addr4.addr == 0x20607);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("0.2.6.7", buf) == 0);

	lrtr_ip_str_to_addr("78.69.255.0", &addr);
	assert(addr.ver == LRTR_IPV4);
	assert(addr.u.addr4.addr == 0x4e45ff00);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("78.69.255.0", buf) == 0);

	lrtr_ip_str_to_addr("1.1.1.1", &addr);
	assert(addr.ver == LRTR_IPV4);
	assert(addr.u.addr4.addr == 0x1010101);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("1.1.1.1", buf) == 0);

	lrtr_ip_str_to_addr("5.0.255.255", &addr);
	assert(addr.ver == LRTR_IPV4);
	assert(addr.u.addr4.addr == 0x500ffff);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("5.0.255.255", buf) == 0);

	lrtr_ip_str_to_addr("8.9.6.3", &addr);
	assert(addr.ver == LRTR_IPV4);
	assert(addr.u.addr4.addr == 0x8090603);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("8.9.6.3", buf) == 0);

	/* check some malformed addresses */
	assert(lrtr_ip_str_to_addr("8,3,4,5", &addr) == -1);
	assert(lrtr_ip_str_to_addr("8.4.5", &addr) == -1);
}

/*
 * @brief test ipv6 parsing
 */
static void test_v6(void)
{
	struct lrtr_ip_addr addr;
	char buf[INET6_ADDRSTRLEN];

	lrtr_ip_str_to_addr("fdf8:f53b:82e4::53", &addr);
	assert(addr.u.addr6.addr[0] == 0xfdf8f53b);
	assert(addr.u.addr6.addr[1] == 0x82e40000);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 0x53);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("fdf8:f53b:82e4::53", buf) == 0);

	lrtr_ip_str_to_addr("fe80::200:5aee:feaa:20a2", &addr);
	assert(addr.u.addr6.addr[0] == 0xfe800000);
	assert(addr.u.addr6.addr[1] == 0);
	assert(addr.u.addr6.addr[2] == 0x2005aee);
	assert(addr.u.addr6.addr[3] == 0xfeaa20a2);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("fe80::200:5aee:feaa:20a2", buf) == 0);

	lrtr_ip_str_to_addr("2001::1", &addr);
	assert(addr.u.addr6.addr[0] == 0x20010000);
	assert(addr.u.addr6.addr[1] == 0);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 0x1);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("2001::1", buf) == 0);

	lrtr_ip_str_to_addr("2001:0:4136:e378:8000:63bf:3fff:fdd2", &addr);
	assert(addr.u.addr6.addr[0] == 0x20010000);
	assert(addr.u.addr6.addr[1] == 0x4136e378);
	assert(addr.u.addr6.addr[2] == 0x800063bf);
	assert(addr.u.addr6.addr[3] == 0x3ffffdd2);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("2001:0:4136:e378:8000:63bf:3fff:fdd2", buf) == 0);

	lrtr_ip_str_to_addr("2001:2:6c::430", &addr);
	assert(addr.u.addr6.addr[0] == 0x20010002);
	assert(addr.u.addr6.addr[1] == 0x6C0000);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 0x430);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("2001:2:6c::430", buf) == 0);

	lrtr_ip_str_to_addr("2001:10:240:ab::a", &addr);
	assert(addr.u.addr6.addr[0] == 0x20010010);
	assert(addr.u.addr6.addr[1] == 0x24000AB);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 0xa);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("2001:10:240:ab::a", buf) == 0);

	lrtr_ip_str_to_addr("2002:cb0a:3cdd:1::1", &addr);
	assert(addr.u.addr6.addr[0] == 0x2002cb0a);
	assert(addr.u.addr6.addr[1] == 0x3cdd0001);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 0x1);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("2002:cb0a:3cdd:1::1", buf) == 0);

	lrtr_ip_str_to_addr("2001:db8:8:4::2", &addr);
	assert(addr.u.addr6.addr[0] == 0x20010db8);
	assert(addr.u.addr6.addr[1] == 0x80004);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 0x2);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("2001:db8:8:4::2", buf) == 0);

	lrtr_ip_str_to_addr("FF01:0:0:0:0:0:0:2", &addr);
	assert(addr.u.addr6.addr[0] == 0xff010000);
	assert(addr.u.addr6.addr[1] == 0);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 0x2);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("ff01::2", buf) == 0);

	lrtr_ip_str_to_addr("fdf8:f53b:82e4::53", &addr);
	assert(addr.u.addr6.addr[0] == 0xfdf8f53b);
	assert(addr.u.addr6.addr[1] == 0x82e40000);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 0x53);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("fdf8:f53b:82e4::53", buf) == 0);

	lrtr_ip_str_to_addr("fe80::200:5aee:feaa:20a2", &addr);
	assert(addr.u.addr6.addr[0] == 0xfe800000);
	assert(addr.u.addr6.addr[1] == 0);
	assert(addr.u.addr6.addr[2] == 0x2005aee);
	assert(addr.u.addr6.addr[3] == 0xfeaa20a2);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("fe80::200:5aee:feaa:20a2", buf) == 0);

	lrtr_ip_str_to_addr("2001::1", &addr);
	assert(addr.u.addr6.addr[0] == 0x20010000);
	assert(addr.u.addr6.addr[1] == 0);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 1);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("2001::1", buf) == 0);

	lrtr_ip_str_to_addr("2001:0:4136:e378:8000:63bf:3fff:fdd2", &addr);
	assert(addr.u.addr6.addr[0] == 0x20010000);
	assert(addr.u.addr6.addr[1] == 0x4136e378);
	assert(addr.u.addr6.addr[2] == 0x800063bf);
	assert(addr.u.addr6.addr[3] == 0x3ffffdd2);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("2001:0:4136:e378:8000:63bf:3fff:fdd2", buf) == 0);

	/* test embedded ipv4 */
	lrtr_ip_str_to_addr("::ffff:192.0.2.128", &addr);
	assert(addr.u.addr6.addr[0] == 0);
	assert(addr.u.addr6.addr[1] == 0);
	assert(addr.u.addr6.addr[2] == 0xffff);
	assert(addr.u.addr6.addr[3] == 0xc0000280);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("::ffff:192.0.2.128", buf) == 0);

	lrtr_ip_str_to_addr("::10.58.64.34", &addr);
	assert(addr.u.addr6.addr[0] == 0);
	assert(addr.u.addr6.addr[1] == 0);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 0xa3a4022);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("::10.58.64.34", buf) == 0);

	/* test check for malformed embedded ipv4 */
	assert(lrtr_ip_str_to_addr("::ffff:192.0,2.128", &addr) == -1);

	/* buffer size check*/
	assert(lrtr_ip_addr_to_str(&addr, buf, 10) == -1);

	/* test leading single colon check */
	assert(lrtr_ip_str_to_addr(":ffff::ffff", &addr) == -1);

	/* test multiple double colons check */
	assert(lrtr_ip_str_to_addr("::ffff::ffff", &addr) == -1);

	/* test check for to long addresses */
	assert(lrtr_ip_str_to_addr("2001:0:6:8:0:f:3fff:fdd2:55", &addr) == -1);

	/* test check for to big groups */
	assert(lrtr_ip_str_to_addr("::fffff", &addr) == -1);

	/* check for null byte in address string */
	assert(lrtr_ip_str_to_addr("2001:\0::", &addr) == -1);
}

/*
 * @brief test ip comparisons
 */
static void test_cmp(void)
{
	struct lrtr_ip_addr addr1, addr2;

	lrtr_ip_str_to_addr("2001:0:4136:e378:8000:63bf:3fff:fdd2", &addr1);
	lrtr_ip_str_to_addr("2001:0:4136:e378:8000:63bf:3fff:fdd2", &addr2);

	assert(lrtr_ip_addr_equal(addr1, addr2) == true);

	lrtr_ip_str_to_addr("2001:0:4136:e378:8000:63bf:3fff:fdd3", &addr2);
	assert(lrtr_ip_addr_equal(addr1, addr2) == false);

	lrtr_ip_str_to_addr("141.22.5.22", &addr2);
	assert(lrtr_ip_addr_equal(addr1, addr2) == false);

	lrtr_ip_str_to_addr("141.22.5.22", &addr1);
	assert(lrtr_ip_addr_equal(addr1, addr2) == true);

	lrtr_ip_str_to_addr("141.26.5.23", &addr1);
}

int main(void)
{
	test_v4();
	test_v6();
	test_cmp();
	printf("Test successful\n");
	return EXIT_SUCCESS;
}
