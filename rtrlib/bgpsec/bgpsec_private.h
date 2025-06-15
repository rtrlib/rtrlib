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

#ifndef RTR_BGPSEC_PRIVATE_H
#define RTR_BGPSEC_PRIVATE_H

#include "rtrlib/bgpsec/bgpsec_utils_private.h"

/**
 * @brief Validation function for AS path validation.
 * @param[in] data Data required for AS path validation. See @ref rtr_bgpsec.
 * @param[in] table The SPKI table that contains the router keys.
 * @return RTR_BGPSEC_VALID If the AS path was valid.
 * @return RTR_BGPSEC_NOT_VALID If the AS path was not valid.
 * @return RTR_BGPSEC_ERROR If an error occurred. Refer to error codes for
 *			more details.
 */
int rtr_bgpsec_validate_as_path(const struct rtr_bgpsec *data, struct spki_table *table);

/**
 * @brief Signing function for a BGPsec_PATH.
 * @param[in] data Data required for AS path validation. See @ref rtr_bgpsec.
 * @param[in] private_key The raw bytes of the private key that is used for signing.
 * @param[out] new_signature Contains the generated signature and its length if
 *			     successful. Must not be allocated.
 * @return RTR_BGPSEC_SUCCESS If the signature was successfully generated.
 * @return RTR_BGPSEC_ERROR If an error occurred. Refer to error codes for
 *			    more details.
 */
int rtr_bgpsec_generate_signature(const struct rtr_bgpsec *data, uint8_t *private_key,
				  struct rtr_signature_seg **new_signature);

/**
 * @brief Returns the highest supported BGPsec version.
 * @return RTR_BGPSEC_VERSION The currently supported BGPsec version.
 */
int rtr_bgpsec_get_version(void);

/**
 * @brief Check, if an algorithm suite is supported by RTRlib.
 * @param[in] alg_suite The algorithm suite that is to be checked.
 * @return RTR_BGPSEC_SUCCESS If the algorithm suite is supported.
 * @return RTR_BGPSEC_ERROR If the algorithm suite is not supported.
 */
int rtr_bgpsec_has_algorithm_suite(uint8_t alg_suite);

/**
 * @brief Returns a pointer to a list that holds all supported algorithm suites.
 * @param[out] algs_arr A char pointer that contains all supported suites.
 * @return ALGORITHM_SUITES_COUNT The size of algs_arr
 */
int rtr_bgpsec_get_algorithm_suites(const uint8_t **algs_arr);

/** @brief Free a signature and any signatures that are pointed to.
 * @param[in] seg The signature that has been passed to the signing
 *	      function.
 */
void rtr_bgpsec_free_signatures(struct rtr_signature_seg *seg);

/**
 * @brief Return an allocated and initialized Secure Path Segment.
 * @param[in] pcount The pcount field.
 * @param[in] flags The flags field.
 * @param[in] asn The ASN of the segment.
 * @return A pointer to an initialized rtr_secure_path_seg struct
 *         or NULL if an error occurred, e.g. the memory allocation
 *         failed.
 */
struct rtr_secure_path_seg *rtr_bgpsec_new_secure_path_seg(uint8_t pcount, uint8_t flags, uint32_t asn);

/**
 * @brief Prepend a given Secure Path Segment to @ref rtr_bgpsec.path.
 * @param[in] bgpsec The rtr_bgpsec struct that holds the path.
 * @param[in] new_seg The Secure Path Segment that is appended to the path.
 */
void rtr_bgpsec_prepend_sec_path_seg(struct rtr_bgpsec *bgpsec, struct rtr_secure_path_seg *new_seg);

/**
 * @brief Return an allocated and initialized Signature.
 * @param[in] ski The Subject Key Identifier as byte representation.
 * @param[in] sig_len The length of the signature.
 * @param[in] signature The signature itself.
 * @return A pointer to an initialized rtr_secure_path_seg struct.
 *	   @ref rtr_signature_seg.signature is allocated with sig_len
 *	   bytes.
 */
struct rtr_signature_seg *rtr_bgpsec_new_signature_seg(uint8_t *ski, uint16_t sig_len, uint8_t *signature);

/**
 * @brief Prepend a given Signature Segment to @ref rtr_bgpsec.sigs. All fields
 *	  of the new_seg must be filled.
 * @param[in] bgpsec The rtr_bgpsec struct that holds the signatures.
 * @param[in] new_seg The Signature Segment that is appended to the signatures.
 * @return RTR_BGPSEC_SUCCESS If the signature was successfully prepended.
 * @return RTR_BGPSEC_ERROR If an error occurred during prepending, e.g. one
 *			    or more fields of new_seg was missing.
 */
int rtr_bgpsec_prepend_sig_seg(struct rtr_bgpsec *bgpsec, struct rtr_signature_seg *new_seg);

/**
 * @brief Initializes and returns a pointer to a rtr_bgpsec struct.
 * @param[in] alg The Algorithm Suite Identifier.
 * @param[in] safi The Subsequent Address Family Identifier.
 * @param[in] afi The Address Family Identifier.
 * @param[in] my_as The AS that is currently performing validation (you).
 * @param[in] target_as The AS where the update should be sent to.
 * @param[in] nlri The Network Layer Reachability Information.
 * @return A pointer to an initialized rtr_bgpsec struct.
 */
struct rtr_bgpsec *rtr_bgpsec_new(uint8_t alg, uint8_t safi, uint16_t afi, uint32_t my_as, uint32_t target_as,
				  struct rtr_bgpsec_nlri *nlri);

/**
 * @brief Allocate memory for a rtr_bgpsec_nlri struct.
 * @return A pointer to an allocated rtr_bgpsec_nlri struct
 *         or NULL if the allocation failed.
 */
struct rtr_bgpsec_nlri *rtr_bgpsec_nlri_new(int nlri_len);

/**
 * @brief Free a rtr_bgpsec_nlri struct.
 * @param[in] nlri The rtr_bgpsec_nlri struct that is to be freed.
 */
void rtr_bgpsec_nlri_free(struct rtr_bgpsec_nlri *nlri);

/**
 * @brief Free a rtr_bgpsec struct and any Secure Path and Signature
 *	  Segments it holds.
 * @param[in] bgpsec The rtr_bgpsec struct that is to be freed.
 */
void rtr_bgpsec_free(struct rtr_bgpsec *bgpsec);

/**
 * @brief Free a Secure Path Segment and any segments that are pointed to
 *	  by @ref rtr_secure_path_seg.next.
 * @param[in] seg The Secure Path Segment that is to be freed.
 */
void rtr_bgpsec_free_secure_path(struct rtr_secure_path_seg *seg);

/**
 * @brief Pop off the first Signature Segment from a given rtr_bgpsec struct
 *	  and return this Signature Segment.
 * @param[in] bgpsec The rtr_bgpsec struct containing the Signature Segments
 *	      @ref rtr_bgpsec.sigs.
 * @return The Signature Segment that was popped off from @ref rtr_bgpsec.sigs.
 */
struct rtr_signature_seg *rtr_bgpsec_pop_signature_seg(struct rtr_bgpsec *bgpsec);

/**
 * @brief Pop off the first Secure Path Segment from a given rtr_bgpsec struct
 *	  and return this Secure Path Segment.
 * @param[in] bgpsec The rtr_bgpsec struct containing the Secure Path Segments
 *	      @ref rtr_bgpsec.path.
 * @return The Secure Path Segment that was popped off from @ref rtr_bgpsec.path.
 */
struct rtr_secure_path_seg *rtr_bgpsec_pop_secure_path_seg(struct rtr_bgpsec *bgpsec);

/**
 * @brief Append a Signature Segment to the end of the @ref rtr_bgpsec.sigs of a
 *	  given rtr_bgpsec struct.
 * @param[in] bgpsec The rtr_bgpsec struct with the @ref rtr_bgpsec.sigs to
 *	      append the Signature Segment to.
 * @param[in] new_seg The Signature Segments that will be appended.
 * @return RTR_BGPSEC_SUCCESS If the Signature Segment was successfully appended.
 * @return RTR_BGPSEC_ERROR If an error occurred in the proccess.
 */
int rtr_bgpsec_append_sig_seg(struct rtr_bgpsec *bgpsec, struct rtr_signature_seg *new_seg);

/**
 * @brief Append a Secure Path Segment to the end of the @ref rtr_bgpsec.path of a
 *	  given rtr_bgpsec struct.
 * @param[in] bgpsec The rtr_bgpsec struct with the @ref rtr_bgpsec.path to
 *	      append the Secure Path Segment to.
 * @param[in] new_seg The Secure Path Segments that will be appended.
 */
void rtr_bgpsec_append_sec_path_seg(struct rtr_bgpsec *bgpsec, struct rtr_secure_path_seg *new_seg);

/**
 * @brief Manually add a SPKI record into the SPKI table.
 * @param[in] table The SPKI table holding the SPKI data.
 * @param[in] record The new record that will be added to the SPKI table.
 */
void rtr_bgpsec_add_spki_record(struct spki_table *table, struct spki_record *record);
#endif
/* @} */
