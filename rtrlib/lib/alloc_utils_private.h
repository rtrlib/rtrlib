/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef RTR_ALLOC_UTILS_PRIVATE_H
#define RTR_ALLOC_UTILS_PRIVATE_H

#include "alloc_utils.h"

#include <stdlib.h>

void *rtr_malloc(size_t size);

void *rtr_calloc(size_t nmemb, size_t size);

void rtr_free(void *ptr);

void *rtr_realloc(void *ptr, size_t size);

/**
 * @brief Duplicates a string
 * @pre string != NULL
 * @param[in] string
 * @returns Duplicated string
 * @returns NULL on error
 */
char *rtr_strdup(const char *string);

#endif
