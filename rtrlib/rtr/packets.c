/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "packets_private.h"

#include "rtrlib/aspa/aspa_private.h"
#include "rtrlib/config.h"
#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/lib/convert_byte_order_private.h"
#include "rtrlib/lib/log_private.h"
#include "rtrlib/lib/utils_private.h"
#include "rtrlib/pfx/pfx_private.h"
#include "rtrlib/rtr/rtr_pdus.h"
#include "rtrlib/rtr/rtr_private.h"
#include "rtrlib/spki/hashtable/ht-spkitable_private.h"
#include "rtrlib/spki/spkitable.h"
#include "rtrlib/transport/transport_private.h"
#ifdef RTRLIB_BGPSEC_ENABLED
#include "rtrlib/bgpsec/bgpsec_utils_private.h"
#endif

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MGR_DBG1(a) lrtr_dbg("RTR_MGR: " a)
#define TEMPORARY_PDU_STORE_INCREMENT_VALUE 100
#define MAX_SUPPORTED_PDU_TYPE 10

/**
 * @brief @c ASPA_UPDATE_MECHANISM macro sets the mechanism used in this file to update the ASPA table.
 * @see aspa/aspa_private.h
 */
#define ASPA_IN_PLACE 'P'
#define ASPA_SWAP_IN 'S'
#define ASPA_UPDATE_MECHANISM ASPA_SWAP_IN

struct aspa_pdu_list_node {
	struct pdu_aspa *pdu;
	struct aspa_pdu_list_node *next;
};

struct recv_loop_cleanup_args {
	struct pdu_ipv4 **ipv4_pdus;
	struct pdu_ipv6 **ipv6_pdus;
	struct pdu_router_key **router_key_pdus;
	struct pdu_aspa ***aspa_pdus;
	size_t *aspa_pdu_count;
};

//static void recv_loop_cleanup(struct recv_loop_cleanup_args *args);
static void recv_loop_cleanup(void *p);

static int rtr_send_error_pdu_from_network(const struct rtr_socket *rtr_socket, const void *erroneous_pdu,
					   const uint32_t erroneous_pdu_len, const enum pdu_error_type error,
					   const char *err_text, const uint32_t err_text_len);

static int rtr_send_error_pdu_from_host(const struct rtr_socket *rtr_socket, const void *erroneous_pdu,
					const uint32_t erroneous_pdu_len, const enum pdu_error_type error,
					const char *err_text, const uint32_t err_text_len);

static int interval_send_error_pdu(struct rtr_socket *rtr_socket, void *pdu, uint32_t interval, uint16_t minimum,
				   uint32_t maximum);

static bool rtr_version_supported(uint8_t version)
{
	return RTR_PROTOCOL_MIN_SUPPORTED_VERSION <= version && RTR_PROTOCOL_MAX_SUPPORTED_VERSION >= version;
}

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
		RTR_DBG("Received expiration value out of range. Was %u. It will be ignored.", interval);
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
		if (header->ver == RTR_PROTOCOL_VERSION_1 || header->ver == RTR_PROTOCOL_VERSION_2) {
			((struct pdu_end_of_data_v1_v2 *)pdu)->expire_interval = lrtr_convert_long(
				target_byte_order, ((struct pdu_end_of_data_v1_v2 *)pdu)->expire_interval);

			((struct pdu_end_of_data_v1_v2 *)pdu)->refresh_interval = lrtr_convert_long(
				target_byte_order, ((struct pdu_end_of_data_v1_v2 *)pdu)->refresh_interval);

			((struct pdu_end_of_data_v1_v2 *)pdu)->retry_interval = lrtr_convert_long(
				target_byte_order, ((struct pdu_end_of_data_v1_v2 *)pdu)->retry_interval);

			((struct pdu_end_of_data_v1_v2 *)pdu)->sn =
				lrtr_convert_long(target_byte_order, ((struct pdu_end_of_data_v1_v2 *)pdu)->sn);
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

	case ASPA:
		((struct pdu_aspa *)pdu)->provider_count =
			lrtr_convert_short(target_byte_order, ((struct pdu_aspa *)pdu)->provider_count);
		((struct pdu_aspa *)pdu)->customer_asn =
			lrtr_convert_long(target_byte_order, ((struct pdu_aspa *)pdu)->customer_asn);

		uint16_t asn_count = ((struct pdu_aspa *)pdu)->provider_count;

		// prevent converting twice
		if (target_byte_order != TO_HOST_HOST_BYTE_ORDER)
			asn_count = lrtr_convert_short(TO_HOST_HOST_BYTE_ORDER, asn_count);

		for (size_t i = 0; i < asn_count; i++) {
			((struct pdu_aspa *)pdu)->provider_asns[i] =
				lrtr_convert_long(target_byte_order, ((struct pdu_aspa *)pdu)->provider_asns[i]);
		}
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

static size_t rtr_size_of_aspa_pdu(const struct pdu_aspa *pdu)
{
	return sizeof(struct pdu_aspa) + sizeof(*pdu->provider_asns) * pdu->provider_count;
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
	const struct pdu_aspa *aspa_pdu = NULL;
	bool retval = false;
	size_t expected_size = 0;

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
		    ((pdu->ver == RTR_PROTOCOL_VERSION_1 || pdu->ver == RTR_PROTOCOL_VERSION_2) &&
		     (sizeof(struct pdu_end_of_data_v1_v2) == pdu->len))) {
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
		expected_size = 4 + sizeof(struct pdu_error);
		if (err_pdu->len < expected_size) {
			RTR_DBG1("PDU is too small to contain \"Length of Error Text\" field!");
			break;
		}

		// Check if the PDU really contains the error PDU
		uint32_t enc_pdu_len = ntohl(err_pdu->len_enc_pdu);

		RTR_DBG("enc_pdu_len: %u", enc_pdu_len);
		expected_size += enc_pdu_len;
		if (err_pdu->len < expected_size) {
			RTR_DBG1("PDU is too small to contain erroneous PDU!");
			break;
		}

		// Check if the the PDU really contains the error msg
		uint32_t err_msg_len = ntohl(*((uint32_t *)(err_pdu->rest + enc_pdu_len)));

		RTR_DBG("err_msg_len: %u", err_msg_len);
		expected_size += err_msg_len;
		if (err_pdu->len != expected_size) {
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
	case ASPA:
		aspa_pdu = (const struct pdu_aspa *)pdu;
		expected_size = sizeof(struct pdu_aspa);
		if (aspa_pdu->len < expected_size) {
			RTR_DBG1("PDU is too small to contain ASPA PDU!");
			break;
		}

		// Check if the PDU really contains the ASPA PDU
		uint16_t asn_count = ntohs(aspa_pdu->provider_count);

		// ASN is 4 bytes each
		expected_size += asn_count * sizeof(aspa_pdu->provider_asns[0]);
		if (aspa_pdu->len != expected_size) {
			RTR_DBG1("The PDU is malformed, because the specified size doesn't match the real PDU size.");
			break;
		}

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
		if (header.type != ERROR && rtr_socket->version > header.ver && rtr_version_supported(header.ver)) {
			RTR_DBG("First received PDU is a version %u PDU, downgrading from version %u to %u", header.ver,
				rtr_socket->version, header.ver);
			rtr_socket->version = header.ver;
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
	if (header.type == ASPA) {
		if (((struct pdu_aspa *)pdu)->ver != RTR_PROTOCOL_VERSION_2) {
			error = UNSUPPORTED_PROTOCOL_VER;
			goto error;
		}

		if (((struct pdu_aspa *)pdu)->zero != 0)
			RTR_DBG1("Warning: Zero field of received ASPA PDU doesn't contain 0");

		if (((struct pdu_aspa *)pdu)->afi_flags != 0b11)
			RTR_DBG1("Warning: AFI flags of received ASPA PDU not set to 0x03");
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
		char txt[43];

		snprintf(txt, sizeof(txt), "PDU too big, max. PDU size is: %lu bytes", RTR_MAX_PDU_LEN);
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

__attribute__((always_inline)) inline void rtr_aspa_pdu_2_aspa_operation(struct pdu_aspa *pdu,
									 struct aspa_update_operation *op)
{
	op->type = (pdu->flags & 1) == 1 ? ASPA_ADD : ASPA_REMOVE;
	op->is_no_op = false;
	op->record.customer_asn = pdu->customer_asn;
	op->record.provider_count = (op->type == ASPA_ADD) ? pdu->provider_count : 0;
	op->record.provider_asns = (op->type == ASPA_ADD) ? pdu->provider_asns : NULL;
}

/**
 * @brief Removes all prefixes from the @p pfx_table with flag field == ADD, adds all prefixes
 * to the @p pfx_table with if flag field == REMOVE.
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

/**
 * @brief Removes all prefixes from multiple PDUs from the @p pfx_table with flag field == ADD, adds all prefixes
 * to the @p pfx_table if flag field == REMOVE.
 */
static int rtr_undo_update_pfx_table_batch(struct rtr_socket *rtr_socket, struct pfx_table *pfx_table,
					   struct pdu_ipv4 *ipv4_pdus, size_t ipv4_pdu_count,
					   struct pdu_ipv6 *ipv6_pdus, size_t ipv6_pdu_count)
{
	for (size_t i = 0; i < ipv4_pdu_count; i++) {
		int res = rtr_undo_update_pfx_table(rtr_socket, pfx_table, &(ipv4_pdus[i]));

		if (res == RTR_ERROR || res == PFX_ERROR) {
			// Undo failed, cannot recover, remove all records associated with the socket instead
			RTR_DBG1(
				"Couldn't undo all update operations from failed data synchronisation: Purging all prefix records");
			pfx_table_src_remove(pfx_table, rtr_socket);
			return RTR_ERROR;
		}
	}

	for (size_t i = 0; i < ipv6_pdu_count; i++) {
		int res = rtr_undo_update_pfx_table(rtr_socket, pfx_table, &(ipv6_pdus[i]));

		if (res == RTR_ERROR || res == PFX_ERROR) {
			// Undo failed, cannot recover, remove all records associated with the socket instead
			RTR_DBG1(
				"Couldn't undo all update operations from failed data synchronisation: Purging all prefix records");
			pfx_table_src_remove(pfx_table, rtr_socket);
			return RTR_ERROR;
		}
	}
	return RTR_SUCCESS;
}

/**
 * @brief Removes router key from the @p spki_table with flag field == ADD, adds router keys
 * to the @p spki_table if flag field == REMOVE.
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

/**
 * @brief Removes router key from multiple PDUs from the @p spki_table with flag field == ADD, adds router keys
 * to the @p spki_table if flag field == REMOVE.
 */
static int rtr_undo_update_spki_table_batch(struct rtr_socket *rtr_socket, struct spki_table *spki_table,
					    struct pdu_router_key *pdus, size_t pdu_count)
{
	for (size_t i = 0; i < pdu_count; i++) {
		int res = rtr_undo_update_spki_table(rtr_socket, spki_table, &(pdus[i]));

		if (res == RTR_ERROR || res == SPKI_ERROR) {
			// Undo failed, cannot recover, remove all records associated with the socket instead
			RTR_DBG1(
				"Couldn't undo all update operations from failed data synchronisation: Purging all SPKI records");
			spki_table_src_remove(spki_table, rtr_socket);
			return RTR_ERROR;
		}
	}
	return RTR_SUCCESS;
}

/**
 * @brief Appends the Prefix PDU pdu to ary.
 *
 * @return @c RTR_SUCCESS On success
 * @return @c RTR_ERROR On realloc failure
 * @attention ary is not freed in this case, because it might contain data that is still needed
 */
static int rtr_store_prefix_pdu(struct rtr_socket *rtr_socket, const void *pdu, const unsigned int pdu_size, void **ary,
				unsigned int *ind, unsigned int *size)
{
	const enum pdu_type type = rtr_get_pdu_type(pdu);

	assert(type == IPV4_PREFIX || type == IPV6_PREFIX);
	if (*ind >= *size) {
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

/**
 * @brief Appends the router key to @p ary.
 *
 * @return @c RTR_SUCCESS On success
 * @return @c RTR_ERROR On realloc failure
 * @attention ary is not freed in this case, because it might contain data that is still needed
 */
static int rtr_store_router_key_pdu(struct rtr_socket *rtr_socket, const void *pdu, const unsigned int pdu_size,
				    struct pdu_router_key **ary, unsigned int *ind, unsigned int *size)
{
	assert(rtr_get_pdu_type(pdu) == ROUTER_KEY);

	if (*ind >= *size) {
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

/**
 * @brief Stores the ASPA pdu's contents in an ASPA operation in the given operations array.
 *
 * @return @c RTR_SUCCESS On success
 * @return @c RTR_ERROR On realloc failure
 * @attention @c pdu_array not freed in this case, because it might contain data that is still needed
 */
static int rtr_store_aspa_pdu(struct rtr_socket *rtr_socket, const struct pdu_aspa *pdu, struct pdu_aspa ***array,
			      size_t *count, size_t *capacity)
{
	if (*count >= *capacity) {
		*capacity += TEMPORARY_PDU_STORE_INCREMENT_VALUE;
		void *tmp = lrtr_realloc(*array, *capacity * sizeof(struct pdu_aspa *));

		if (!tmp) {
			const char txt[] = "Realloc failed";

			RTR_DBG("%s", txt);
			rtr_send_error_pdu_from_host(rtr_socket, NULL, 0, INTERNAL_ERROR, txt, sizeof(txt));
			rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
			return RTR_ERROR;
		}
		*array = tmp;
	}
	size_t pdu_size = rtr_size_of_aspa_pdu(pdu);
	struct pdu_aspa *copy = lrtr_malloc(pdu_size);

	if (!copy) {
		const char txt[] = "Malloc failed";

		RTR_DBG("%s", txt);
		rtr_send_error_pdu_from_host(rtr_socket, NULL, 0, INTERNAL_ERROR, txt, sizeof(txt));
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		return RTR_ERROR;
	}

	memcpy(copy, pdu, pdu_size);
	*(*array + *count) = copy;
	(*count)++;
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

	if (((struct pdu_ipv4 *)pdu)->flags == 1) { // add record
		rtval = pfx_table_add(pfx_table, &pfxr);
	} else if (((struct pdu_ipv4 *)pdu)->flags == 0) { // remove record
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

#if ASPA_UPDATE_MECHANISM == ASPA_SWAP_IN
static int rtr_compute_update_aspa_table(struct rtr_socket *rtr_socket, struct aspa_table *aspa_table,
					 struct pdu_aspa **aspa_pdus, size_t pdu_count, struct aspa_update **update)
{
	// Fail hard in debug builds.
	assert(rtr_socket);
	assert(aspa_table);
	assert(update);

	if (!update || (pdu_count > 0 && !aspa_pdus))
		return RTR_ERROR;

	if (pdu_count == 0)
		return RTR_SUCCESS;

	struct aspa_update_operation *operations = lrtr_malloc(sizeof(struct aspa_update_operation) * pdu_count);

	if (!operations) {
		const char txt[] = "Malloc failed";

		RTR_DBG("%s", txt);
		rtr_send_error_pdu_from_host(rtr_socket, NULL, 0, INTERNAL_ERROR, txt, sizeof(txt));
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		return RTR_ERROR;
	}

	for (size_t i = 0; i < pdu_count; i++) {
		rtr_aspa_pdu_2_aspa_operation(aspa_pdus[i], &operations[i]);
		operations[i].index = i;
	}

	enum aspa_status res = aspa_table_update_swap_in_compute(aspa_table, rtr_socket, operations, pdu_count, update);

	if (*update && (*update)->failed_operation) {
		struct pdu_aspa *pdu = aspa_pdus[(*update)->failed_operation->index];
		size_t pdu_size = rtr_size_of_aspa_pdu(pdu);

		if (res == ASPA_DUPLICATE_RECORD) {
			RTR_DBG("Duplicate Announcement for ASPA customer ASN: %u received", pdu->customer_asn);
			rtr_send_error_pdu_from_host(rtr_socket, pdu, pdu_size, DUPLICATE_ANNOUNCEMENT, NULL, 0);
			rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
			return RTR_ERROR;
		} else if (res == ASPA_RECORD_NOT_FOUND) {
			RTR_DBG("Withdrawal of unknown ASPA customer ASN: %u", pdu->customer_asn);
			rtr_send_error_pdu_from_host(rtr_socket, pdu, pdu_size, WITHDRAWAL_OF_UNKNOWN_RECORD, NULL, 0);
			rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
			return RTR_ERROR;
		}
	} else if (res != ASPA_SUCCESS) {
		const char txt[] = "aspa_table Error";

		RTR_DBG("%s", txt);
		rtr_send_error_pdu_from_host(rtr_socket, NULL, 0, INTERNAL_ERROR, txt, sizeof(txt));
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		return RTR_ERROR;
	}

	RTR_DBG1("ASPA update computed");
	return RTR_SUCCESS;
}
#endif

#if ASPA_UPDATE_MECHANISM == ASPA_IN_PLACE
/**
 * @brief Undoes changes made to the given @p aspa_table based on an array of @c aspa_update_operation s.
 */
static int rtr_undo_update_aspa_table(struct rtr_socket *rtr_socket, struct aspa_table *aspa_table,
				      struct aspa_update_operation *operations, size_t count,
				      struct aspa_update_operation *failed_operation)
{
	enum aspa_status res =
		aspa_table_update_in_place_undo(aspa_table, rtr_socket, operations, count, failed_operation);

	if (res == ASPA_SUCCESS) {
		return RTR_SUCCESS;
	}
	// Undo failed, cannot recover, remove all records associated with the socket instead
	RTR_DBG1("Couldn't undo all update operations from failed data synchronisation: Purging all ASPA records");
	aspa_table_src_remove(aspa_table, rtr_socket, true);
	return RTR_ERROR;
}

static int rtr_update_aspa_table(struct rtr_socket *rtr_socket, struct aspa_table *aspa_table,
				 struct pdu_aspa **aspa_pdus, size_t pdu_count, struct aspa_update_operation **ops,
				 struct aspa_update_operation **failed_operation)
{
	// Fail hard in debug builds.
	assert(rtr_socket);
	assert(aspa_table);
	assert(failed_operation);
	assert(ops);

	if (!ops || !failed_operation || (pdu_count > 0 && !aspa_pdus))
		return RTR_ERROR;

	if (!aspa_pdus && pdu_count == 0)
		return RTR_SUCCESS;

	struct aspa_update_operation *operations = lrtr_malloc(sizeof(struct aspa_update_operation) * pdu_count);

	if (!operations) {
		const char txt[] = "Malloc failed";

		RTR_DBG("%s", txt);
		rtr_send_error_pdu_from_host(rtr_socket, NULL, 0, INTERNAL_ERROR, txt, sizeof(txt));
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		return RTR_ERROR;
	}

	*ops = operations;

	for (size_t i = 0; i < pdu_count; i++) {
		rtr_aspa_pdu_2_aspa_operation(aspa_pdus[i], &operations[i]);
		operations[i].index = i;
	}

	enum aspa_status res =
		aspa_table_update_in_place(aspa_table, rtr_socket, operations, pdu_count, failed_operation);

	if (*failed_operation) {
		struct pdu_aspa *pdu = aspa_pdus[(*failed_operation)->index];
		size_t pdu_size = rtr_size_of_aspa_pdu(pdu);

		if (res == ASPA_DUPLICATE_RECORD) {
			RTR_DBG("Duplicate Announcement for ASPA customer ASN: %u received", pdu->customer_asn);
			rtr_send_error_pdu_from_host(rtr_socket, pdu, pdu_size, DUPLICATE_ANNOUNCEMENT, NULL, 0);
			rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
			return RTR_ERROR;
		} else if (res == ASPA_RECORD_NOT_FOUND) {
			RTR_DBG("Withdrawal of unknown ASPA customer ASN: %u", pdu->customer_asn);
			rtr_send_error_pdu_from_host(rtr_socket, pdu, pdu_size, WITHDRAWAL_OF_UNKNOWN_RECORD, NULL, 0);
			rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
			return RTR_ERROR;
		}
	} else if (res != ASPA_SUCCESS) {
		const char txt[] = "aspa_table Error";

		RTR_DBG("%s", txt);
		rtr_send_error_pdu_from_host(rtr_socket, NULL, 0, INTERNAL_ERROR, txt, sizeof(txt));
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		return RTR_ERROR;
	}

	return RTR_SUCCESS;
}
#endif

static int rtr_sync_update_tables(struct rtr_socket *rtr_socket, struct pfx_table *pfx_table,
				  struct spki_table *spki_table, struct aspa_table *aspa_table,
				  struct pdu_ipv4 *ipv4_pdus, const unsigned int ipv4_pdu_count,
				  struct pdu_ipv6 *ipv6_pdus, const unsigned int ipv6_pdu_count,
				  struct pdu_router_key *router_key_pdus, const unsigned int router_key_pdu_count,
				  struct pdu_aspa **aspa_pdus, const size_t aspa_pdu_count,
				  struct pdu_end_of_data_v0 *eod_pdu)
{
	bool proceed = true;
	bool undo_failed = false;
#if ASPA_UPDATE_MECHANISM == ASPA_SWAP_IN
	struct aspa_update *aspa_update = NULL;
#elif ASPA_UPDATE_MECHANISM == ASPA_IN_PLACE
	struct aspa_update_operation *aspa_failed_operation = NULL;
	struct aspa_update_operation *aspa_operations = NULL;
#else
#error "Invalid ASPA_UPDATE_MECHANISM value."
#endif /* ASPA_UPDATE_MECHANISM */

	// add all IPv4 prefix pdu to the pfx_table
	for (size_t i = 0; i < ipv4_pdu_count; i++) {
		if (rtr_update_pfx_table(rtr_socket, pfx_table, &(ipv4_pdus[i])) == RTR_ERROR) {
			RTR_DBG1("error while updating v4 prefixes");
			proceed = false;

			if (rtr_undo_update_pfx_table_batch(rtr_socket, pfx_table, ipv4_pdus, i, ipv6_pdus, 0) ==
			    RTR_ERROR)
				undo_failed = true;

			break;
		}
	}

	if (proceed) {
		RTR_DBG1("v4 prefixes added");

		// add all IPv6 prefix pdu to the pfx_table
		for (size_t i = 0; i < ipv6_pdu_count; i++) {
			if (rtr_update_pfx_table(rtr_socket, pfx_table, &(ipv6_pdus[i])) == RTR_ERROR) {
				RTR_DBG1("error while updating v6 prefixes");
				proceed = false;

				if (rtr_undo_update_pfx_table_batch(rtr_socket, pfx_table, ipv4_pdus, ipv4_pdu_count,
								    ipv6_pdus, i) == RTR_ERROR)
					undo_failed = true;

				break;
			}
		}
	}

	if (proceed) {
		RTR_DBG1("v6 prefixes added");

		// add all router key pdu to the spki_table
		for (size_t i = 0; i < router_key_pdu_count; i++) {
			if (rtr_update_spki_table(rtr_socket, spki_table, &(router_key_pdus[i])) == RTR_ERROR) {
				RTR_DBG1("error while updating spki data");
				proceed = false;

				if (rtr_undo_update_spki_table_batch(rtr_socket, spki_table, router_key_pdus, i) ==
				    RTR_ERROR)
					undo_failed = true;

				if (rtr_undo_update_pfx_table_batch(rtr_socket, pfx_table, ipv4_pdus, ipv4_pdu_count,
								    ipv6_pdus, ipv6_pdu_count) == RTR_ERROR)
					undo_failed = true;

				break;
			}
		}
	}

#if ASPA_UPDATE_MECHANISM == ASPA_SWAP_IN
	if (proceed) {
		RTR_DBG1("spki data added");

		if (rtr_compute_update_aspa_table(rtr_socket, aspa_table, aspa_pdus, aspa_pdu_count, &aspa_update) ==
		    RTR_ERROR) {
			RTR_DBG1("error while computing ASPA update");
			proceed = false;

			if (rtr_undo_update_spki_table_batch(rtr_socket, spki_table, router_key_pdus,
							     router_key_pdu_count) == RTR_ERROR)
				undo_failed = true;

			if (rtr_undo_update_pfx_table_batch(rtr_socket, pfx_table, ipv4_pdus, ipv4_pdu_count, ipv6_pdus,
							    ipv6_pdu_count) == RTR_ERROR)
				undo_failed = true;
		}
	}

	if (proceed) {
		aspa_table_update_swap_in_apply(&aspa_update);
		RTR_DBG1("ASPA data added");
	} else {
		aspa_table_update_swap_in_discard(&aspa_update);
	}

#elif ASPA_UPDATE_MECHANISM == ASPA_IN_PLACE
	if (proceed) {
		RTR_DBG1("spki data added");

		if (rtr_update_aspa_table(rtr_socket, aspa_table, aspa_pdus, aspa_pdu_count, &aspa_operations,
					  &aspa_failed_operation) == RTR_ERROR) {
			RTR_DBG1("error while updating ASPA data");
			proceed = false;

			if (rtr_undo_update_aspa_table(rtr_socket, aspa_table, aspa_operations, aspa_pdu_count,
						       aspa_failed_operation) == RTR_ERROR)
				undo_failed = true;

			if (rtr_undo_update_spki_table_batch(rtr_socket, spki_table, router_key_pdus,
							     router_key_pdu_count) == RTR_ERROR)
				undo_failed = true;

			if (rtr_undo_update_pfx_table_batch(rtr_socket, pfx_table, ipv4_pdus, ipv4_pdu_count, ipv6_pdus,
							    ipv6_pdu_count) == RTR_ERROR)
				undo_failed = true;
		}
	}

	if (proceed)
		RTR_DBG1("ASPA data added");

	aspa_table_update_in_place_cleanup(&aspa_operations, aspa_pdu_count);
	aspa_failed_operation = NULL;
#else
#error "Invalid ASPA_UPDATE_MECHANISM value."
#endif /* ASPA_UPDATE_MECHANISM */

	// An update attempted above failed
	if (!proceed) {
		if (undo_failed)
			// undo failed, so request new session
			rtr_socket->request_session_id = true;

		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		return RTR_ERROR;
	}

	rtr_socket->serial_number = eod_pdu->sn;
	RTR_DBG("Sync successful, received %u Prefix PDUs, %u Router Key PDUs, %lu ASPA PDUs, session_id: %u, SN: %u",
		(ipv4_pdu_count + ipv6_pdu_count), router_key_pdu_count, aspa_pdu_count, rtr_socket->session_id,
		rtr_socket->serial_number);

	return RTR_SUCCESS;
}

static inline int rtr_handle_eod_pdu(struct rtr_socket *rtr_socket, struct pdu_end_of_data_v0 *eod_pdu, char pdu_data[])
{
	if (eod_pdu->session_id != rtr_socket->session_id) {
		char txt[67];

		snprintf(txt, sizeof(txt), "Expected session_id: %u, received session_id. %u in EOD PDU",
			 rtr_socket->session_id, eod_pdu->session_id);
		rtr_send_error_pdu_from_host(rtr_socket, pdu_data, RTR_MAX_PDU_LEN, CORRUPT_DATA, txt, strlen(txt) + 1);
		rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
		return RTR_ERROR;
	}

	if ((eod_pdu->ver == RTR_PROTOCOL_VERSION_1 || eod_pdu->ver == RTR_PROTOCOL_VERSION_2) &&
	    rtr_socket->iv_mode != RTR_INTERVAL_MODE_IGNORE_ANY) {
		int interv_retval;

		interv_retval = rtr_check_interval_option(rtr_socket, rtr_socket->iv_mode,
							  ((struct pdu_end_of_data_v1_v2 *)pdu_data)->expire_interval,
							  RTR_INTERVAL_TYPE_EXPIRATION);

		if (interv_retval == RTR_ERROR) {
			interval_send_error_pdu(rtr_socket, pdu_data,
						((struct pdu_end_of_data_v1_v2 *)pdu_data)->expire_interval,
						RTR_EXPIRATION_MIN, RTR_EXPIRATION_MAX);
			return RTR_ERROR;
		}

		interv_retval = rtr_check_interval_option(rtr_socket, rtr_socket->iv_mode,
							  ((struct pdu_end_of_data_v1_v2 *)pdu_data)->refresh_interval,
							  RTR_INTERVAL_TYPE_REFRESH);

		if (interv_retval == RTR_ERROR) {
			interval_send_error_pdu(rtr_socket, pdu_data,
						((struct pdu_end_of_data_v1_v2 *)pdu_data)->refresh_interval,
						RTR_REFRESH_MIN, RTR_REFRESH_MAX);
			return RTR_ERROR;
		}

		interv_retval = rtr_check_interval_option(rtr_socket, rtr_socket->iv_mode,
							  ((struct pdu_end_of_data_v1_v2 *)pdu_data)->retry_interval,
							  RTR_INTERVAL_TYPE_RETRY);

		if (interv_retval == RTR_ERROR) {
			interval_send_error_pdu(rtr_socket, pdu_data,
						((struct pdu_end_of_data_v1_v2 *)pdu_data)->retry_interval,
						RTR_RETRY_MIN, RTR_RETRY_MAX);
			return RTR_ERROR;
		}

		RTR_DBG("New interval values: expire_interval:%u, refresh_interval:%u, retry_interval:%u",
			rtr_socket->expire_interval, rtr_socket->refresh_interval, rtr_socket->retry_interval);
	}
	return RTR_SUCCESS;
}

void recv_loop_cleanup(void *p)
{
	struct recv_loop_cleanup_args *args = p;

	if (!args)
		return;

	if (*args->ipv4_pdus) {
		lrtr_free(*args->ipv4_pdus);
		*args->ipv4_pdus = NULL;
	}

	if (*args->ipv6_pdus) {
		lrtr_free(*args->ipv6_pdus);
		*args->ipv6_pdus = NULL;
	}

	if (*args->router_key_pdus) {
		lrtr_free(*args->router_key_pdus);
		*args->router_key_pdus = NULL;
	}

	if (*args->aspa_pdus) {
		for (size_t i = 0; i < *args->aspa_pdu_count; i++) {
			if ((*args->aspa_pdus)[i])
				lrtr_free((*args->aspa_pdus)[i]);
		}

		lrtr_free(*args->aspa_pdus);
		*args->aspa_pdus = NULL;
		*args->aspa_pdu_count = 0;
	}
}

/* WARNING: This Function has cancelable sections*/
static int rtr_sync_receive_and_store_pdus(struct rtr_socket *rtr_socket)
{
	char pdu_data[RTR_MAX_PDU_LEN];
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

	struct pdu_aspa **aspa_pdus = NULL;
	size_t aspa_pdus_capacity = 0;
	size_t aspa_pdus_count = 0;

	int oldcancelstate;
	struct recv_loop_cleanup_args cleanup_args = {.ipv4_pdus = &ipv4_pdus,
						      .ipv6_pdus = &ipv6_pdus,
						      .router_key_pdus = &router_key_pdus,
						      .aspa_pdus = &aspa_pdus,
						      .aspa_pdu_count = &aspa_pdus_count};

	// receive LRTR_IPV4/IPV6/ASPA PDUs till EOD
	do {
		pthread_cleanup_push(recv_loop_cleanup, &cleanup_args);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldcancelstate);

		// Receive PDUs, cancellable
		retval = rtr_receive_pdu(rtr_socket, pdu_data, RTR_MAX_PDU_LEN, RTR_RECV_TIMEOUT);

		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldcancelstate);
		pthread_cleanup_pop(0);

		if (retval == TR_WOULDBLOCK) {
			rtr_change_socket_state(rtr_socket, RTR_ERROR_TRANSPORT);
			retval = RTR_ERROR;
			break;
		} else if (retval < 0) {
			retval = RTR_ERROR;
			break;
		}

		type = rtr_get_pdu_type(pdu_data);

		switch (type) {
		case SERIAL_NOTIFY:
			RTR_DBG1("Ignoring Serial Notify");
			break;

		case IPV4_PREFIX:
			// Temporarily store prefix PDU, handle later after EOD PDU has been received
			if (rtr_store_prefix_pdu(rtr_socket, pdu_data, sizeof(*ipv4_pdus), (void **)&ipv4_pdus,
						 &ipv4_pdus_nindex, &ipv4_pdus_size) == RTR_ERROR) {
				rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
				retval = RTR_ERROR;
			}
			break;

		case IPV6_PREFIX:
			// Temporarily store prefix PDU, handle later after EOD PDU has been received
			if (rtr_store_prefix_pdu(rtr_socket, pdu_data, sizeof(*ipv6_pdus), (void **)&ipv6_pdus,
						 &ipv6_pdus_nindex, &ipv6_pdus_size) == RTR_ERROR) {
				rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
				retval = RTR_ERROR;
			}
			break;

		case ROUTER_KEY:
			// Temporarily store router key PDU, handle later after EOD PDU has been received
			if (rtr_store_router_key_pdu(rtr_socket, pdu_data, sizeof(*router_key_pdus), &router_key_pdus,
						     &router_key_pdus_nindex, &router_key_pdus_size) == RTR_ERROR) {
				rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
				retval = RTR_ERROR;
			}
			break;

		case ASPA:
			// Temporarily store ASPA PDU, handle later after EOD PDU has been received
			if (rtr_store_aspa_pdu(rtr_socket, (struct pdu_aspa *)pdu_data, &aspa_pdus, &aspa_pdus_count,
					       &aspa_pdus_capacity) == RTR_ERROR) {
				rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
				retval = RTR_ERROR;
			}
			break;

		case ERROR:
			rtr_handle_error_pdu(rtr_socket, pdu_data);
			retval = RTR_ERROR;
			break;

		case EOD:
			RTR_DBG1("EOD PDU received.");
			struct pdu_end_of_data_v0 *eod_pdu = (struct pdu_end_of_data_v0 *)pdu_data;

			retval = rtr_handle_eod_pdu(rtr_socket, eod_pdu, pdu_data);
			if (retval != RTR_SUCCESS)
				break;

			if (rtr_socket->is_resetting) {
				// Use table copies instead in order to perform an atomic update
				RTR_DBG1("Reset in progress creating shadow table for atomic reset");

				struct pfx_table *pfx_shadow_table = lrtr_malloc(sizeof(struct pfx_table));

				if (!pfx_shadow_table) {
					RTR_DBG1("Memory allocation for pfx shadow table failed");
					retval = RTR_ERROR;
					goto cleanup;
				}
				pfx_table_init(pfx_shadow_table, NULL);
				if (pfx_table_copy_except_socket(rtr_socket->pfx_table, pfx_shadow_table, rtr_socket) !=
				    PFX_SUCCESS) {
					RTR_DBG1("Creation of pfx shadow table failed");
					rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
					retval = RTR_ERROR;
					goto cleanup;
				}

				struct spki_table *spki_shadow_table = lrtr_malloc(sizeof(struct spki_table));

				if (!spki_shadow_table) {
					RTR_DBG1("Memory allocation for spki shadow table failed");
					retval = RTR_ERROR;
					goto cleanup;
				}
				spki_table_init(spki_shadow_table, NULL);
				if (spki_table_copy_except_socket(rtr_socket->spki_table, spki_shadow_table,
								  rtr_socket) != SPKI_SUCCESS) {
					RTR_DBG1("Creation of spki shadow table failed");
					rtr_change_socket_state(rtr_socket, RTR_ERROR_FATAL);
					retval = RTR_ERROR;
					goto cleanup;
				}

				struct aspa_table *aspa_shadow_table = lrtr_malloc(sizeof(struct aspa_table));

				if (!aspa_shadow_table) {
					RTR_DBG1("Memory allocation for aspa shadow table failed");
					retval = RTR_ERROR;
					goto cleanup;
				}
				aspa_table_init(aspa_shadow_table, NULL);

				RTR_DBG1("Shadow tables created");

				retval = rtr_sync_update_tables(rtr_socket, pfx_shadow_table, spki_shadow_table,
								aspa_shadow_table, ipv4_pdus, ipv4_pdus_nindex,
								ipv6_pdus, ipv6_pdus_nindex, router_key_pdus,
								router_key_pdus_nindex, aspa_pdus, aspa_pdus_count,
								eod_pdu);

				if (retval == RTR_SUCCESS) {
					RTR_DBG1("Reset finished. Swapping new table in.");
					pfx_table_swap(rtr_socket->pfx_table, pfx_shadow_table);
					spki_table_swap(rtr_socket->spki_table, spki_shadow_table);

					// No need to notify shadow table's clients as it hasn't any.
					RTR_DBG1("Replacing ASPA records");
					aspa_table_src_replace(rtr_socket->aspa_table, aspa_shadow_table, rtr_socket,
							       !!rtr_socket->aspa_table->update_fp, false);

					if (rtr_socket->pfx_table->update_fp) {
						RTR_DBG1("Calculating and notifying pfx diff");
						pfx_table_notify_diff(rtr_socket->pfx_table, pfx_shadow_table,
								      rtr_socket);
					} else {
						RTR_DBG1("No pfx update callback. Skipping diff");
					}

					if (rtr_socket->spki_table->update_fp) {
						RTR_DBG1("Calculating and notifying spki diff");
						spki_table_notify_diff(rtr_socket->spki_table, spki_shadow_table,
								       rtr_socket);
					} else {
						RTR_DBG1("No spki update callback. Skipping diff");
					}
				}

			cleanup:
				RTR_DBG1("Freeing shadow tables.");
				if (pfx_shadow_table) {
					pfx_table_free_without_notify(pfx_shadow_table);
					lrtr_free(pfx_shadow_table);
				}

				if (spki_shadow_table) {
					spki_table_free_without_notify(spki_shadow_table);
					lrtr_free(spki_shadow_table);
				}

				if (aspa_shadow_table) {
					aspa_table_free(aspa_shadow_table, false);
					lrtr_free(aspa_shadow_table);
				}

				rtr_socket->is_resetting = false;
			} else {
				retval = rtr_sync_update_tables(rtr_socket, rtr_socket->pfx_table,
								rtr_socket->spki_table, rtr_socket->aspa_table,
								ipv4_pdus, ipv4_pdus_nindex, ipv6_pdus,
								ipv6_pdus_nindex, router_key_pdus,
								router_key_pdus_nindex, aspa_pdus, aspa_pdus_count,
								eod_pdu);
			}

			break;

		default:
			RTR_DBG("Received unexpected PDU (Type: %u)", ((struct pdu_header *)pdu_data)->type);
			const char txt[] = "Unexpected PDU received during data synchronisation";

			rtr_send_error_pdu_from_host(rtr_socket, pdu_data, sizeof(struct pdu_header), CORRUPT_DATA, txt,
						     sizeof(txt));
			retval = RTR_ERROR;
			break;
		}
	} while (type != EOD && retval != RTR_ERROR);

	recv_loop_cleanup(&cleanup_args);
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
