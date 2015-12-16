/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
*/

#include "rtrlib/lib/ipv4.h"
#include "rtrlib/lib/utils.h"
#include <assert.h>
#include <stdio.h>

struct lrtr_ipv4_addr lrtr_ipv4_get_bits(const struct lrtr_ipv4_addr *val, const uint8_t from, const uint8_t quantity)
{
    struct lrtr_ipv4_addr result;
    result.addr = lrtr_get_bits(val->addr, from, quantity);
    return result;
}

int lrtr_ipv4_addr_to_str(const struct lrtr_ipv4_addr *ip, char *str, unsigned int len)
{
    const uint8_t *t = (uint8_t *) &(ip->addr);

    if (snprintf(str, len, "%hhu.%hhu.%hhu.%hhu", t[3], t[2], t[1], t[0]) < 0) {
        return -1;
    }
    return 0;
}

int lrtr_ipv4_str_to_addr(const char *str, struct lrtr_ipv4_addr *ip)
{
    unsigned char *t =  (unsigned char *) &(ip->addr);
    if(sscanf(str, "%hhu.%hhu.%hhu.%hhu", &(t[3]), &(t[2]), &(t[1]), &(t[0])) != 4)
        return -1;
    return 0;
}

bool lrtr_ipv4_addr_equal(const struct lrtr_ipv4_addr *a, const struct lrtr_ipv4_addr *b)
{
    if(a->addr == b->addr)
        return true;
    return false;
}
