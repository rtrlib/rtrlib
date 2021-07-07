/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

struct rtr_bgpsec *setup_bgpsec(void);

int __wrap_check_router_keys(const struct rtr_signature_seg *sig_segs,
			     struct spki_table *table);

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

int __wrap_validate_signature(
		const unsigned char *hash,
		const struct rtr_signature_seg *sig,
		struct spki_record *record);

int __wrap_spki_table_search_by_ski(struct spki_table *spki_table,
				    uint8_t *ski,
				    struct spki_record **result,
				    unsigned int *result_size);
