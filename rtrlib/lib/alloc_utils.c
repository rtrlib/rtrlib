/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include <stdlib.h>

#include "alloc_utils.h"

void* (*MALLOC_PTR)(size_t size);
void* (*REALLOC_PTR)(void* ptr, size_t size);
void (*FREE_PTR)(void* ptr);


void lrtr_set_malloc(void* (*malloc_function)(size_t size))
{
	 MALLOC_PTR = malloc_function;
}

void lrtr_set_realloc(void* (*realloc_ptr)(void* ptr, size_t size))
{
	REALLOC_PTR = realloc_ptr;
}

void lrtr_set_free(void (free_ptr)(void* ptr))
{
	FREE_PTR = free_ptr;
}

inline void* lrtr_malloc(size_t size)
{
	if (MALLOC_PTR) {
		return MALLOC_PTR(size);
	} else {
		return malloc(size);
	}
}

inline void lrtr_free(void* ptr)
{
	if (FREE_PTR) {
		return FREE_PTR(ptr);
	} else {
		return free(ptr);
	}
}

inline void* lrtr_realloc(void* ptr, size_t size)
{
	if (REALLOC_PTR) {
		return REALLOC_PTR(ptr, size);
	}  else {
		return realloc(ptr, size);
	}
}
