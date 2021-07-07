/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef RTR_BGPSEC_UTILS_PRIVATE_H
#define RTR_BGPSEC_UTILS_PRIVATE_H

#include <openssl/x509.h>
#include <string.h>
#include <arpa/inet.h>

#include "rtrlib/bgpsec/bgpsec.h"
#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/rtrlib_export_private.h"
#include "rtrlib/lib/log_private.h"

#define BGPSEC_DBG(fmt, ...) lrtr_dbg("BGPSEC: " fmt, ## __VA_ARGS__)
#define BGPSEC_DBG1(a) lrtr_dbg("BGPSEC: " a)

/** The length of a rtr_secure_path_seg without the next pointer:
 * pcount(1) + flags(1) + asn(4)
 */
#define SECURE_PATH_SEG_SIZE 6

/** Control flag, validation and signing procedures for aligning data differs.
 */
enum align_type {
	VALIDATION,
	SIGNING,
};

/* Forward declaration of stream to make it opaque. */
struct stream;

/* Initialize a stream of size bytes */
struct stream *init_stream(uint16_t size);

/* Copy a stream s and return the copy */
struct stream *copy_stream(struct stream *s);

/* Free stream s */
void free_stream(struct stream *s);

/* Write len bytes from data to stream s */
void write_stream(struct stream *s, void *data, uint16_t len);

/* Get the start position pointer of stream s */
uint8_t *get_stream_start(struct stream *s);

/* Get the size of the storable data of stream s */
uint16_t get_stream_size(struct stream *s);

/* Read one byte from stream s */
uint8_t read_stream(struct stream *s);

/* Read len bytes from stream s and write them to buff */
void read_n_bytes_stream(uint8_t *buff, struct stream *s, uint16_t len);

/* Read len bytes from stream s, starting from position start and write
 * the result to buff.
 */
void read_stream_at(uint8_t *buff, struct stream *s, uint16_t start,
		    uint16_t len);

/* Calculate the reqired size for a stream, so that all information from data
 * fit into it. type controls, if it is for validation or signing purposes.
 */
unsigned int req_stream_size(const struct rtr_bgpsec *data,
			     enum align_type type);

/* Get the length in bytes for a all signature segments */
int get_sig_seg_size(
		const struct rtr_signature_seg *sig_segs,
		enum align_type type);

/* Check, if there is at least one router key for each SKI from sig_segs. */
int check_router_keys(
		const struct rtr_signature_seg *sig_segs,
		struct spki_table *table);

/* Store the string representation of a BGPsec_PATH segment in buffer. */
int bgpsec_segment_to_str(
		char *buffer,
		struct rtr_signature_seg *sig_seg,
		struct rtr_secure_path_seg *sec_path);

/* Store the hex-string representation of a byte sequence in buffer. */
int byte_sequence_to_str(
		char *buffer,
		uint8_t *bytes,
		unsigned int bytes_len,
		unsigned int tabstops);

/* Takes a binary encoded SKI and stores it in ski_str as a human readable
 * hex string.
 */
void ski_to_char(char *ski_str, uint8_t *ski);

/* Align the BGPsec data as a byte sequence and store it in stream s. type
 * controls, if the alignment is for validation or signing.
 */
int align_byte_sequence(
		const struct rtr_bgpsec *data,
		struct stream *s,
		enum align_type type);

/* Hash a byte sequence and store it in result_buffer. */
int hash_byte_sequence(
		uint8_t *bytes,
		unsigned int bytes_len,
		uint8_t alg_suite_id,
		unsigned char **result_buffer);

/* Validate a signature sig. */
int validate_signature(
		const unsigned char *hash,
		const struct rtr_signature_seg *sig,
		struct spki_record *record);

/* Load a binary private key bytes_key and store it in the openssl EC_KEY
 * priv_key.
 */
int load_private_key(EC_KEY **priv_key, uint8_t *bytes_key);

/* Load a binary public key spki and store it in the openssl EC_KEY
 * pub_key.
 */
int load_public_key(EC_KEY **pub_key, uint8_t *spki);

/* Sign a byte sequence, depending on the algorithm suite. The signature and
 * its length are stored in new_signature.
 */
int sign_byte_sequence(uint8_t *hash_result,
		       EC_KEY *priv_key,
		       uint8_t alg,
		       struct rtr_signature_seg *new_signature);

/* Check, if all elements of a SKI are 0. */
int ski_is_empty(uint8_t *ski);

#endif
