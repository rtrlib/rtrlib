/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

/**
 * @defgroup mod_spki_h Subject Public Key Info table
 * @brief The spki_table is an abstract data structure to organize the received Router Key PDUs
 * from a RPKI-RTR cache server.
 *
 * @{
 */

#ifndef RTR_SPKI_H
#define RTR_SPKI_H

#include "rtrlib/rtr/rtr.h"

#include <stdbool.h>
#include <stdint.h>

#define SKI_SIZE 20
#define SPKI_SIZE 91

struct spki_table;

/**
 * @brief spki_record.
 * @param ski Subject Key Identifier
 * @param asn Origin AS number
 * @param spki Subject public key info
 * @param socket Pointer to the rtr_socket this spki_record was received in
 */
struct spki_record {
	uint8_t ski[SKI_SIZE];
	uint32_t asn;
	uint8_t spki[SPKI_SIZE];
	const struct rtr_socket *socket;
};

/**
 * @brief A function pointer that is called if an record was added
 * to the spki_table or was removed from the spki_table.
 * @param spki_table which was updated.
 * @param record spki_record that was modified.
 * @param added True if the record was added, false if the record was removed.
 */
typedef void (*spki_update_fp)(struct spki_table *spki_table, const struct spki_record record, const bool added);
#endif
/** @} */
