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

#ifndef RTR_PACKETS_H
#define RTR_PACKETS_H
#include <arpa/inet.h>
#include "rtrlib/lib/ipv6.h"
#include "rtrlib/lib/ipv4.h"
#include "rtrlib/rtr/rtr.h"

static const uint8_t RTR_PROTOCOL_VERSION = 0;
static const unsigned int RTR_MAX_PDU_LEN = 1648; //error pdu: header(8) + len(4) + ipv6_pdu(32) + len(4) + 200*8 (200char text)
static const unsigned int RTR_RECV_TIMEOUT = 60;
static const unsigned int RTR_SEND_TIMEOUT = 60;

typedef enum pdu_error_type{
    CORRUPT_DATA = 0,
    INTERNAL_ERROR = 1,
    NO_DATA_AVAIL = 2,
    INVALID_REQUEST = 3,
    UNSUPPORTED_PROTOCOL_VER = 4,
    UNSUPPORTED_PDU_TYPE = 5,
    WITHDRAWAL_OF_UNKNOWN_RECORD = 6,
    DUPLICATE_ANNOUNCEMENT = 7
} pdu_error_type;

typedef enum pdu_type{
    SERIAL_NOTIFY = 0,
    SERIAL_QUERY = 1,
    RESET_QUERY = 2,
    CACHE_RESPONSE = 3,
    IPV4_PREFIX = 4,
    IPV6_PREFIX = 6,
    EOD = 7,
    CACHE_RESET = 8,
    ERROR = 10
} pdu_type;


typedef struct pdu_header{
    uint8_t ver;
    uint8_t type;
    uint16_t reserved;
    uint32_t len;
} pdu_header;

typedef struct pdu_serial_notify{
    uint8_t ver;
    uint8_t type;
    uint16_t nonce;
    uint32_t len;
    uint32_t sn;
} pdu_serial_notify;

typedef struct pdu_serial_query{
    uint8_t ver;
    uint8_t type;
    uint16_t nonce;
    uint32_t len;
    uint32_t sn;
} pdu_serial_query;

typedef struct pdu_serial_query pdu_eod;

typedef struct pdu_ipv4{
    uint8_t ver;
    uint8_t type;
    uint16_t reserved;
    uint32_t len;
    uint8_t flags;
    uint8_t prefix_len;
    uint8_t max_prefix_len;
    uint8_t zero;
    uint32_t prefix;
    uint32_t asn;
} pdu_ipv4;

typedef struct pdu_ipv6{
    uint8_t ver;
    uint8_t type;
    uint16_t reserved;
    uint32_t len;
    uint8_t flags;
    uint8_t prefix_len;
    uint8_t max_prefix_len;
    uint8_t zero;
    uint8_t prefix[16];
    uint32_t asn;
} pdu_ipv6;

typedef struct pdu_error{
    uint8_t ver;
    uint8_t type;
    uint16_t error_code;
    uint32_t len;
    uint32_t len_enc_pdu;
    //void pdu*; //todo
    //uint32_t len_txt;
    char rest[];
} pdu_error;


/*
   0          8          16         24        31
   .-------------------------------------------.
   | Protocol |   PDU    |                     |
   | Version  |   Type   |    reserved = zero  |
   |    0     |    2     |                     |
   +-------------------------------------------+
   |                                           |
   |                 Length=8                  |
   |                                           |
   `-------------------------------------------'
 */
static const pdu_header pdu_reset_query =
{
    0, //sollte RTR_PROTOCOL_VERSION sein, aber gcc mag dat net:(
    2,
    0,
    8
};

enum rtr_rtvals{
    RTR_SUCCESS = 0,
    RTR_ERROR = -1
};


void rtr_change_socket_state(rtr_socket* rtr_socket, const rtr_socket_state new_state);
int rtr_sync(rtr_socket* rtr_socket);
int rtr_wait_for_sync(rtr_socket* rtr_socket);
int rtr_send_serial_query(rtr_socket* rtr_socket);
int rtr_send_reset_query(rtr_socket* rtr_socket);

#endif
