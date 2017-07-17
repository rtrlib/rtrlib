/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#ifndef LRTR_UTILS_H
#define LRTR_UTILS_H

#include <stdlib.h>

extern void* (*MALLOC_PTR)(size_t size);
extern void (*FREE_PTR)(void* ptr);
extern void* (*REALLOC_PTR)(void* ptr, size_t size);

/**
* @brief Sets a custom malloc function that is used throughout rtrlib.
* If you set this function you must also set custom free and custom realloc
* @param[in] Pointer to a malloc function
*/
void lrtr_set_malloc(void* (*malloc_function)(size_t size));

/**
* @brief Sets a custom free function that is used throughout rtrlib.
* If you set this function you must also set custom malloc and realloc
* @param[in] Pointer to a free function
*/
void lrtr_set_free(void (free_ptr)(void* ptr));

/**
* @brief Sets a custom realloc function that is used throughout rtrlib.
* If you set this function you must also set custom malloc and free.
* @param[in] Pointer to a realloc function
*/
void lrtr_set_realloc(void* (*realloc_ptr)(void* ptr, size_t size));

inline void* lrtr_malloc(size_t size) {
	if (MALLOC_PTR) {
		return MALLOC_PTR(size);
	} else {
		return malloc(size);
	}
}

inline void lrtr_free(void* ptr) {
	if (FREE_PTR) {
		return FREE_PTR(ptr);
	} else {
		return free(ptr);
	}
}

inline void* lrtr_realloc(void* ptr, size_t size) {
	if (REALLOC_PTR) {
		return REALLOC_PTR(ptr, size);
	}  else {
		return realloc(ptr, size);
	}
}

#endif
