/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

struct rtr_bgpsec *setup_bgpsec(void);

int __wrap_load_private_key(EC_KEY **priv_key, uint8_t *bytes_key);

int __wrap_ECDSA_size(const EC_KEY *key);

int __wrap_align_byte_sequence(
		const struct rtr_bgpsec *data,
		struct stream *s,
		enum align_type type);

unsigned int __wrap_req_stream_size(const struct rtr_bgpsec *data,
				    enum align_type type);

int __wrap_hash_byte_sequence(
		uint8_t *bytes,
		unsigned int bytes_len,
		uint8_t alg_suite_id,
		unsigned char **hash_result);

int __wrap_sign_byte_sequence(uint8_t *hash_result,
			      EC_KEY *priv_key,
			      uint8_t alg,
			      struct rtr_signature_seg *new_signature);
