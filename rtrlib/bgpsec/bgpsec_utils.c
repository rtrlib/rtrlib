/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include <stdio.h>

#include "rtrlib/bgpsec/bgpsec_utils_private.h"
#include "rtrlib/spki/spkitable_private.h"

/** The string length of a SKI, including spaces. */
#define SKI_STR_LEN 61

/** The total length of a private key in bytes. */
#define PRIVATE_KEY_LENGTH 121L

/** Macro to get the NLRI length in bytes. */
#define NLRI_BYTE_LEN(data)	(((data)->nlri.prefix_len + 7) / 8)

struct stream {
	uint16_t size;
	uint8_t *stream;
	const uint8_t *start;
	uint16_t w_head;
	uint16_t r_head;
};

struct stream *init_stream(uint16_t size)
{
	struct stream *s = lrtr_calloc(sizeof(struct stream), 1);

	s->stream = lrtr_calloc(size, 1);
	s->start = s->stream;
	s->size = size;
	s->w_head = 0;
	s->r_head = 0;
	return s;
}

/* cppcheck-suppress unusedFunction */
struct stream *copy_stream(struct stream *s)
{
	struct stream *cpy = init_stream(s->size);

	memcpy(cpy->stream, s->stream, s->size);
	cpy->w_head = s->w_head;
	cpy->r_head = s->r_head;
	return cpy;
}

void free_stream(struct stream *s)
{
	lrtr_free((uint8_t *)s->start);
	lrtr_free(s);
}

void write_stream(struct stream *s, void *data, uint16_t len)
{
	memcpy(s->stream + s->w_head, data, len);
	s->w_head += len;
}

/* cppcheck-suppress unusedFunction */
uint8_t read_stream(struct stream *s)
{
	//TODO: add condition if r_head > len
	uint8_t c = *(s->start + s->r_head);

	s->r_head++;
	return c;
}

//TODO: return how many bytes where actually read, in case len > s->len.
/* cppcheck-suppress unusedFunction */
void read_n_bytes_stream(uint8_t *buff, struct stream *s, uint16_t len)
{
	memcpy(buff, s->start + s->r_head, len);
	s->r_head += len;
}

void read_stream_at(uint8_t *buff, struct stream *s, uint16_t start,
		    uint16_t len)
{
	if (start + len > s->size)
		len = s->size - start;
	memcpy(buff, s->start + start, len);
}

uint8_t *get_stream_start(struct stream *s)
{
	return (uint8_t *)s->start;
}

uint16_t get_stream_size(struct stream *s)
{
	return s->size;
}

unsigned int req_stream_size(const struct rtr_bgpsec *data,
			     enum align_type type)
{
	unsigned int sig_segs_size = get_sig_seg_size(data->sigs,
						      type);
	//TODO: include NLRI_BYTE_LEN macro somehow...
	uint8_t nlri_len_b = (data->nlri.prefix_len + 7) / 8; // bits to bytes
	unsigned int bytes_len = 9 // alg(1) + afi(2) + safi(1) + asn(4) + prefix_len(1)
		   + nlri_len_b
		   + sig_segs_size
		   + (SECURE_PATH_SEG_SIZE * data->path_len);
	return bytes_len;
}

int get_sig_seg_size(
		const struct rtr_signature_seg *sig_segs,
		enum align_type type)
{
	unsigned int sig_segs_size = 0;

	if (!sig_segs)
		return 0;

	/* Iterate over all signature segments and add the calculated
	 * length to sig_segs_size. Return the sum at the end.
	 */
	const struct rtr_signature_seg *curr = sig_segs;

	if (type == VALIDATION)
		curr = curr->next;

	while (curr) {
		sig_segs_size += curr->sig_len +
				 sizeof(curr->sig_len) +
				 SKI_SIZE;
		curr = curr->next;
	}

	return sig_segs_size;
}

int check_router_keys(
		const struct rtr_signature_seg *sig_segs,
		struct spki_table *table)
{
	struct spki_record *tmp_key = NULL;
	const struct rtr_signature_seg *curr = sig_segs;

	while (curr) {
		unsigned int router_keys_len = 0;
		enum spki_rtvals spki_retval = spki_table_search_by_ski(
							table,
							(uint8_t *)curr->ski,
							&tmp_key,
							&router_keys_len);
		if (spki_retval == SPKI_ERROR)
			return RTR_BGPSEC_ERROR;

		/* Return an error, if a router key was not found. */
		if (router_keys_len == 0) {
			char ski_str[SKI_STR_LEN] = {0};

			ski_to_char(ski_str, (uint8_t *)curr->ski);
			BGPSEC_DBG(
				"ERROR: Could not find router key for SKI: %s",
				ski_str);
			return RTR_BGPSEC_ROUTER_KEY_NOT_FOUND;
		}
		lrtr_free(tmp_key);
		curr = curr->next;
	}

	return RTR_BGPSEC_SUCCESS;
}

int byte_sequence_to_str(
		char *buffer,
		uint8_t *bytes,
		unsigned int bytes_len,
		unsigned int tabstops)
{
	unsigned int bytes_printed = 1;

	for (unsigned int j = 0; j < tabstops; j++)
		buffer += sprintf(buffer, "\t");

	for (unsigned int i = 0; i < bytes_len; i++, bytes_printed++) {
		buffer += sprintf(buffer, "%02X ", bytes[i]);

		/* Only print 16 bytes in a single line. */
		if (bytes_printed % 16 == 0) {
			buffer += sprintf(buffer, "\n");
			for (unsigned int j = 0; j < tabstops; j++)
				buffer += sprintf(buffer, "\t");
		}
	}

	/* TODO: that's ugly.
	 * If there was no new line printed at the end of the for loop,
	 * print an extra new line.
	 */
	if (bytes_len % 16 != 0)
		buffer += sprintf(buffer, "\n");
	sprintf(buffer, "\n");
	return RTR_BGPSEC_SUCCESS;
}

/* cppcheck-suppress unusedFunction */
int bgpsec_segment_to_str(
		char *buffer,
		struct rtr_signature_seg *sig_seg,
		struct rtr_secure_path_seg *sec_path)
{
	char byte_buffer[256] = {'\0'};

	buffer += sprintf(buffer, "++++++++++++++++++++++++++++++++++++++++\n");
	buffer += sprintf(buffer, "Signature Segment:\n");
	buffer += sprintf(buffer, "\tSKI:\n");

	byte_sequence_to_str(byte_buffer, sig_seg->ski, SKI_SIZE, 2);
	buffer += sprintf(buffer, "%s\n", byte_buffer);

	buffer += sprintf(buffer, "\tLength: %d\n", sig_seg->sig_len);
	buffer += sprintf(buffer, "\tSignature:\n");

	memset(byte_buffer, 0, sizeof(byte_buffer));
	byte_sequence_to_str(byte_buffer, sig_seg->signature, sig_seg->sig_len,
			     2);
	buffer += sprintf(buffer, "%s\n", byte_buffer);

	buffer += sprintf(buffer, "----------------------------------------\n");
	buffer += sprintf(buffer, "Secure_Path Segment:\n"
			"\tpCount: %d\n"
			"\tFlags: %d\n"
			"\tAS number: %d\n",
			sec_path->pcount,
			sec_path->flags,
			sec_path->asn);
	buffer += sprintf(buffer, "++++++++++++++++++++++++++++++++++++++++\n");
	buffer += sprintf(buffer, "\n");
	*buffer = '\0';

	return RTR_BGPSEC_SUCCESS;
}

void ski_to_char(char *ski_str, uint8_t *ski)
{
	for (unsigned int i = 0; i < SKI_SIZE; i++)
		sprintf(&ski_str[i * 3], "%02X ", ski[i]);
}

/*
 * One step in validating a BGPsec signature is hashing some of the
 * content of the BGPsec update. These information must be aligned
 * in a specific order before they are hashed. The function below
 * handles this alignment. Since every byte affects the resulting
 * hash, padding or trailing bytes must not exist in the byte
 * sequence.
 */
int align_byte_sequence(
		const struct rtr_bgpsec *data,
		struct stream *s,
		enum align_type type)
{
	/* Variables used for network-to-host-order transformation. */
	uint32_t asn = 0;
	uint16_t afi = 0;

	/* Temp secure path and signature segments to prevent any
	 * alteration of the original data.
	 */
	struct rtr_secure_path_seg *tmp_sec = NULL;
	struct rtr_signature_seg *tmp_sig = NULL;

	/* The data alignment begins here, starting with the target ASN. */
	asn = ntohl(data->target_as);
	write_stream(s, &asn, sizeof(asn));

	/* Depending on whether we are dealing with alignment for validation
	 * or signing, the first signature segment is skipped.
	 */
	if (type == VALIDATION)
		tmp_sig = data->sigs->next;
	else
		tmp_sig = data->sigs;

	tmp_sec = data->path;

	while (tmp_sec) {
		if (tmp_sig) {
			uint16_t sig_len = ntohs(tmp_sig->sig_len);

			/* Write the signature segment data to stream. */
			write_stream(s, tmp_sig->ski, SKI_SIZE);
			write_stream(s, &sig_len, sizeof(sig_len));
			write_stream(s, tmp_sig->signature, tmp_sig->sig_len);

			tmp_sig = tmp_sig->next;
		}

		/* Write the secure path segment data to stream. */
		write_stream(s, (uint8_t *)&tmp_sec->pcount, 1);
		write_stream(s, (uint8_t *)&tmp_sec->flags, 1);

		asn = ntohl(tmp_sec->asn);
		write_stream(s, &asn, sizeof(asn));
		tmp_sec = tmp_sec->next;
	}

	/* Write the rest of the data to stream. */
	write_stream(s, (uint8_t *)&data->alg, 1);

	afi = ntohs(data->afi);
	write_stream(s, &afi, sizeof(afi));

	write_stream(s, (uint8_t *)&data->safi, 1);
	write_stream(s, (uint8_t *)&data->nlri.prefix_len, 1);

	/* Make sure we write the right IP address type by checking the AFI. */
	switch (data->nlri.prefix.ver) {
	case LRTR_IPV4:
		write_stream(s, (uint8_t *)&data->nlri.prefix.u.addr4.addr,
			     NLRI_BYTE_LEN(data));
		break;
	case LRTR_IPV6:
		write_stream(s, (uint8_t *)&data->nlri.prefix.u.addr6.addr,
			     NLRI_BYTE_LEN(data));
		break;
	default:
		/* Should not come here. */
		return RTR_BGPSEC_UNSUPPORTED_AFI;
	}

	return RTR_BGPSEC_SUCCESS;
}

int validate_signature(
		const unsigned char *hash,
		const struct rtr_signature_seg *sig,
		struct spki_record *record)
{
	int status = 0;
	enum rtr_bgpsec_rtvals retval;

	EC_KEY *pub_key = NULL;

	/* Load the contents of the spki buffer into the
	 * OpenSSL public key.
	 */
	retval = load_public_key(&pub_key, record->spki);

	if (retval != RTR_BGPSEC_SUCCESS) {
		/* The output string looks like this: "XX XX XX XX"
		 * where XX is a single byte. Including the spaces,
		 * we need to multiply by 3. The plus 1 is for the string
		 * terminator.
		 */
		char ski_str[(SKI_SIZE * 3) + 1] = {'\0'};

		ski_to_char(ski_str, record->ski);
		BGPSEC_DBG("WARNING: Invalid public key for SKI: %s", ski_str);
		retval = RTR_BGPSEC_ERROR;
		goto err;
	}

	/* The OpenSSL validation function to validate the signature. */
	status = ECDSA_verify(
			0,
			hash,
			SHA256_DIGEST_LENGTH,
			sig->signature,
			sig->sig_len,
			pub_key);

	switch (status) {
	case -1:
		BGPSEC_DBG1("ERROR: Failed to verify EC Signature");
		retval = RTR_BGPSEC_ERROR;
		break;
	case 0:
		BGPSEC_DBG1("Validation result of signature: invalid");
		retval = RTR_BGPSEC_NOT_VALID;
		break;
	case 1:
		BGPSEC_DBG1("Validation result of signature: valid");
		retval = RTR_BGPSEC_VALID;
		break;
	}

	EC_KEY_free(pub_key);

err:
	return retval;
}

int load_public_key(EC_KEY **pub_key, uint8_t *spki)
{
	char *p = NULL;
	int status = 0;

	p = (char *)spki;
	*pub_key = NULL;

	/* This whole procedure is one way to copy the spki into
	 * an EC_KEY, suggested by OpenSSL. Basically, this function
	 * returns the public key as a long int, which can later be
	 * casted to an EC_KEY
	 */
	*pub_key = d2i_EC_PUBKEY(NULL, (const unsigned char **)&p,
				 (long)SPKI_SIZE);

	if (!*pub_key)
		return RTR_BGPSEC_LOAD_PUB_KEY_ERROR;

	status = EC_KEY_check_key(*pub_key);
	if (status == 0) {
		EC_KEY_free(*pub_key);
		*pub_key = NULL;
		return RTR_BGPSEC_LOAD_PUB_KEY_ERROR;
	}

	return RTR_BGPSEC_SUCCESS;
}

int load_private_key(EC_KEY **priv_key, uint8_t *bytes_key)
{
	int status = 0;
	char *p = (char *)bytes_key;
	*priv_key = NULL;

	/* The private key copying is similar to the public key
	 * copying, except that the private key is returned directly
	 * as an EC_KEY.
	 */
	*priv_key = d2i_ECPrivateKey(NULL, (const unsigned char **)&p,
				     (long)PRIVATE_KEY_LENGTH);

	if (!*priv_key)
		return RTR_BGPSEC_LOAD_PRIV_KEY_ERROR;

	status = EC_KEY_check_key(*priv_key);
	if (status == 0) {
		EC_KEY_free(*priv_key);
		*priv_key = NULL;
		return RTR_BGPSEC_LOAD_PRIV_KEY_ERROR;
	}

	return RTR_BGPSEC_SUCCESS;
}

int hash_byte_sequence(
		uint8_t *bytes,
		unsigned int bytes_len,
		uint8_t alg_suite_id,
		unsigned char **hash_result)
{
	if (alg_suite_id == RTR_BGPSEC_ALGORITHM_SUITE_1) {
		SHA256_CTX ctx;

		*hash_result = lrtr_malloc(SHA256_DIGEST_LENGTH);
		if (!*hash_result)
			return RTR_BGPSEC_ERROR;

		SHA256_Init(&ctx);
		SHA256_Update(&ctx, (const unsigned char *)bytes, bytes_len);
		SHA256_Final(*hash_result, &ctx);

		if (!*hash_result)
			return RTR_BGPSEC_ERROR;
	} else {
		return RTR_BGPSEC_UNSUPPORTED_ALGORITHM_SUITE;
	}

	return RTR_BGPSEC_SUCCESS;
}

int sign_byte_sequence(uint8_t *hash_result,
		       EC_KEY *priv_key,
		       uint8_t alg,
		       struct rtr_signature_seg *new_signature)
{
	enum rtr_bgpsec_rtvals retval = RTR_BGPSEC_SUCCESS;
	unsigned int sig_res = 0;

	if (alg == RTR_BGPSEC_ALGORITHM_SUITE_1) {
		ECDSA_sign(0, hash_result, SHA256_DIGEST_LENGTH, new_signature->signature,
			   &sig_res, priv_key);
		if (sig_res < 1)
			retval = RTR_BGPSEC_SIGNING_ERROR;
		else
			new_signature->sig_len = sig_res;
	} else {
		retval = RTR_BGPSEC_UNSUPPORTED_ALGORITHM_SUITE;
	}

	return retval;
}

int ski_is_empty(uint8_t *ski)
{
	for (int i = 0; i < SKI_SIZE; i++) {
		if (ski[i])
			return 0;
	}
	return 1;
}

