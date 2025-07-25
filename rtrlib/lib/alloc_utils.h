/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef RTR_ALLOC_UTILS_H
#define RTR_ALLOC_UTILS_H

#include <stdlib.h>

/**
 * @brief Sets custom malloc, realloc and free function
 * that is used throughout rtrlib.
 * @param[in] Pointer to malloc function
 * @param[in] Pointer to realloc function
 * @param[in] Pointer to free function
 */
void rtr_set_alloc_functions(void *(*malloc_function)(size_t size), void *(*realloc_function)(void *ptr, size_t size),
			      void (*free_function)(void *ptr));

#endif
