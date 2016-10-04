
/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "rtrlib/lib/ip.h"

static void test_v4(void) {
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


}

static void test_v6(void) {
	struct lrtr_ip_addr addr;
	char buf[INET6_ADDRSTRLEN];

	lrtr_ip_str_to_addr("fdf8:f53b:82e4::53", &addr);
	assert(addr.u.addr6.addr[0] == 4260951355);
	assert(addr.u.addr6.addr[1] == 2195980288);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 83);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("fdf8:f53b:82e4::53", buf) == 0);

	lrtr_ip_str_to_addr("fe80::200:5aee:feaa:20a2", &addr);
	assert(addr.u.addr6.addr[0] == 4269801472);
	assert(addr.u.addr6.addr[1] == 0);
	assert(addr.u.addr6.addr[2] == 33577710);
	assert(addr.u.addr6.addr[3] == 4272562338);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("fe80::200:5aee:feaa:20a2", buf) == 0);

	lrtr_ip_str_to_addr("2001::1", &addr);
	assert(addr.u.addr6.addr[0] == 536936448);
	assert(addr.u.addr6.addr[1] == 0);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 1);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("2001::1", buf) == 0);

	lrtr_ip_str_to_addr("2001:0:4136:e378:8000:63bf:3fff:fdd2", &addr);
	assert(addr.u.addr6.addr[0] == 536936448);
	assert(addr.u.addr6.addr[1] == 1094116216);
	assert(addr.u.addr6.addr[2] == 2147509183);
	assert(addr.u.addr6.addr[3] == 1073741266);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("2001:0:4136:e378:8000:63bf:3fff:fdd2", buf) == 0);

	lrtr_ip_str_to_addr("2001:2:6c::430", &addr);
	assert(addr.u.addr6.addr[0] == 536936450);
	assert(addr.u.addr6.addr[1] == 7077888);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 1072);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("2001:2:6c::430", buf) == 0);

	lrtr_ip_str_to_addr("2001:10:240:ab::a", &addr);
	assert(addr.u.addr6.addr[0] == 536936464);
	assert(addr.u.addr6.addr[1] == 37748907);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 10);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("2001:10:240:ab::a", buf) == 0);

	lrtr_ip_str_to_addr("2002:cb0a:3cdd:1::1", &addr);
	assert(addr.u.addr6.addr[0] == 537053962);
	assert(addr.u.addr6.addr[1] == 1021116417);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 1);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("2002:cb0a:3cdd:1::1", buf) == 0);

	lrtr_ip_str_to_addr("2001:db8:8:4::2", &addr);
	assert(addr.u.addr6.addr[0] == 536939960);
	assert(addr.u.addr6.addr[1] == 524292);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 2);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("2001:db8:8:4::2", buf) == 0);

	lrtr_ip_str_to_addr("FF01:0:0:0:0:0:0:2", &addr);
	assert(addr.u.addr6.addr[0] == 4278255616);
	assert(addr.u.addr6.addr[1] == 0);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 2);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("ff01::2", buf) == 0);

	lrtr_ip_str_to_addr("fdf8:f53b:82e4::53", &addr);
	assert(addr.u.addr6.addr[0] == 4260951355);
	assert(addr.u.addr6.addr[1] == 2195980288);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 83);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("fdf8:f53b:82e4::53", buf) == 0);

	lrtr_ip_str_to_addr("fe80::200:5aee:feaa:20a2", &addr);
	assert(addr.u.addr6.addr[0] == 4269801472);
	assert(addr.u.addr6.addr[1] == 0);
	assert(addr.u.addr6.addr[2] == 33577710);
	assert(addr.u.addr6.addr[3] == 4272562338);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("fe80::200:5aee:feaa:20a2", buf) == 0);

	lrtr_ip_str_to_addr("2001::1", &addr);
	assert(addr.u.addr6.addr[0] == 536936448);
	assert(addr.u.addr6.addr[1] == 0);
	assert(addr.u.addr6.addr[2] == 0);
	assert(addr.u.addr6.addr[3] == 1);
	lrtr_ip_addr_to_str(&addr, buf, sizeof(buf));
	assert(strcmp("2001::1", buf) == 0);

	lrtr_ip_str_to_addr("2001:0:4136:e378:8000:63bf:3fff:fdd2", &addr);
	assert(addr.u.addr6.addr[0] == 536936448);
	assert(addr.u.addr6.addr[1] == 1094116216);
	assert(addr.u.addr6.addr[2] == 2147509183);
	assert(addr.u.addr6.addr[3] == 1073741266);
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


	/* buffer size check*/
	assert(lrtr_ip_addr_to_str(&addr, buf, 10) == -1);

	/* test leading single colon check */
	assert(lrtr_ip_str_to_addr(":ffff::ffff", &addr) == -1);

	/* test multiple double colons check */
	assert(lrtr_ip_str_to_addr("::ffff::ffff", &addr) == -1);

}


int main(void) {
	test_v4();
	test_v6();
	printf("Test successful\n");
	return EXIT_SUCCESS;
}
