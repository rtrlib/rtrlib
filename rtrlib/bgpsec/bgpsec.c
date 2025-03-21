/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include "rtrlib/bgpsec/bgpsec.h"

#include "rtrlib/bgpsec/bgpsec_private.h"
#include "rtrlib/bgpsec/bgpsec_utils_private.h"
#include "rtrlib/lib/alloc_utils_private.h"
#include "rtrlib/rtrlib_export_private.h"
#include "rtrlib/spki/spkitable_private.h"

#define SEC_PATH_LEN(seg, idx)                 \
	struct rtr_secure_path_seg *tmp = seg; \
	while (tmp) {                          \
		(idx)++;                       \
		tmp = tmp->next;               \
	}

/** The latest supported BGPsec version. */
#define BGPSEC_VERSION 0

/**
 * @brief A static list that contains all supported algorithm suites.
 */
static const uint8_t algorithm_suites[] = {RTR_BGPSEC_ALGORITHM_SUITE_1};

/*
 * The data for digestion must be ordered exactly like this:
 *
 * +------------------------------------+
 * | Target AS Number                   |
 * +------------------------------------+----\
 * | Signature Segment   : N-1          |     \
 * +------------------------------------+     |
 * | Secure_Path Segment : N            |     |
 * +------------------------------------+     \
 *       ...                                  >  Data from
 * +------------------------------------+     /   N Segments
 * | Signature Segment   : 1            |     |
 * +------------------------------------+     |
 * | Secure_Path Segment : 2            |     |
 * +------------------------------------+     /
 * | Secure_Path Segment : 1            |    /
 * +------------------------------------+---/
 * | Algorithm Suite Identifier         |
 * +------------------------------------+
 * | AFI                                |
 * +------------------------------------+
 * | SAFI                               |
 * +------------------------------------+
 * | NLRI                               |
 * +------------------------------------+
 *
 * https://tools.ietf.org/html/rfc8205#section-4.2
 */

/* The arrays are passed in "AS path order", meaning the last appended
 * Signature Segment / Secure_Path Segment is at the first
 * position of the array.
 */

int rtr_bgpsec_validate_as_path(const struct rtr_bgpsec *data, struct spki_table *table)
{
	if (table == NULL) {
		BGPSEC_DBG1("TRYING TO VALIDATE A, BUT NO SPKI TABLE INITIALIZED");
		return RTR_BGPSEC_ERROR;
	}

	/* The AS path validation result. */
	enum rtr_bgpsec_rtvals retval = 0;

	/* The resulting hash. */
	unsigned char *hash_result = NULL;

	/* A temporare spki record */
	struct spki_record *tmp_key = NULL;

	/* A stream that holds the data that is hashed */
	struct stream *s = NULL;

	/* Total size of required space for the stream */
	unsigned int stream_size = 0;

	/* Use a temp variable in the validation loop since we don't want to
	 * alter data->sigs.
	 */
	struct rtr_signature_seg *tmp_sig = NULL;

	/* Temp variable that holds the signature length of the of the
	 * next signature segment.
	 */
	int tmp_sig_len = 0;

	/* Check, if the parameters are not NULL */
	if (!data || !data->path || !data->sigs || !table)
		return RTR_BGPSEC_INVALID_ARGUMENTS;

	/* Check, if there are as many signature segments as there are
	 * secure path segments
	 */
	if (data->path_len != data->sigs_len)
		return RTR_BGPSEC_WRONG_SEGMENT_COUNT;

	/* Check, if the algorithm suite is supported by RTRlib. */
	if (rtr_bgpsec_has_algorithm_suite(data->alg) == RTR_BGPSEC_ERROR)
		return RTR_BGPSEC_UNSUPPORTED_ALGORITHM_SUITE;

	/* Check, if the AFI is usable with BGPsec */
	if ((data->nlri->afi != BGPSEC_IPV4) && (data->nlri->afi != BGPSEC_IPV6))
		return RTR_BGPSEC_UNSUPPORTED_AFI;

	/* Make sure that all router keys are available. */
	retval = check_router_keys(data->sigs, table);

	if (retval != RTR_BGPSEC_SUCCESS)
		goto err;

	/* Calculate the required stream size and initialize the stream */
	stream_size = req_stream_size(data, VALIDATION);
	s = init_stream(stream_size);

	/* Align the byte sequence and store it in the stream */
	retval = align_byte_sequence(data, s, VALIDATION);

	if (retval != RTR_BGPSEC_SUCCESS)
		goto err;

	/*
	 * The validation is an iterative process. In the first iteration,
	 * the entire byte sequence is hashed to validate the first signature.
	 * In the next iteration, the process is repeated. This time, the
	 * starting position for hashing is moved by an offset to only hash
	 * the data required to validate the next signature. This procedure
	 * is repeated until all signatures are validated or a signature is
	 * determined invalid.
	 *
	 * offset: the current position from where on the bytes should
	 * be hashed.
	 *
	 * next_offset: adds to offset, after the bytes on the
	 * current offset have been processed. next_offset is not
	 * constant and must be calculated each iteration:
	 *
	 * signature length +
	 * SKI size +
	 * sizeof(var that holds signature length) +
	 * sizeof(a secure path segment)
	 *
	 *
	 *  offset
	 * |o++++++++++++++++++++++++++++++++++++++++| bytes
	 *
	 *		offset+=
	 *		new_offset
	 * |------------o++++++++++++++++++++++++++++| bytes
	 *
	 *			    offset+=
	 *			    new_offset
	 * |------------------------o++++++++++++++++| bytes
	 *
	 *
	 * A more detailed view can be found at
	 *https://mailarchive.ietf.org/arch/msg/sidr/8B_e4CNxQCUKeZ_AUzsdnn2f5Mu
	 **/

	/* Set retval to RTR_BGPSEC_VALID so the for-condition does not
	 * fail on the first for loop check.
	 */
	retval = RTR_BGPSEC_VALID;
	tmp_sig = data->sigs;

	for (unsigned int offset = 0, next_offset = 0; offset <= get_stream_size(s) && retval == RTR_BGPSEC_VALID;
	     offset += next_offset) {
		if (tmp_sig->next)
			tmp_sig_len = tmp_sig->next->sig_len;
		else
			tmp_sig_len = tmp_sig->sig_len;

		next_offset = tmp_sig_len + SKI_SIZE + sizeof(tmp_sig->sig_len) + SECURE_PATH_SEG_SIZE;

		/*
		 * From a certain position on, read until the end of the stream.
		 * The starting position is determined by the offset. The end
		 * of the stream is the total length minus the offset.
		 * The curr pointer points to the resulting "sub-stream".
		 * Hacky, could be improved.
		 */
		size_t len = get_stream_size(s) - offset;

		uint8_t *curr = lrtr_malloc(len);

		read_stream_at(curr, s, offset, len);

		retval = hash_byte_sequence(curr, len, data->alg, &hash_result);

		lrtr_free(curr);

		if (retval != RTR_BGPSEC_SUCCESS)
			goto err;

		/* Store all router keys for the given SKI in tmp_key. */
		unsigned int router_keys_len = 0;
		enum spki_rtvals spki_retval =
			spki_table_search_by_ski(table, tmp_sig->ski, &tmp_key, &router_keys_len);
		if (spki_retval == SPKI_ERROR) {
			retval = RTR_BGPSEC_ERROR;
			goto err;
		}

		/* Loop in case there are multiple router keys for one SKI. */
		for (unsigned int j = 0; j < router_keys_len; j++) {
			/* Validate the siganture depending on the algorithm
			 * suite. More if-cases are added with new algorithm
			 * suites.
			 */
			if (data->alg == RTR_BGPSEC_ALGORITHM_SUITE_1) {
				retval = validate_signature(hash_result, tmp_sig, &tmp_key[j]);
			} else {
				retval = RTR_BGPSEC_UNSUPPORTED_ALGORITHM_SUITE;
				goto err;
			}
			/* As soon as one of the router keys produces a valid
			 * result, exit the loop.
			 */
			if (retval == RTR_BGPSEC_VALID)
				break;
		}
		lrtr_free(hash_result);
		lrtr_free(tmp_key);
		hash_result = NULL;
		tmp_key = NULL;
		tmp_sig = tmp_sig->next;
	}

err:
	if (hash_result)
		lrtr_free(hash_result);
	if (tmp_key)
		lrtr_free(tmp_key);
	if (s)
		free_stream(s);

	if (retval == RTR_BGPSEC_VALID)
		BGPSEC_DBG1("Validation result for the whole BGPsec_PATH: valid");
	else if (retval == RTR_BGPSEC_NOT_VALID)
		BGPSEC_DBG1("Validation result for the whole BGPsec_PATH: invalid");

	return retval;
}

int rtr_bgpsec_generate_signature(const struct rtr_bgpsec *data, uint8_t *private_key,
				  struct rtr_signature_seg **new_signature)
{
	/* The signature generation result. */
	enum rtr_bgpsec_rtvals retval = 0;

	/* The resulting hash. */
	unsigned char *hash_result = NULL;

	/* The required size for the signature */
	int req_sig_size = 0;

	/* A stream that holds the data that is hashed */
	struct stream *s = NULL;

	/* Total size of required space for the stream */
	unsigned int stream_size = 0;

	/* OpenSSL private key structure. */
	EC_KEY *priv_key = NULL;

	/* Check, if the parameters are not NULL except for *new_signature,
	 * which must be NULL.
	 */
	if (!data || !data->path || !private_key || *new_signature)
		return RTR_BGPSEC_INVALID_ARGUMENTS;

	/* Make sure the algorithm suite is supported. */
	if (rtr_bgpsec_has_algorithm_suite(data->alg) == RTR_BGPSEC_ERROR)
		return RTR_BGPSEC_UNSUPPORTED_ALGORITHM_SUITE;

	/* Check, if the AFI is usable with BGPsec */
	if ((data->nlri->afi != BGPSEC_IPV4) && (data->nlri->afi != BGPSEC_IPV6))
		return RTR_BGPSEC_UNSUPPORTED_AFI;

	/* Check, if there is one more secure path segment than there are
	 * signature segments. This is because the next signature is generated
	 * for a secure path segment, that was not signed yet (the last appended
	 * secure path segment).
	 */
	if (data->path_len != (data->sigs_len + 1))
		return RTR_BGPSEC_WRONG_SEGMENT_COUNT;

	/* Load the private key from buffer into OpenSSL structure. */
	retval = load_private_key(&priv_key, private_key);

	if (retval != RTR_BGPSEC_SUCCESS) {
		retval = RTR_BGPSEC_LOAD_PRIV_KEY_ERROR;
		goto err;
	}

	/* Allocate the needed space for the signature */
	req_sig_size = ECDSA_size(priv_key);
	if (req_sig_size == 0) {
		retval = RTR_BGPSEC_LOAD_PRIV_KEY_ERROR;
		goto err;
	}

	*new_signature = rtr_bgpsec_new_signature_seg(NULL, req_sig_size, NULL);
	if (!(*new_signature)) {
		retval = RTR_BGPSEC_ERROR;
		goto err;
	}

	/* The function call above expects the exact size of the signature.
	 * Because the signature was not generated yet, we pass the maximum
	 * possible length to it so enough space is allocated. Then, the
	 * sig_len field is set to 0, since the signature does not exist yet.
	 * This is a rather hacky workaround and might get changed in the
	 * future.
	 */
	(*new_signature)->sig_len = 0;

	/* Calculate the required stream size and initialize the stream */
	stream_size = req_stream_size(data, SIGNING);
	s = init_stream(stream_size);

	/* Align the bytes to prepare them for hashing. */
	retval = align_byte_sequence(data, s, SIGNING);

	if (retval != RTR_BGPSEC_SUCCESS)
		goto err;

	/* Hash the aligned bytes. */
	uint8_t *curr = get_stream_start(s);

	retval = hash_byte_sequence(curr, get_stream_size(s), data->alg, &hash_result);

	if (retval != RTR_BGPSEC_SUCCESS)
		goto err;

	/* Sign the hash depending on the algorithm suite. */
	retval = sign_byte_sequence(hash_result, priv_key, data->alg, *new_signature);

err:
	if (hash_result)
		lrtr_free(hash_result);
	if (*new_signature && (retval == RTR_BGPSEC_SIGNING_ERROR))
		rtr_bgpsec_free_signatures(*new_signature);
	if (s)
		free_stream(s);
	if (priv_key) {
		EC_KEY_free(priv_key);
		priv_key = NULL;
	}

	return retval;
}

/*************************************************
 **** Functions for versions and algo suites *****
 ************************************************/

int rtr_bgpsec_get_version(void)
{
	return BGPSEC_VERSION;
}

int rtr_bgpsec_has_algorithm_suite(uint8_t alg_suite)
{
	int alg_suites_len = sizeof(algorithm_suites) / sizeof(uint8_t);

	for (int i = 0; i < alg_suites_len; i++) {
		if (alg_suite == algorithm_suites[i])
			return RTR_BGPSEC_SUCCESS;
	}

	return RTR_BGPSEC_ERROR;
}

int rtr_bgpsec_get_algorithm_suites(const uint8_t **algs_arr)
{
	*algs_arr = algorithm_suites;
	return sizeof(algorithm_suites) / sizeof(uint8_t);
}

int rtr_bgpsec_prepend_sig_seg(struct rtr_bgpsec *bgpsec, struct rtr_signature_seg *new_seg)
{
	if (!new_seg || !new_seg->signature || !new_seg->sig_len || ski_is_empty(new_seg->ski))
		return RTR_BGPSEC_ERROR;

	if (bgpsec->sigs)
		new_seg->next = bgpsec->sigs;

	bgpsec->sigs = new_seg;
	bgpsec->sigs_len++;

	return RTR_BGPSEC_SUCCESS;
}

struct rtr_signature_seg *rtr_bgpsec_pop_signature_seg(struct rtr_bgpsec *bgpsec)
{
	struct rtr_signature_seg *tmp = NULL;

	if (!bgpsec->sigs)
		return NULL;

	tmp = bgpsec->sigs;
	bgpsec->sigs = bgpsec->sigs->next;
	bgpsec->sigs_len--;
	tmp->next = NULL;
	return tmp;
}

int rtr_bgpsec_append_sig_seg(struct rtr_bgpsec *bgpsec, struct rtr_signature_seg *new_seg)
{
	struct rtr_signature_seg *last = bgpsec->sigs;

	if (!new_seg || !new_seg->signature || !new_seg->sig_len || ski_is_empty(new_seg->ski))
		return RTR_BGPSEC_ERROR;

	if (bgpsec->sigs) {
		while (last->next)
			last = last->next;
		last->next = new_seg;
	} else {
		bgpsec->sigs = new_seg;
	}

	bgpsec->sigs_len++;

	return RTR_BGPSEC_SUCCESS;
}

struct rtr_signature_seg *rtr_bgpsec_new_signature_seg(uint8_t *ski, uint16_t sig_len, uint8_t *signature)
{
	struct rtr_signature_seg *sig_seg = lrtr_malloc(sizeof(struct rtr_signature_seg));

	if (!sig_seg)
		return NULL;

	sig_seg->signature = lrtr_calloc(sig_len, 1);
	if (!sig_seg->signature) {
		lrtr_free(sig_seg);
		return NULL;
	}

	if (ski)
		memcpy(sig_seg->ski, ski, SKI_SIZE);
	else
		memset(sig_seg->ski, '\0', SKI_SIZE);

	if (signature)
		memcpy(sig_seg->signature, signature, sig_len);

	sig_seg->sig_len = sig_len;
	sig_seg->next = NULL;
	return sig_seg;
}

void rtr_bgpsec_free_signatures(struct rtr_signature_seg *seg)
{
	if (!seg)
		return;
	if (seg->next)
		rtr_bgpsec_free_signatures(seg->next);
	lrtr_free(seg->signature);
	lrtr_free(seg);
}

void rtr_bgpsec_prepend_sec_path_seg(struct rtr_bgpsec *bgpsec, struct rtr_secure_path_seg *new_seg)
{
	if (bgpsec->path)
		new_seg->next = bgpsec->path;

	bgpsec->path = new_seg;
	bgpsec->path_len++;
}

void rtr_bgpsec_append_sec_path_seg(struct rtr_bgpsec *bgpsec, struct rtr_secure_path_seg *new_seg)
{
	struct rtr_secure_path_seg *last = bgpsec->path;

	if (bgpsec->path) {
		while (last->next)
			last = last->next;
		last->next = new_seg;
	} else {
		bgpsec->path = new_seg;
	}

	bgpsec->path_len++;
}

struct rtr_secure_path_seg *rtr_bgpsec_new_secure_path_seg(uint8_t pcount, uint8_t flags, uint32_t asn)
{
	struct rtr_secure_path_seg *seg = lrtr_malloc(sizeof(struct rtr_secure_path_seg));

	seg->pcount = pcount;
	seg->flags = flags;
	seg->asn = asn;
	seg->next = NULL;
	return seg;
}

struct rtr_secure_path_seg *rtr_bgpsec_pop_secure_path_seg(struct rtr_bgpsec *bgpsec)
{
	struct rtr_secure_path_seg *tmp = NULL;

	if (!bgpsec->path)
		return NULL;

	tmp = bgpsec->path;
	bgpsec->path = bgpsec->path->next;
	bgpsec->path_len--;
	tmp->next = NULL;
	return tmp;
}

struct rtr_bgpsec *rtr_bgpsec_new(uint8_t alg, uint8_t safi, uint16_t afi, uint32_t my_as, uint32_t target_as,
				  struct rtr_bgpsec_nlri *nlri)
{
	struct rtr_bgpsec *bgpsec = lrtr_malloc(sizeof(struct rtr_bgpsec));

	if (!bgpsec)
		return NULL;

	bgpsec->alg = alg;
	bgpsec->safi = safi;
	bgpsec->afi = afi;
	bgpsec->my_as = my_as;
	bgpsec->target_as = target_as;
	bgpsec->nlri = nlri;
	bgpsec->path_len = 0;
	bgpsec->path = NULL;
	bgpsec->sigs_len = 0;
	bgpsec->sigs = NULL;

	return bgpsec;
}

void rtr_bgpsec_free(struct rtr_bgpsec *bgpsec)
{
	if (bgpsec->path)
		rtr_bgpsec_free_secure_path(bgpsec->path);
	if (bgpsec->sigs)
		rtr_bgpsec_free_signatures(bgpsec->sigs);
	if (bgpsec->nlri)
		rtr_bgpsec_nlri_free(bgpsec->nlri);
	lrtr_free(bgpsec);
}

void rtr_bgpsec_free_secure_path(struct rtr_secure_path_seg *seg)
{
	if (!seg)
		return;
	if (seg->next)
		rtr_bgpsec_free_secure_path(seg->next);
	lrtr_free(seg);
}

struct rtr_bgpsec_nlri *rtr_bgpsec_nlri_new(int nlri_len)
{
	struct rtr_bgpsec_nlri *nlri = lrtr_malloc(sizeof(struct rtr_bgpsec_nlri));

	nlri->nlri = lrtr_malloc(nlri_len);
	return nlri;
}

void rtr_bgpsec_nlri_free(struct rtr_bgpsec_nlri *nlri)
{
	if (!nlri)
		return;

	if (nlri->nlri)
		lrtr_free(nlri->nlri);

	lrtr_free(nlri);
}

void rtr_bgpsec_add_spki_record(struct spki_table *table, struct spki_record *record)
{
	spki_table_add_entry(table, record);
}
