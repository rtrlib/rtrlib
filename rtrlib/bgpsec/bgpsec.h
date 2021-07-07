/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

/**
 * @defgroup mod_bgpsec_h BGPsec AS path validation
 * @brief BGPsec allows for validation of the BGPsec_PATH attribute of a
 * BGPsec update.
 * @{
 */

#ifndef RTR_BGPSEC_H
#define RTR_BGPSEC_H

#include <stdint.h>

#include "rtrlib/spki/spkitable.h"
#include "rtrlib/lib/ip.h"

/**
 * @brief All supported algorithm suites.
 */
enum rtr_bgpsec_algorithm_suites {
	/** Algorithm suite 1 */
	RTR_BGPSEC_ALGORITHM_SUITE_1 = 1,
};

/**
 * @brief Status codes for various cases.
 */
enum rtr_bgpsec_rtvals {
	/** At least one signature is not valid. */
	RTR_BGPSEC_NOT_VALID = 2,
	/** All signatures are valid. */
	RTR_BGPSEC_VALID = 1,
	/** An operation was successful. */
	RTR_BGPSEC_SUCCESS = 0,
	/** An operation was not sucessful. */
	RTR_BGPSEC_ERROR = -1,
	/** The public key could not be loaded. */
	RTR_BGPSEC_LOAD_PUB_KEY_ERROR = -2,
	/** The private key could not be loaded. */
	RTR_BGPSEC_LOAD_PRIV_KEY_ERROR = -3,
	/** The SKI for a router key was not found. */
	RTR_BGPSEC_ROUTER_KEY_NOT_FOUND = -4,
	/** An error during signing occurred. */
	RTR_BGPSEC_SIGNING_ERROR = -5,
	/** The specified algorithm suite is not supported by RTRlib. */
	RTR_BGPSEC_UNSUPPORTED_ALGORITHM_SUITE = -6,
	/** The specified AFI is not supported by BGPsec. */
	RTR_BGPSEC_UNSUPPORTED_AFI = -7,
	/** The amount of signature and secure path segments are not equal. */
	RTR_BGPSEC_WRONG_SEGMENT_AMOUNT = -8,
	/** There is data missing for validation or signing. */
	RTR_BGPSEC_MISSING_DATA = -9,
};

/**
 * @brief A single Secure Path Segment.
 * @param next Reference to the next Secure Path Segment (do not edit manually).
 * @param pcount The pCount field of the segment.
 * @param flags The flags field of the segment.
 * @param asn The ASN of the Segment.
 */
struct rtr_secure_path_seg {
	/** Reference to the next Secure Path Segment (do not edit manually). */
	struct rtr_secure_path_seg *next;
	uint8_t pcount;
	uint8_t flags;
	uint32_t asn;
};

/**
 * @brief A single Signature Segment.
 * @param next Reference to the next Signature Segment (do not edit manually).
 * @param ski The SKI of the segment.
 * @param sig_len The length in octets of the signature field.
 * @param signature The signature of the segment.
 */
struct rtr_signature_seg {
	struct rtr_signature_seg *next;
	uint8_t ski[SKI_SIZE];
	uint16_t sig_len;
	/** The signature of the segment. */
	uint8_t *signature;
};

/**
 * @brief This struct contains the Network Layer Reachability Information
 *	  (NLRI). The NLRI consists of a prefix and its length.
 * @param prefix_len The length of the prefix in bits.
 * @param prefix The struct that contains the IPv4/6 address. Trailing bits
 *		 must be set to 0.
 */
struct rtr_bgpsec_nlri {
	uint8_t prefix_len;
	struct lrtr_ip_addr prefix;
};

/**
 * @brief The data that is passed to the rtr_mgr_bgpsec_validate_as_path function.
 * @param alg The Algorithm Suite Identifier.
 * @param safi The Subsequent Address Family Identifier.
 * @param afi The Address Family Identifier.
 * @param my_as The AS that is currently performing validation (you).
 * @param target_as The AS where the update should be sent to.
 * @param sigs_len Count of Signature Segments (do not edit manually).
 * @param path_len Count of Secure Path Segments (do not edit manually).
 * @param nlri The Network Layer Reachability Information.
 * @param sigs Reference to the Signature Segments.
 * @param path Reference to the Secure Path Segments.
 */
struct rtr_bgpsec {
	uint8_t alg;
	uint8_t safi;
	uint16_t afi;
	uint32_t my_as;
	uint32_t target_as;
	/** Count of Signature Segments (do not edit manually). */
	uint16_t sigs_len;
	/** Count of Secure Path Segments (do not edit manually). */
	uint8_t path_len;
	struct rtr_bgpsec_nlri nlri;
	/** Reference to the Signature Segments. */
	struct rtr_signature_seg *sigs;
	/** Reference to the Secure Path Segments. */
	struct rtr_secure_path_seg *path;
};
#endif
/* @} */
