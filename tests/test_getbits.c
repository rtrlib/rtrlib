#include "rtrlib/lib/ip_private.h"

#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * @brief Test IPv4 address bit operations required by trie
 */
static void get_bits_testv4(void)
{
	struct lrtr_ip_addr addr;
	struct lrtr_ip_addr result;

	addr.ver = LRTR_IPV4;
	addr.u.addr4.addr = 0xAABBCC22;

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

	/* ip_str_to_addr("109.105.128.0", &addr);
	 * result = ip_addr_get_bits(&addr, 0, 8);
	 * printf("%u\n", result.u.addr4.addr);
	 */
	char buf[INET_ADDRSTRLEN];

	assert(lrtr_ip_str_to_addr("10.10.10.5", &addr) == 0);
	assert(lrtr_ip_addr_to_str(&addr, buf, sizeof(buf)) == 0);
	assert(strcmp("10.10.10.5", buf) == 0);
}

/*
 * @brief Test IPv6 address bit operations required by trie
 */
static void get_bits_testv6(void)
{
	struct lrtr_ip_addr addr;
	struct lrtr_ip_addr result;

	addr.ver = LRTR_IPV6;
	addr.u.addr6.addr[0] = 0x22AABBCC;
	addr.u.addr6.addr[1] = 0xDDEEFF99;
	addr.u.addr6.addr[2] = 0x33001122;
	addr.u.addr6.addr[3] = 0x33445566;

	result = lrtr_ip_addr_get_bits(&addr, 0, 128);
	assert(result.u.addr6.addr[0] == addr.u.addr6.addr[0] && result.u.addr6.addr[1] == addr.u.addr6.addr[1] &&
	       result.u.addr6.addr[2] == addr.u.addr6.addr[2] && result.u.addr6.addr[3] == addr.u.addr6.addr[3]);

	result = lrtr_ip_addr_get_bits(&addr, 0, 64);
	assert(result.u.addr6.addr[0] == addr.u.addr6.addr[0] && result.u.addr6.addr[1] == addr.u.addr6.addr[1] &&
	       result.u.addr6.addr[2] == 0 && result.u.addr6.addr[3] == 0);

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

int main(void)
{
	get_bits_testv4();
	get_bits_testv6();
	printf("Test successful\n");
	return EXIT_SUCCESS;
}
