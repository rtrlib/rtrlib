/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "packets_private.h"

#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/lib/convert_byte_order_private.h"
#include "rtrlib/lib/log_private.h"
#include "rtrlib/lib/utils_private.h"
#include "rtrlib/pfx/pfx_private.h"
#include "rtrlib/rtr/rtr_private.h"
#include "rtrlib/spki/hashtable/ht-spkitable_private.h"
#include "rtrlib/transport/transport_private.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MGR_DBG1(a) lrtr_dbg("RTR_MGR: " a)
#define TEMPORARY_PDU_STORE_INCREMENT_VALUE 100
#define MAX_SUPPORTED_PDU_TYPE 10

enum pdu_error_type {
	CORRUPT_DATA = 0,
	INTERNAL_ERROR = 1,
	NO_DATA_AVAIL = 2,
	INVALID_REQUEST = 3,
	UNSUPPORTED_PROTOCOL_VER = 4,
	UNSUPPORTED_PDU_TYPE = 5,
	WITHDRAWAL_OF_UNKNOWN_RECORD = 6,
	DUPLICATE_ANNOUNCEMENT = 7,
	UNEXPECTED_PROTOCOL_VERSION = 8,
	PDU_TOO_BIG = 32
};

enum pdu_type {
	SERIAL_NOTIFY = 0,
	SERIAL_QUERY = 1,
	RESET_QUERY = 2,
	CACHE_RESPONSE = 3,
	IPV4_PREFIX = 4,
	RESERVED = 5,
	IPV6_PREFIX = 6,
	EOD = 7,
	CACHE_RESET = 8,
	ROUTER_KEY = 9,
	ERROR = 10
};

struct pdu_header {
	uint8_t ver;
	uint8_t type;
	uint16_t reserved;
	uint32_t len;
};

struct pdu_cache_response {
	uint8_t ver;
	uint8_t type;
	uint16_t session_id;
	uint32_t len;
};

struct pdu_serial_notify {
	uint8_t ver;
	uint8_t type;
	uint16_t session_id;
	uint32_t len;
	uint32_t sn;
};

struct pdu_serial_query {
	uint8_t ver;
	uint8_t type;
	uint16_t session_id;
	uint32_t len;
	uint32_t sn;
};

struct pdu_ipv4 {
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
};

struct pdu_ipv6 {
	uint8_t ver;
	uint8_t type;
	uint16_t reserved;
	uint32_t len;
	uint8_t flags;
	uint8_t prefix_len;
	uint8_t max_prefix_len;
	uint8_t zero;
	uint32_t prefix[4];
	uint32_t asn;
};

struct pdu_error {
	uint8_t ver;
	uint8_t type;
	uint16_t error_code;
	uint32_t len;
	uint32_t len_enc_pdu;
	uint8_t rest[];
};

struct pdu_router_key {
	uint8_t ver;
	uint8_t type;
	uint8_t flags;
	uint8_t zero;
	uint32_t len;
	uint8_t ski[SKI_SIZE];
	uint32_t asn;
	uint8_t spki[SPKI_SIZE];
} __attribute__((packed));

/*
 * 0          8          16         24        31
 * .-------------------------------------------.
 * | Protocol |   PDU    |                     |
 * | Version  |   Type   |    reserved = zero  |
 * |    0     |    2     |                     |
 * +-------------------------------------------+
 * |                                           |
 * |                 Length=8                  |
 * |                                           |
 * `-------------------------------------------'
 */
struct pdu_reset_query {
	uint8_t ver;
	uint8_t type;
	uint16_t flags;
	uint32_t len;
};

struct pdu_end_of_data_v0 {
	uint8_t ver;
	uint8_t type;
	uint16_t session_id;
	uint32_t len;
	uint32_t sn;
};

struct pdu_end_of_data_v1 {
	uint8_t ver;
	uint8_t type;
	uint16_t session_id;
	uint32_t len;
	uint32_t sn;
	uint32_t refresh_interval;
	uint32_t retry_interval;
	uint32_t expire_interval;
};

struct recv_loop_cleanup_args {
	struct pdu_ipv4 *ipv4_pdus;
	struct pdu_ipv6 *ipv6_pdus;
	struct pdu_router_key *router_key_pdus;
};

static void recv_loop_cleanup(void *p);

static int rtr_send_error_pdu_from_network(const struct rtr_socket *rtr_socket, const void *erroneous_pdu,
					   const uint32_t erroneous_pdu_len, const enum pdu_error_type error,
					   const char *err_text, const uint32_t err_text_len);

static int rtr_send_error_pdu_from_host(const struct rtr_socket *rtr_socket, const void *erroneous_pdu,
					const uint32_t erroneous_pdu_len, const enum pdu_error_type error,
					const char *err_text, const uint32_t err_text_len);

static int interval_send_error_pdu(struct rtr_socket *rtr_socket, void *pdu, uint32_t interval, uint16_t minimum,
				   uint32_t maximum);

static inline enum pdu_type rtr_get_pdu_type(const void *pdu)
{
	return *((char *)pdu + 1);
}

static int rtr_set_last_update(struct rtr_socket *rtr_socket)
{
	if (lrtr_get_monotonic_time(&(rtr_socket->last_update)) == -1) {
		RTR_DBG1("get_monotonic_time(..) failed ");
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		return RTR_ERROR;
	}
	return RTR_SUCCESS;
}

int rtr_check_interval_range(uint32_t interval, uint32_t minimum, uint32_t maximum)
{
	if (interval < minimum)
		return RTR_BELOW_INTERVAL_RANGE;
	else if (interval > maximum)
		return RTR_ABOVE_INTERVAL_RANGE;

	return RTR_INSIDE_INTERVAL_RANGE;
}

void apply_interval_value(struct rtr_socket *rtr_socket, uint32_t interval, enum rtr_interval_type type)
{
	if (type == RTR_INTERVAL_TYPE_EXPIRATION)
		rtr_socket->expire_interval = interval;
	else if (type == RTR_INTERVAL_TYPE_REFRESH)
		rtr_socket->refresh_interval = interval;
	else if (type == RTR_INTERVAL_TYPE_RETRY)
		rtr_socket->retry_interval = interval;
}

int rtr_check_interval_option(struct rtr_socket *rtr_socket, int interval_mode, uint32_t interval,
			      enum rtr_interval_type type)
{
	uint16_t minimum;
	uint32_t maximum;

	int interv_retval;

	switch (type) {
	case RTR_INTERVAL_TYPE_EXPIRATION:
		minimum = RTR_EXPIRATION_MIN;
		maximum = RTR_EXPIRATION_MAX;
		interv_retval = rtr_check_interval_range(interval, minimum, maximum);
		break;
	case RTR_INTERVAL_TYPE_REFRESH:
		minimum = RTR_REFRESH_MIN;
		maximum = RTR_REFRESH_MAX;
		interv_retval = rtr_check_interval_range(interval, minimum, maximum);
		break;
	case RTR_INTERVAL_TYPE_RETRY:
		minimum = RTR_RETRY_MIN;
		maximum = RTR_RETRY_MAX;
		interv_retval = rtr_check_interval_range(interval, minimum, maximum);
		break;
	default:
		RTR_DBG("Invalid interval type: %u.", type);
		return RTR_ERROR;
	}

	if (interv_retval == RTR_INSIDE_INTERVAL_RANGE || interval_mode == RTR_INTERVAL_MODE_ACCEPT_ANY) {
		apply_interval_value(rtr_socket, interval, type);
	} else if (interval_mode == RTR_INTERVAL_MODE_DEFAULT_MIN_MAX) {
		if (interv_retval == RTR_BELOW_INTERVAL_RANGE)
			apply_interval_value(rtr_socket, minimum, type);
		else
			apply_interval_value(rtr_socket, maximum, type);
	} else {
		RTR_DBG("Received expiration value out of range. Was %u. It will be ignored.",
			interval);
	}

	return RTR_SUCCESS;
}

void rtr_change_socket_state(struct rtr_socket *rtr_socket, const enum rtr_socket_state new_state)
{
	if (rtr_socket->state == new_state)
		return;

	// RTR_SHUTDOWN state is final,struct rtr_socket will be shutdowned can't be switched to any other state
	if (rtr_socket->state == RTR_SHUTDOWN)
		return;

	rtr_socket->state = new_state;
	if (new_state == RTR_SHUTDOWN)
		MGR_DBG1("Calling rtr_mgr_cb with RTR_SHUTDOWN");

	if (rtr_socket->connection_state_fp)
		rtr_socket->connection_state_fp(rtr_socket, new_state, rtr_socket->connection_state_fp_param_config,
						rtr_socket->connection_state_fp_param_group);
}

static void rtr_pdu_convert_header_byte_order(void *pdu, const enum target_byte_order target_byte_order)
{
	struct pdu_header *header = pdu;

	// The ROUTER_KEY PDU has two 1 Byte fields instead of the 2 Byte reserved field.
	if (header->type != ROUTER_KEY)
		header->reserved = lrtr_convert_short(target_byte_order, header->reserved);

	header->len = lrtr_convert_long(target_byte_order, header->len);
}

static void rtr_pdu_convert_footer_byte_order(void *pdu, const enum target_byte_order target_byte_order)
{
	struct pdu_error *err_pdu;
	struct pdu_header *header = pdu;
	uint32_t addr6[4];
	const enum pdu_type type = rtr_get_pdu_type(pdu);

	switch (type) {
	case SERIAL_QUERY:
		((struct pdu_serial_query *)pdu)->sn =
			lrtr_convert_long(target_byte_order, ((struct pdu_serial_query *)pdu)->sn);
		break;

	case ERROR:
		err_pdu = (struct pdu_error *)pdu;
		if (target_byte_order == TO_NETWORK_BYTE_ORDER) {
			*((uint32_t *)(err_pdu->rest + err_pdu->len_enc_pdu)) = lrtr_convert_long(
				target_byte_order, *((uint32_t *)(err_pdu->rest + err_pdu->len_enc_pdu)));
			err_pdu->len_enc_pdu = lrtr_convert_long(target_byte_order, err_pdu->len_enc_pdu);
		} else {
			err_pdu->len_enc_pdu = lrtr_convert_long(target_byte_order, err_pdu->len_enc_pdu);
			*((uint32_t *)(err_pdu->rest + err_pdu->len_enc_pdu)) = lrtr_convert_long(
				target_byte_order, *((uint32_t *)(err_pdu->rest + err_pdu->len_enc_pdu)));
		}
		break;

	case SERIAL_NOTIFY:
		((struct pdu_serial_notify *)pdu)->sn =
			lrtr_convert_long(target_byte_order, ((struct pdu_serial_notify *)pdu)->sn);
		break;

	case EOD:
		if (header->ver == RTR_PROTOCOL_VERSION_1) {
			((struct pdu_end_of_data_v1 *)pdu)->expire_interval = lrtr_convert_long(
				target_byte_order, ((struct pdu_end_of_data_v1 *)pdu)->expire_interval);

			((struct pdu_end_of_data_v1 *)pdu)->refresh_interval = lrtr_convert_long(
				target_byte_order, ((struct pdu_end_of_data_v1 *)pdu)->refresh_interval);

			((struct pdu_end_of_data_v1 *)pdu)->retry_interval = lrtr_convert_long(
				target_byte_order, ((struct pdu_end_of_data_v1 *)pdu)->retry_interval);

			((struct pdu_end_of_data_v1 *)pdu)->sn =
				lrtr_convert_long(target_byte_order, ((struct pdu_end_of_data_v1 *)pdu)->sn);
		} else {
			((struct pdu_end_of_data_v0 *)pdu)->sn =
				lrtr_convert_long(target_byte_order, ((struct pdu_end_of_data_v0 *)pdu)->sn);
		}
		break;

	case IPV4_PREFIX:
		lrtr_ipv4_addr_convert_byte_order(((struct pdu_ipv4 *)pdu)->prefix, &((struct pdu_ipv4 *)pdu)->prefix,
						  target_byte_order);
		((struct pdu_ipv4 *)pdu)->asn = lrtr_convert_long(target_byte_order, ((struct pdu_ipv4 *)pdu)->asn);
		break;

	case IPV6_PREFIX:
		lrtr_ipv6_addr_convert_byte_order(((struct pdu_ipv6 *)pdu)->prefix, addr6, target_byte_order);
		memcpy(((struct pdu_ipv6 *)pdu)->prefix, addr6, sizeof(addr6));
		((struct pdu_ipv6 *)pdu)->asn = lrtr_convert_long(target_byte_order, ((struct pdu_ipv6 *)pdu)->asn);
		break;

	case ROUTER_KEY:
		((struct pdu_router_key *)pdu)->asn =
			lrtr_convert_long(target_byte_order, ((struct pdu_router_key *)pdu)->asn);
		break;

	default:
		break;
	}
}

static void rtr_pdu_header_to_network_byte_order(void *pdu)
{
	rtr_pdu_convert_header_byte_order(pdu, TO_NETWORK_BYTE_ORDER);
}

static void rtr_pdu_footer_to_network_byte_order(void *pdu)
{
	rtr_pdu_convert_footer_byte_order(pdu, TO_NETWORK_BYTE_ORDER);
}

static void rtr_pdu_to_network_byte_order(void *pdu)
{
	rtr_pdu_footer_to_network_byte_order(pdu);
	rtr_pdu_header_to_network_byte_order(pdu);
}

static void rtr_pdu_footer_to_host_byte_order(void *pdu)
{
	rtr_pdu_convert_footer_byte_order(pdu, TO_HOST_HOST_BYTE_ORDER);
}

static void rtr_pdu_header_to_host_byte_order(void *pdu)
{
	rtr_pdu_convert_header_byte_order(pdu, TO_HOST_HOST_BYTE_ORDER);
}

/*
 * Check if the PDU is big enough for the PDU type it
 * pretend to be.
 * @param pdu A pointer to a PDU that is at least pdu->len byte large.
 * @return False if the check fails, else true
 */
static bool rtr_pdu_check_size(const struct pdu_header *pdu)
{
	const enum pdu_type type = rtr_get_pdu_type(pdu);
	const struct pdu_error *err_pdu = NULL;
	bool retval = false;
	uint64_t min_size = 0;

	switch (type) {
	case SERIAL_NOTIFY:
		if (sizeof(struct pdu_serial_notify) == pdu->len)
			retval = true;
		break;
	case CACHE_RESPONSE:
		if (sizeof(struct pdu_cache_response) == pdu->len)
			retval = true;
		break;
	case IPV4_PREFIX:
		if (sizeof(struct pdu_ipv4) == pdu->len)
			retval = true;
		break;
	case IPV6_PREFIX:
		if (sizeof(struct pdu_ipv6) == pdu->len)
			retval = true;
		break;
	case EOD:
		if ((pdu->ver == RTR_PROTOCOL_VERSION_0 && (sizeof(struct pdu_end_of_data_v0) == pdu->len)) ||
		    (pdu->ver == RTR_PROTOCOL_VERSION_1 && (sizeof(struct pdu_end_of_data_v1) == pdu->len))) {
			retval = true;
		}
		break;
	case CACHE_RESET:
		if (sizeof(struct pdu_header) == pdu->len)
			retval = true;
		break;
	case ROUTER_KEY:
		if (sizeof(struct pdu_router_key) == pdu->len)
			retval = true;
		break;
	case ERROR:
		err_pdu = (const struct pdu_error *)pdu;
		// +4 because of the "Length of Error Text" field
		min_size = 4 + sizeof(struct pdu_error);
		if (err_pdu->len < min_size) {
			RTR_DBG1("PDU is too small to contain \"Length of Error Text\" field!");
			break;
		}

		// Check if the PDU really contains the error PDU
		uint32_t enc_pdu_len = ntohl(err_pdu->len_enc_pdu);

		RTR_DBG("enc_pdu_len: %u", enc_pdu_len);
		min_size += enc_pdu_len;
		if (err_pdu->len < min_size) {
			RTR_DBG1("PDU is too small to contain erroneous PDU!");
			break;
		}

		// Check if the the PDU really contains the error msg
		uint32_t err_msg_len = ntohl(*((uint32_t *)(err_pdu->rest + enc_pdu_len)));

		RTR_DBG("err_msg_len: %u", err_msg_len);
		min_size += err_msg_len;
		if (err_pdu->len != min_size) {
			RTR_DBG1("PDU is too small to contain error_msg!");
			break;
		}

		retval = true;
		break;
	case SERIAL_QUERY:
		if (sizeof(struct pdu_serial_query) == pdu->len)
			retval = true;
		break;
	case RESET_QUERY:
		if (sizeof(struct pdu_reset_query) == pdu->len)
			retval = true;
		break;
	case RESERVED:
	default:
		RTR_DBG1("PDU type is unknown or reserved!");
		retval = false;
		break;
	}

#ifndef NDEBUG
	if (!retval)
		RTR_DBG1("Received malformed PDU!");
#endif

	return retval;
}

static int rtr_send_pdu(const struct rtr_socket *rtr_socket, const void *pdu, const unsigned int len)
{
	char pdu_converted[len];

	memcpy(pdu_converted, pdu, len);
	rtr_pdu_to_network_byte_order(pdu_converted);
	if (rtr_socket->state == RTR_SHUTDOWN)
		return RTR_ERROR;
	const int rtval = tr_send_all(rtr_socket->tr_socket, pdu_converted, len, RTR_SEND_TIMEOUT);

	if (rtval > 0)
		return RTR_SUCCESS;
	if (rtval == TR_WOULDBLOCK) {
		RTR_DBG1("send would block");
		return RTR_ERROR;
	}

	RTR_DBG1("Error sending PDU");
	return RTR_ERROR;
}

/*
 * if RTR_ERROR was returned a error PDU was sent, and the socket state changed
 * @param pdu_len must >= RTR_MAX_PDU_LEN Bytes
 * @return RTR_SUCCESS
 * @return RTR_ERROR, error pdu was sent and socket_state changed
 * @return TR_WOULDBLOCK
 * \post
 * If RTR_SUCCESS is returned PDU points to a well formed PDU that has
 * the appropriate size for the PDU type it pretend to be. Thus, casting it to
 * the PDU type struct and using it is save. Furthermore all PDU field are
 * in host-byte-order.
 */
static int rtr_receive_pdu(struct rtr_socket *rtr_socket, void *pdu, const size_t pdu_len, const time_t timeout)
{
	int error = RTR_SUCCESS;

	assert(pdu_len >= RTR_MAX_PDU_LEN);

	if (rtr_socket->state == RTR_SHUTDOWN)
		return RTR_ERROR;
	// receive packet header
	error = tr_recv_all(rtr_socket->tr_socket, pdu, sizeof(struct pdu_header), timeout);
	if (error < 0)
		goto error;
	else
		error = RTR_SUCCESS;

	// header in hostbyte order, retain original received pdu, in case we need to detach it to an error pdu
	struct pdu_header header;

	memcpy(&header, pdu, sizeof(header));
	rtr_pdu_header_to_host_byte_order(&header);

	// if header->len is < packet_header = corrupt data received
	if (header.len < sizeof(header)) {
		error = CORRUPT_DATA;
		goto error;
	} else if (header.len > RTR_MAX_PDU_LEN) { // PDU too big, > than MAX_PDU_LEN Bytes
		error = PDU_TOO_BIG;
		goto error;
	}

	// Handle live downgrading
	if (!rtr_socket->has_received_pdus) {
		if (rtr_socket->version == RTR_PROTOCOL_VERSION_1 && header.ver == RTR_PROTOCOL_VERSION_0 &&
		    header.type != ERROR) {
			RTR_DBG("First received PDU is a version 0 PDU, downgrading to %u", RTR_PROTOCOL_VERSION_0);
			rtr_socket->version = RTR_PROTOCOL_VERSION_0;
		}
		rtr_socket->has_received_pdus = true;
	}

	// Handle wrong protocol version
	// If it is a error PDU, it will be handled by rtr_handle_error_pdu
	if (header.ver != rtr_socket->version && header.type != ERROR) {
		error = UNEXPECTED_PROTOCOL_VERSION;
		goto error;
	}

	// receive packet payload
	const unsigned int remaining_len = header.len - sizeof(header);

	if (remaining_len > 0) {
		if (rtr_socket->state == RTR_SHUTDOWN)
			return RTR_ERROR;
		error = tr_recv_all(rtr_socket->tr_socket, (((char *)pdu) + sizeof(header)), remaining_len,
				    RTR_RECV_TIMEOUT);
		if (error < 0)
			goto error;
		else
			error = RTR_SUCCESS;
	}
	// copy header in host_byte_order to pdu
	memcpy(pdu, &header, sizeof(header));

	// Check if the header len value is valid
	if (rtr_pdu_check_size(pdu) == false) {
		// TODO Restore byteorder for sending error PDU
		error = CORRUPT_DATA;
		goto error;
	}
	// At this point it is save to cast and use the PDU

	rtr_pdu_footer_to_host_byte_order(pdu);

	// Here we should handle error PDUs instead of doing it in
	// several other places...

	if (header.type == IPV4_PREFIX || header.type == IPV6_PREFIX) {
		if (((struct pdu_ipv4 *)pdu)->zero != 0)
			RTR_DBG1("Warning: Zero field of received Prefix PDU doesn't contain 0");
	}
	if (header.type == ROUTER_KEY && ((struct pdu_router_key *)pdu)->zero != 0)
		RTR_DBG1("Warning: ROUTER_KEY_PDU zero field is != 0");

	return RTR_SUCCESS;

error:
	// send error msg to server, including unmodified pdu header(pdu variable instead header)
	if (error == -1) {
		rtr_change_socket_state(rtr_socket, RTR_ERROR_TRANSPORT);
		return RTR_ERROR;
	} else if (error == TR_WOULDBLOCK) {
		RTR_DBG1("receive timeout expired");
		return TR_WOULDBLOCK;
	} else if (error == TR_INTR) {
		RTR_DBG1("receive call interrupted");
		return TR_INTR;
	} else if (error == CORRUPT_DATA) {
		RTR_DBG1("corrupt PDU received");
		const char txt[] = "corrupt data received, length value in PDU is too small";

		rtr_send_error_pdu_from_network(rtr_socket, pdu, sizeof(header), CORRUPT_DATA, txt, sizeof(txt));
	} else if (error == PDU_TOO_BIG) {
		RTR_DBG1("PDU too big");
		char txt[42];

		snprintf(txt, sizeof(txt), "PDU too big, max. PDU size is: %u bytes", RTR_MAX_PDU_LEN);
		RTR_DBG("%s", txt);
		rtr_send_error_pdu_from_network(rtr_socket, pdu, sizeof(header), CORRUPT_DATA, txt, sizeof(txt));
	} else if (error == UNSUPPORTED_PDU_TYPE) {
		RTR_DBG("Unsupported PDU type (%u) received", header.type);
		rtr_send_error_pdu_from_network(rtr_socket, pdu, sizeof(header), UNSUPPORTED_PDU_TYPE, NULL, 0);
	} else if (error == UNSUPPORTED_PROTOCOL_VER) {
		RTR_DBG("PDU with unsupported Protocol version (%u) received", header.ver);
		rtr_send_error_pdu_from_network(rtr_socket, pdu, sizeof(header), UNSUPPORTED_PROTOCOL_VER, NULL, 0);
		return RTR_ERROR;
	} else if (error == UNEXPECTED_PROTOCOL_VERSION) {
		RTR_DBG("PDU with unexpected Protocol version (%u) received", header.ver);
		rtr_send_error_pdu_from_network(rtr_socket, pdu, sizeof(header), UNEXPECTED_PROTOCOL_VERSION, NULL, 0);
		return RTR_ERROR;
	}

	rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
	return RTR_ERROR;
}

static int rtr_handle_error_pdu(struct rtr_socket *rtr_socket, const void *buf)
{
	RTR_DBG1("Error PDU received"); // TODO: append server ip & port
	const struct pdu_error *pdu = buf;

	switch (pdu->error_code) {
	case CORRUPT_DATA:
		RTR_DBG1("Corrupt data received");
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		break;
	case INTERNAL_ERROR:
		RTR_DBG1("Internal error on server-side");
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		break;
	case NO_DATA_AVAIL:
		RTR_DBG1("No data available");
		rtr_change_socket_state(rtr_socket, RTR_ERROR_NO_DATA_AVAIL);
		break;
	case INVALID_REQUEST:
		RTR_DBG1("Invalid request from client");
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		break;
	case UNSUPPORTED_PROTOCOL_VER:
		RTR_DBG1("Client uses unsupported protocol version");
		if (pdu->ver <= RTR_PROTOCOL_MAX_SUPPORTED_VERSION && pdu->ver >= RTR_PROTOCOL_MIN_SUPPORTED_VERSION &&
		    pdu->ver < rtr_socket->version) {
			RTR_DBG("Downgrading from %i to version %i", rtr_socket->version, pdu->ver);
			rtr_socket->version = pdu->ver;
			rtr_change_socket_state(rtr_socket, RTR_FAST_RECONNECT);
		} else {
			RTR_DBG("Got UNSUPPORTED_PROTOCOL_VER error PDU with invalid values, current version:%i, PDU version:%i",
				rtr_socket->version, pdu->ver);
			rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		}
		break;
	case UNSUPPORTED_PDU_TYPE:
		RTR_DBG1("Client set unsupported PDU type");
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		break;
	default:
		RTR_DBG("error unknown, server sent unsupported error code %u", pdu->error_code);
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		break;
	}

	const uint32_t len_err_txt = *((uint32_t *)(pdu->rest + pdu->len_enc_pdu));

	if (len_err_txt > 0) {
		if ((sizeof(pdu->ver) + sizeof(pdu->type) + sizeof(pdu->error_code) + sizeof(pdu->len) +
		     sizeof(pdu->len_enc_pdu) + pdu->len_enc_pdu + 4 + len_err_txt) != pdu->len) {
			RTR_DBG1("error: Length of error text contains an incorrect value");
		} else {
			char *pdu_txt = (char *)pdu->rest + pdu->len_enc_pdu + sizeof(len_err_txt);

			RTR_DBG("Error PDU included the following error msg: \'%.*s\'", len_err_txt, pdu_txt);
		}
	}

	return RTR_SUCCESS;
}

static int rtr_handle_cache_response_pdu(struct rtr_socket *rtr_socket, char *pdu)
{
	RTR_DBG1("Cache Response PDU received");
	struct pdu_cache_response *cr_pdu = (struct pdu_cache_response *)pdu;
	// set connection session_id
	if (rtr_socket->request_session_id) {
		if (rtr_socket->last_update != 0) {
			RTR_DBG1("Resetting Socket.");

			rtr_socket->last_update = 0;
			rtr_socket->is_resetting = true;
		}
		rtr_socket->session_id = cr_pdu->session_id;
	} else {
		if (rtr_socket->session_id != cr_pdu->session_id) {
			const char txt[] =
				"Wrong session_id in Cache Response PDU"; //TODO: Appendrtr_socket->session_id to string
			rtr_send_error_pdu_from_host(rtr_socket, NULL, 0, CORRUPT_DATA, txt, sizeof(txt));
			rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
			return RTR_ERROR;
		}
	}
	return RTR_SUCCESS;
}

static void rtr_key_pdu_2_spki_record(const struct rtr_socket *rtr_socket, const struct pdu_router_key *pdu,
				      struct spki_record *entry, const enum pdu_type type)
{
	assert(type == ROUTER_KEY);
	entry->asn = pdu->asn;
	memcpy(entry->ski, pdu->ski, SKI_SIZE);
	memcpy(entry->spki, pdu->spki, SPKI_SIZE);
	entry->socket = rtr_socket;
}

static void rtr_prefix_pdu_2_pfx_record(const struct rtr_socket *rtr_socket, const void *pdu, struct pfx_record *pfxr,
					const enum pdu_type type)
{
	assert(type == IPV4_PREFIX || type == IPV6_PREFIX);
	if (type == IPV4_PREFIX) {
		const struct pdu_ipv4 *ipv4 = pdu;

		pfxr->prefix.u.addr4.addr = ipv4->prefix;
		pfxr->asn = ipv4->asn;
		pfxr->prefix.ver = LRTR_IPV4;
		pfxr->min_len = ipv4->prefix_len;
		pfxr->max_len = ipv4->max_prefix_len;
		pfxr->socket = rtr_socket;
	} else if (type == IPV6_PREFIX) {
		const struct pdu_ipv6 *ipv6 = pdu;

		pfxr->asn = ipv6->asn;
		pfxr->prefix.ver = LRTR_IPV6;
		memcpy(pfxr->prefix.u.addr6.addr, ipv6->prefix, sizeof(pfxr->prefix.u.addr6.addr));
		pfxr->min_len = ipv6->prefix_len;
		pfxr->max_len = ipv6->max_prefix_len;
		pfxr->socket = rtr_socket;
	}
}

/*
 * @brief Removes all Prefix from the pfx_table with flag field == ADD, ADDs all Prefix PDU to the pfx_table with flag
 * field == REMOVE.
 */
static int rtr_undo_update_pfx_table(struct rtr_socket *rtr_socket, struct pfx_table *pfx_table, void *pdu)
{
	const enum pdu_type type = rtr_get_pdu_type(pdu);

	assert(type == IPV4_PREFIX || type == IPV6_PREFIX);

	struct pfx_record pfxr;

	rtr_prefix_pdu_2_pfx_record(rtr_socket, pdu, &pfxr, type);

	int rtval = RTR_ERROR;
	// invert add/remove operation
	if (((struct pdu_ipv4 *)pdu)->flags == 1)
		rtval = pfx_table_remove(pfx_table, &pfxr);
	else if (((struct pdu_ipv4 *)pdu)->flags == 0)
		rtval = pfx_table_add(pfx_table, &pfxr);
	return rtval;
}

/*
 * @brief Removes router_key from the spki_table with flag field == ADD, ADDs router_key PDU to the spki_table with flag
 * field == REMOVE.
 */
static int rtr_undo_update_spki_table(struct rtr_socket *rtr_socket, struct spki_table *spki_table, void *pdu)
{
	const enum pdu_type type = rtr_get_pdu_type(pdu);

	assert(type == ROUTER_KEY);

	struct spki_record entry;

	rtr_key_pdu_2_spki_record(rtr_socket, pdu, &entry, type);

	int rtval = RTR_ERROR;
	// invert add/remove operation
	if (((struct pdu_router_key *)pdu)->flags == 1)
		rtval = spki_table_remove_entry(spki_table, &entry);
	else if (((struct pdu_router_key *)pdu)->flags == 0)
		rtval = spki_table_add_entry(spki_table, &entry);
	return rtval;
}

/*
 * @brief Appends the Prefix PDU pdu to ary.
 *
 * @return RTR_SUCCESS On success
 * @return RTR_ERROR On realloc failure
 * @attention ary is not freed in this case, because it might contain data that is still needed
 */
static int rtr_store_prefix_pdu(struct rtr_socket *rtr_socket, const void *pdu, const unsigned int pdu_size, void **ary,
				unsigned int *ind, unsigned int *size)
{
	const enum pdu_type type = rtr_get_pdu_type(pdu);

	assert(type == IPV4_PREFIX || type == IPV6_PREFIX);
	if ((*ind) >= *size) {
		*size += TEMPORARY_PDU_STORE_INCREMENT_VALUE;
		void *tmp = lrtr_realloc(*ary, *size * pdu_size);

		if (!tmp) {
			const char txt[] = "Realloc failed";

			RTR_DBG("%s", txt);
			rtr_send_error_pdu_from_host(rtr_socket, NULL, 0, INTERNAL_ERROR, txt, sizeof(txt));
			rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
			return RTR_ERROR;
		}
		*ary = tmp;
	}
	if (type == IPV4_PREFIX) {
		struct pdu_ipv4 *ary_ipv4 = *ary;

		memcpy(ary_ipv4 + *ind, pdu, pdu_size);
	} else if (type == IPV6_PREFIX) {
		struct pdu_ipv6 *ary_ipv6 = *ary;

		memcpy(ary_ipv6 + *ind, pdu, pdu_size);
	}
	(*ind)++;
	return RTR_SUCCESS;
}

/*
 * @brief Appends the router key to ary.
 *
 * @return RTR_SUCCESS On success
 * @return RTR_ERROR On realloc failure
 * @attention ary is not freed in this case, because it might contain data that is still needed
 */
static int rtr_store_router_key_pdu(struct rtr_socket *rtr_socket, const void *pdu, const unsigned int pdu_size,
				    struct pdu_router_key **ary, unsigned int *ind, unsigned int *size)
{
	assert(rtr_get_pdu_type(pdu) == ROUTER_KEY);

	if ((*ind) >= *size) {
		*size += TEMPORARY_PDU_STORE_INCREMENT_VALUE;
		void *tmp = lrtr_realloc(*ary, *size * pdu_size);

		if (!tmp) {
			const char txt[] = "Realloc failed";

			RTR_DBG("%s", txt);
			rtr_send_error_pdu_from_host(rtr_socket, NULL, 0, INTERNAL_ERROR, txt, sizeof(txt));
			rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
			return RTR_ERROR;
		}
		*ary = tmp;
	}

	memcpy((struct pdu_router_key *)*ary + *ind, pdu, pdu_size);
	(*ind)++;
	return RTR_SUCCESS;
}

static int rtr_update_pfx_table(struct rtr_socket *rtr_socket, struct pfx_table *pfx_table, const void *pdu)
{
	const enum pdu_type type = rtr_get_pdu_type(pdu);

	assert(type == IPV4_PREFIX || type == IPV6_PREFIX);

	struct pfx_record pfxr;
	size_t pdu_size = (type == IPV4_PREFIX ? sizeof(struct pdu_ipv4) : sizeof(struct pdu_ipv6));

	rtr_prefix_pdu_2_pfx_record(rtr_socket, pdu, &pfxr, type);

	int rtval;

	if (((struct pdu_ipv4 *)pdu)->flags == 1) {
		rtval = pfx_table_add(pfx_table, &pfxr);
	} else if (((struct pdu_ipv4 *)pdu)->flags == 0) {
		rtval = pfx_table_remove(pfx_table, &pfxr);
	} else {
		const char txt[] = "Prefix PDU with invalid flags value received";

		RTR_DBG("%s", txt);
		rtr_send_error_pdu_from_host(rtr_socket, pdu, pdu_size, CORRUPT_DATA, txt, sizeof(txt));
		return RTR_ERROR;
	}

	if (rtval == PFX_DUPLICATE_RECORD) {
		char ip[INET6_ADDRSTRLEN];

		lrtr_ip_addr_to_str(&(pfxr.prefix), ip, INET6_ADDRSTRLEN);
		RTR_DBG("Duplicate Announcement for record: %s/%u-%u, ASN: %u, received", ip, pfxr.min_len,
			pfxr.max_len, pfxr.asn);
		rtr_send_error_pdu_from_host(rtr_socket, pdu, pdu_size, DUPLICATE_ANNOUNCEMENT, NULL, 0);
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		return RTR_ERROR;
	} else if (rtval == PFX_RECORD_NOT_FOUND) {
		RTR_DBG1("Withdrawal of unknown record");
		rtr_send_error_pdu_from_host(rtr_socket, pdu, pdu_size, WITHDRAWAL_OF_UNKNOWN_RECORD, NULL, 0);
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		return RTR_ERROR;
	} else if (rtval == PFX_ERROR) {
		const char txt[] = "PFX_TABLE Error";

		RTR_DBG("%s", txt);
		rtr_send_error_pdu_from_host(rtr_socket, NULL, 0, INTERNAL_ERROR, txt, sizeof(txt));
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		return RTR_ERROR;
	}

	return RTR_SUCCESS;
}

static int rtr_update_spki_table(struct rtr_socket *rtr_socket, struct spki_table *spki_table, const void *pdu)
{
	const enum pdu_type type = rtr_get_pdu_type(pdu);

	assert(type == ROUTER_KEY);

	struct spki_record entry;

	size_t pdu_size = sizeof(struct pdu_router_key);

	rtr_key_pdu_2_spki_record(rtr_socket, pdu, &entry, type);

	int rtval;

	if (((struct pdu_router_key *)pdu)->flags == 1) {
		rtval = spki_table_add_entry(spki_table, &entry);

	} else if (((struct pdu_router_key *)pdu)->flags == 0) {
		rtval = spki_table_remove_entry(spki_table, &entry);

	} else {
		const char txt[] = "Router Key PDU with invalid flags value received";

		RTR_DBG("%s", txt);
		rtr_send_error_pdu_from_host(rtr_socket, pdu, pdu_size, CORRUPT_DATA, txt, sizeof(txt));
		return RTR_ERROR;
	}

	if (rtval == SPKI_DUPLICATE_RECORD) {
		// TODO: This debug message isn't working yet, how to display SKI/SPKI without %x?
		RTR_DBG("Duplicate Announcement for router key: ASN: %u received", entry.asn);
		rtr_send_error_pdu_from_host(rtr_socket, pdu, pdu_size, DUPLICATE_ANNOUNCEMENT, NULL, 0);
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		return RTR_ERROR;
	} else if (rtval == SPKI_RECORD_NOT_FOUND) {
		RTR_DBG1("Withdrawal of unknown router key");
		rtr_send_error_pdu_from_host(rtr_socket, pdu, pdu_size, WITHDRAWAL_OF_UNKNOWN_RECORD, NULL, 0);
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		return RTR_ERROR;
	} else if (rtval == SPKI_ERROR) {
		const char txt[] = "spki_table Error";

		RTR_DBG("%s", txt);
		rtr_send_error_pdu_from_host(rtr_socket, NULL, 0, INTERNAL_ERROR, txt, sizeof(txt));
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		return RTR_ERROR;
	}

	return RTR_SUCCESS;
}

void recv_loop_cleanup(void *p)
{
	struct recv_loop_cleanup_args *args = p;

	lrtr_free(args->ipv4_pdus);
	lrtr_free(args->ipv6_pdus);
	lrtr_free(args->router_key_pdus);
}

/* WARNING: This Function has cancelable sections*/
static int rtr_sync_receive_and_store_pdus(struct rtr_socket *rtr_socket)
{
	char pdu[RTR_MAX_PDU_LEN];
	enum pdu_type type;
	int retval = RTR_SUCCESS;

	struct pdu_ipv6 *ipv6_pdus = NULL;
	unsigned int ipv6_pdus_nindex = 0; // next free index in ipv6_pdus
	unsigned int ipv6_pdus_size = 0;

	struct pdu_ipv4 *ipv4_pdus = NULL;
	unsigned int ipv4_pdus_size = 0;
	unsigned int ipv4_pdus_nindex = 0; // next free index in ipv4_pdus

	struct pdu_router_key *router_key_pdus = NULL;
	unsigned int router_key_pdus_size = 0;
	unsigned int router_key_pdus_nindex = 0;

	struct pfx_table *pfx_shadow_table = NULL;
	struct spki_table *spki_shadow_table = NULL;

	int oldcancelstate;
	struct recv_loop_cleanup_args cleanup_args = {
		.ipv4_pdus = ipv4_pdus, .ipv6_pdus = ipv6_pdus, .router_key_pdus = router_key_pdus};

	// receive LRTR_IPV4/IPV6 PDUs till EOD
	do {
		pthread_cleanup_push(recv_loop_cleanup, &cleanup_args);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldcancelstate);
		retval = rtr_receive_pdu(rtr_socket, pdu, RTR_MAX_PDU_LEN, RTR_RECV_TIMEOUT);
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldcancelstate);
		pthread_cleanup_pop(0);

		if (retval == TR_WOULDBLOCK) {
			rtr_change_socket_state(rtr_socket, RTR_ERROR_TRANSPORT);
			retval = RTR_ERROR;
			goto cleanup;
		} else if (retval < 0) {
			retval = RTR_ERROR;
			goto cleanup;
		}

		type = rtr_get_pdu_type(pdu);
		if (type == IPV4_PREFIX) {
			if (rtr_store_prefix_pdu(rtr_socket, pdu, sizeof(*ipv4_pdus), (void **)&ipv4_pdus,
						 &ipv4_pdus_nindex, &ipv4_pdus_size) == RTR_ERROR) {
				rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
				retval = RTR_ERROR;
				goto cleanup;
			}
		} else if (type == IPV6_PREFIX) {
			if (rtr_store_prefix_pdu(rtr_socket, pdu, sizeof(*ipv6_pdus), (void **)&ipv6_pdus,
						 &ipv6_pdus_nindex, &ipv6_pdus_size) == RTR_ERROR) {
				rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
				retval = RTR_ERROR;
				goto cleanup;
			}
		} else if (type == ROUTER_KEY) {
			if (rtr_store_router_key_pdu(rtr_socket, pdu, sizeof(*router_key_pdus), &router_key_pdus,
						     &router_key_pdus_nindex, &router_key_pdus_size) == RTR_ERROR) {
				rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
				retval = RTR_ERROR;
				goto cleanup;
			}
		} else if (type == EOD) {
			RTR_DBG1("EOD PDU received.");
			struct pdu_end_of_data_v0 *eod_pdu = (struct pdu_end_of_data_v0 *)pdu;

			if (eod_pdu->session_id != rtr_socket->session_id) {
				char txt[67];

				snprintf(txt, sizeof(txt),
					 "Expected session_id: %u, received session_id. %u in EOD PDU",
					 rtr_socket->session_id, eod_pdu->session_id);
				rtr_send_error_pdu_from_host(rtr_socket, pdu, RTR_MAX_PDU_LEN, CORRUPT_DATA, txt,
							     strlen(txt) + 1);
				rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
				retval = RTR_ERROR;
				goto cleanup;
			}

			if (eod_pdu->ver == RTR_PROTOCOL_VERSION_1 &&
			    rtr_socket->iv_mode != RTR_INTERVAL_MODE_IGNORE_ANY) {
				int interv_retval;

				interv_retval =
					rtr_check_interval_option(rtr_socket, rtr_socket->iv_mode,
								  ((struct pdu_end_of_data_v1 *)pdu)->expire_interval,
								  RTR_INTERVAL_TYPE_EXPIRATION);

				if (interv_retval == RTR_ERROR) {
					interval_send_error_pdu(rtr_socket, pdu,
								((struct pdu_end_of_data_v1 *)pdu)->expire_interval,
								RTR_EXPIRATION_MIN, RTR_EXPIRATION_MAX);
					retval = RTR_ERROR;
					goto cleanup;
				}

				interv_retval =
					rtr_check_interval_option(rtr_socket, rtr_socket->iv_mode,
								  ((struct pdu_end_of_data_v1 *)pdu)->refresh_interval,
								  RTR_INTERVAL_TYPE_REFRESH);

				if (interv_retval == RTR_ERROR) {
					interval_send_error_pdu(rtr_socket, pdu,
								((struct pdu_end_of_data_v1 *)pdu)->refresh_interval,
								RTR_REFRESH_MIN, RTR_REFRESH_MAX);
					retval = RTR_ERROR;
					goto cleanup;
				}

				interv_retval = rtr_check_interval_option(
					rtr_socket, rtr_socket->iv_mode,
					((struct pdu_end_of_data_v1 *)pdu)->retry_interval, RTR_INTERVAL_TYPE_RETRY);

				if (interv_retval == RTR_ERROR) {
					interval_send_error_pdu(rtr_socket, pdu,
								((struct pdu_end_of_data_v1 *)pdu)->retry_interval,
								RTR_RETRY_MIN, RTR_RETRY_MAX);
					retval = RTR_ERROR;
					goto cleanup;
				}

				RTR_DBG("New interval values: expire_interval:%u, refresh_interval:%u, retry_interval:%u",
					rtr_socket->expire_interval, rtr_socket->refresh_interval,
					rtr_socket->retry_interval);
			}

			struct pfx_table *pfx_update_table;
			struct spki_table *spki_update_table;

			if (rtr_socket->is_resetting) {
				RTR_DBG1("Reset in progress creating shadow table for atomic reset");
				pfx_shadow_table = lrtr_malloc(sizeof(struct pfx_table));
				if (!pfx_shadow_table) {
					RTR_DBG1("Memory allocation for pfx shadow table failed");
					retval = RTR_ERROR;
					goto cleanup;
				}

				pfx_table_init(pfx_shadow_table, NULL);
				pfx_update_table = pfx_shadow_table;
				if (pfx_table_copy_except_socket(rtr_socket->pfx_table, pfx_update_table, rtr_socket)) {
					RTR_DBG1("Creation of pfx shadow table failed");
					rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
					retval = RTR_ERROR;
					goto cleanup;
				}

				spki_shadow_table = lrtr_malloc(sizeof(struct spki_table));
				if (!spki_shadow_table) {
					RTR_DBG1("Memory allocation for spki shadow table failed");
					retval = RTR_ERROR;
					goto cleanup;
				}
				spki_table_init(spki_shadow_table, NULL);
				spki_update_table = spki_shadow_table;
				if (spki_table_copy_except_socket(rtr_socket->spki_table, spki_update_table,
								  rtr_socket) != SPKI_SUCCESS) {
					RTR_DBG1("Creation of spki shadow table failed");
					rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
					retval = RTR_ERROR;
					goto cleanup;
				}

				RTR_DBG1("Shadow table created");
			} else {
				pfx_update_table = rtr_socket->pfx_table;
				spki_update_table = rtr_socket->spki_table;
			}

			retval = PFX_SUCCESS;
			// add all IPv4 prefix pdu to the pfx_table
			for (unsigned int i = 0; i < ipv4_pdus_nindex; i++) {
				if (rtr_update_pfx_table(rtr_socket, pfx_update_table, &(ipv4_pdus[i])) == PFX_ERROR) {
					// undo all record updates, except the last which produced the error
					RTR_DBG("Error during data synchronisation, recovering Serial Nr. %u state",
						rtr_socket->serial_number);
					for (unsigned int j = 0; j < i && retval == PFX_SUCCESS; j++)
						retval = rtr_undo_update_pfx_table(rtr_socket, pfx_update_table,
										   &(ipv4_pdus[j]));
					if (retval == RTR_ERROR) {
						RTR_DBG1(
							"Couldn't undo all update operations from failed data synchronisation: Purging all records");
						pfx_table_src_remove(rtr_socket->pfx_table, rtr_socket);
						rtr_socket->request_session_id = true;
					}
					rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
					retval = RTR_ERROR;
					goto cleanup;
				}
			}
			RTR_DBG1("v4 prefixes added");
			// add all IPv6 prefix pdu to the pfx_table
			for (unsigned int i = 0; i < ipv6_pdus_nindex; i++) {
				if (rtr_update_pfx_table(rtr_socket, pfx_update_table, &(ipv6_pdus[i])) == PFX_ERROR) {
					// undo all record updates if error occurred
					RTR_DBG("Error during data synchronisation, recovering Serial Nr. %u state",
						rtr_socket->serial_number);
					for (unsigned int j = 0; j < ipv4_pdus_nindex && retval == PFX_SUCCESS; j++)
						retval = rtr_undo_update_pfx_table(rtr_socket, pfx_update_table,
										   &(ipv4_pdus[j]));
					for (unsigned int j = 0; j < i && retval == PFX_SUCCESS; j++)
						retval = rtr_undo_update_pfx_table(rtr_socket, pfx_update_table,
										   &(ipv6_pdus[j]));
					if (retval == PFX_ERROR) {
						RTR_DBG1(
							"Couldn't undo all update operations from failed data synchronisation: Purging all records");
						pfx_table_src_remove(rtr_socket->pfx_table, rtr_socket);
						rtr_socket->request_session_id = true;
					}
					rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
					retval = RTR_ERROR;
					goto cleanup;
				}
			}

			RTR_DBG1("v6 prefixes added");
			// add all router key pdu to the spki_table
			for (unsigned int i = 0; i < router_key_pdus_nindex; i++) {
				if (rtr_update_spki_table(rtr_socket, spki_update_table, &(router_key_pdus[i])) ==
				    SPKI_ERROR) {
					RTR_DBG("Error during router key data synchronisation, recovering Serial Nr. %u state",
						rtr_socket->serial_number);
					for (unsigned int j = 0; j < ipv4_pdus_nindex && retval == PFX_SUCCESS; j++)
						retval = rtr_undo_update_pfx_table(rtr_socket, pfx_update_table,
										   &(ipv4_pdus[j]));
					for (unsigned int j = 0; j < ipv6_pdus_nindex && retval == PFX_SUCCESS; j++)
						retval = rtr_undo_update_pfx_table(rtr_socket, pfx_update_table,
										   &(ipv6_pdus[j]));
					for (unsigned int j = 0;
					// cppcheck-suppress duplicateExpression
					     j < i && (retval == PFX_SUCCESS || retval == SPKI_SUCCESS); j++)
						retval = rtr_undo_update_spki_table(rtr_socket, spki_update_table,
										    &(router_key_pdus[j]));
					// cppcheck-suppress duplicateExpression
					if (retval == RTR_ERROR || retval == SPKI_ERROR) {
						RTR_DBG1(
							"Couldn't undo all update operations from failed data synchronisation: Purging all key entries");
						spki_table_src_remove(spki_update_table, rtr_socket);
						rtr_socket->request_session_id = true;
					}
					rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
					retval = RTR_ERROR;
					goto cleanup;
				}
			}
			RTR_DBG1("spki data added");
			if (rtr_socket->is_resetting) {
				RTR_DBG1("Reset finished. Swapping new table in.");
				pfx_table_swap(rtr_socket->pfx_table, pfx_shadow_table);
				spki_table_swap(rtr_socket->spki_table, spki_shadow_table);

				if (rtr_socket->pfx_table->update_fp) {
					RTR_DBG1("Calculating and notifying pfx diff");
					pfx_table_notify_diff(rtr_socket->pfx_table, pfx_shadow_table, rtr_socket);
				} else {
					RTR_DBG1("No pfx update callback. Skipping diff");
				}

				if (rtr_socket->spki_table->update_fp) {
					RTR_DBG1("Calculating and notifying spki diff");
					spki_table_notify_diff(rtr_socket->spki_table, spki_shadow_table, rtr_socket);
				} else {
					RTR_DBG1("No spki update callback. Skipping diff");
				}
			}

			rtr_socket->serial_number = eod_pdu->sn;
			RTR_DBG("Sync successful, received %u Prefix PDUs, %u Router Key PDUs, session_id: %u, SN: %u",
				(ipv4_pdus_nindex + ipv6_pdus_nindex), router_key_pdus_nindex, rtr_socket->session_id,
				rtr_socket->serial_number);
			goto cleanup;
		} else if (type == ERROR) {
			rtr_handle_error_pdu(rtr_socket, pdu);
			retval = RTR_ERROR;
			goto cleanup;
		} else if (type == SERIAL_NOTIFY) {
			RTR_DBG1("Ignoring Serial Notify");
		} else {
			RTR_DBG("Received unexpected PDU (Type: %u)", ((struct pdu_header *)pdu)->type);
			const char txt[] = "Unexpected PDU received during data synchronisation";

			rtr_send_error_pdu_from_host(rtr_socket, pdu, sizeof(struct pdu_header), CORRUPT_DATA, txt,
						     sizeof(txt));
			retval = RTR_ERROR;
			goto cleanup;
		}
	} while (type != EOD);

cleanup:

	if (rtr_socket->is_resetting) {
		RTR_DBG1("Freeing shadow tables.");
		if (pfx_shadow_table) {
			pfx_table_free_without_notify(pfx_shadow_table);
			lrtr_free(pfx_shadow_table);
		}

		if (spki_shadow_table) {
			spki_table_free_without_notify(spki_shadow_table);
			lrtr_free(spki_shadow_table);
		}
		rtr_socket->is_resetting = false;
	}

	lrtr_free(router_key_pdus);
	lrtr_free(ipv6_pdus);
	lrtr_free(ipv4_pdus);
	return retval;
}

/* WARNING: This Function has cancelable sections */
int rtr_sync(struct rtr_socket *rtr_socket)
{
	char pdu[RTR_MAX_PDU_LEN];
	enum pdu_type type;

	int oldcancelstate;

	do {
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldcancelstate);
		int rtval = rtr_receive_pdu(rtr_socket, pdu, RTR_MAX_PDU_LEN, RTR_RECV_TIMEOUT);

		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldcancelstate);
		// If the cache has closed the connection and we don't have a
		// session_id (no packages where exchanged) we should downgrade.
		if (rtval == TR_CLOSED && rtr_socket->request_session_id) {
			RTR_DBG1("The cache server closed the connection and we have no session_id!");
			if (rtr_socket->version > RTR_PROTOCOL_MIN_SUPPORTED_VERSION) {
				RTR_DBG("Downgrading from %i to version %i", rtr_socket->version,
					rtr_socket->version - 1);
				rtr_socket->version = rtr_socket->version - 1;
				rtr_change_socket_state(rtr_socket, RTR_FAST_RECONNECT);
				return RTR_ERROR;
			}
		}

		if (rtval == TR_WOULDBLOCK) {
			rtr_change_socket_state(rtr_socket, RTR_ERROR_TRANSPORT);
			return RTR_ERROR;
		} else if (rtval < 0) {
			return RTR_ERROR;
		}

		type = rtr_get_pdu_type(pdu);
		if (type == SERIAL_NOTIFY)
			RTR_DBG1("Ignoring Serial Notify");

	} while (type == SERIAL_NOTIFY);

	switch (type) {
	case ERROR:
		rtr_handle_error_pdu(rtr_socket, pdu);
		return RTR_ERROR;
	case CACHE_RESET:
		RTR_DBG1("Cache Reset PDU received");
		rtr_change_socket_state(rtr_socket, RTR_ERROR_NO_INCR_UPDATE_AVAIL);
		return RTR_ERROR;
	case CACHE_RESPONSE:
		rtr_handle_cache_response_pdu(rtr_socket, pdu);
		break;
	default:
		RTR_DBG("Expected Cache Response PDU but received PDU Type (Type: %u)",
			((struct pdu_header *)pdu)->type);
		const char txt[] = "Unexpected PDU received in data synchronisation";

		rtr_send_error_pdu_from_host(rtr_socket, pdu, sizeof(struct pdu_header), CORRUPT_DATA, txt,
					     sizeof(txt));
		return RTR_ERROR;
	}

	// Receive all PDUs until EOD PDU
	if (rtr_sync_receive_and_store_pdus(rtr_socket) == RTR_ERROR)
		return RTR_ERROR;

	rtr_socket->request_session_id = false;
	if (rtr_set_last_update(rtr_socket) == RTR_ERROR)
		return RTR_ERROR;

	return RTR_SUCCESS;
}

int rtr_wait_for_sync(struct rtr_socket *rtr_socket)
{
	char pdu[RTR_MAX_PDU_LEN];

	time_t cur_time;

	lrtr_get_monotonic_time(&cur_time);
	time_t wait = (rtr_socket->last_update + rtr_socket->refresh_interval) - cur_time;

	if (wait < 0)
		wait = 0;

	RTR_DBG("waiting %jd sec. till next sync", (intmax_t)wait);
	const int rtval = rtr_receive_pdu(rtr_socket, pdu, sizeof(pdu), wait);

	if (rtval >= 0) {
		enum pdu_type type = rtr_get_pdu_type(pdu);

		if (type == SERIAL_NOTIFY) {
			RTR_DBG("Serial Notify received (%u)", ((struct pdu_serial_notify *)pdu)->sn);
			return RTR_SUCCESS;
		}
	} else if (rtval == TR_WOULDBLOCK) {
		RTR_DBG1("Refresh interval expired");
		return RTR_SUCCESS;
	}
	return RTR_ERROR;
}

static int rtr_send_error_pdu(const struct rtr_socket *rtr_socket, const void *erroneous_pdu,
			      const uint32_t erroneous_pdu_len, const enum pdu_error_type error, const char *err_text,
			      const uint32_t err_text_len)
{
	struct pdu_error *err_pdu;
	unsigned int msg_size = sizeof(struct pdu_error) + 4 + erroneous_pdu_len + err_text_len;
	uint8_t msg[msg_size];

	// don't send errors for erroneous error PDUs
	if (erroneous_pdu_len >= 2) {
		if (rtr_get_pdu_type(erroneous_pdu) == ERROR) {
			RTR_DBG1("Don't send errors for erroneous error PDUs");
			return RTR_SUCCESS;
		}
	}

	err_pdu = (struct pdu_error *)msg;
	err_pdu->ver = rtr_socket->version;
	err_pdu->type = ERROR;
	err_pdu->error_code = error;
	err_pdu->len = msg_size;

	err_pdu->len_enc_pdu = erroneous_pdu_len;
	if (erroneous_pdu_len > 0)
		memcpy(err_pdu->rest, erroneous_pdu, erroneous_pdu_len);

	*((uint32_t *)(err_pdu->rest + erroneous_pdu_len)) = err_text_len;
	if (err_text_len > 0)
		memcpy(err_pdu->rest + erroneous_pdu_len + 4, err_text, err_text_len);

	return rtr_send_pdu(rtr_socket, msg, msg_size);
}

static int interval_send_error_pdu(struct rtr_socket *rtr_socket, void *pdu, uint32_t interval, uint16_t minimum,
				   uint32_t maximum)
{
	RTR_DBG("Received expiration value out of range. Was %u, must be between %u and %u.", interval, minimum,
		maximum);
	const char txt[] = "Interval value out of range";

	return rtr_send_error_pdu(rtr_socket, pdu, RTR_MAX_PDU_LEN, CORRUPT_DATA, txt, strlen(txt) + 1);
}

static int rtr_send_error_pdu_from_network(const struct rtr_socket *rtr_socket, const void *erroneous_pdu,
					   const uint32_t erroneous_pdu_len, const enum pdu_error_type error,
					   const char *err_text, const uint32_t err_text_len)
{
	return rtr_send_error_pdu(rtr_socket, erroneous_pdu, erroneous_pdu_len, error, err_text, err_text_len);
}

static int rtr_send_error_pdu_from_host(const struct rtr_socket *rtr_socket, const void *erroneous_pdu,
					const uint32_t erroneous_pdu_len, const enum pdu_error_type error,
					const char *err_text, const uint32_t err_text_len)
{
	char pdu[erroneous_pdu_len];

	memcpy(&pdu, erroneous_pdu, erroneous_pdu_len);

	if (erroneous_pdu_len == sizeof(struct pdu_header))
		rtr_pdu_header_to_network_byte_order(&pdu);
	else if (erroneous_pdu_len > sizeof(struct pdu_header))
		rtr_pdu_to_network_byte_order(&pdu);
	else
		return RTR_ERROR;

	return rtr_send_error_pdu(rtr_socket, &pdu, erroneous_pdu_len, error, err_text, err_text_len);
}

int rtr_send_serial_query(struct rtr_socket *rtr_socket)
{
	struct pdu_serial_query pdu;

	pdu.ver = rtr_socket->version;
	pdu.type = SERIAL_QUERY;
	pdu.session_id = rtr_socket->session_id;
	pdu.len = sizeof(pdu);
	pdu.sn = rtr_socket->serial_number;

	RTR_DBG("sending serial query, SN: %u", rtr_socket->serial_number);
	if (rtr_send_pdu(rtr_socket, &pdu, sizeof(pdu)) != RTR_SUCCESS) {
		rtr_change_socket_state(rtr_socket, RTR_ERROR_TRANSPORT);
		return RTR_ERROR;
	}
	return RTR_SUCCESS;
}

int rtr_send_reset_query(struct rtr_socket *rtr_socket)
{
	RTR_DBG1("Sending reset query");
	struct pdu_reset_query pdu;

	pdu.ver = rtr_socket->version;
	pdu.type = 2;
	pdu.flags = 0;
	pdu.len = 8;

	if (rtr_send_pdu(rtr_socket, &pdu, sizeof(pdu)) != RTR_SUCCESS) {
		rtr_change_socket_state(rtr_socket, RTR_ERROR_TRANSPORT);
		return RTR_ERROR;
	}
	return RTR_SUCCESS;
}
