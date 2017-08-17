/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef LRTR_ALLOC_UTILS_H
#define LRTR_ALLOC_UTILS_H

#include <stdlib.h>

extern void *(*MALLOC_PTR)(size_t size);
extern void (*FREE_PTR)(void *ptr);
extern void *(*REALLOC_PTR)(void *ptr, size_t size);

/**
 * @brief Sets custom malloc, realloc and free function
 * that is used throughout rtrlib.
 * @param[in] Pointer to malloc function
 * @param[in] Pointer to realloc function
 * @param[in] Pointer to free function
 */
void lrtr_set_alloc_functions(
		void *(*malloc_function)(size_t size),
		void *(*realloc_function)(void *ptr, size_t size),
		void (free_function)(void *ptr));

void *lrtr_malloc(size_t size);

void lrtr_free(void *ptr);

void *lrtr_realloc(void *ptr, size_t size);

/**
 * @brief Duplicates a string
 * @pre string != NULL
 * @param[in] string
 * @returns Duplicated string
 * @returns NULL on error
 */
char *lrtr_strdup(const char *string);

#endif
