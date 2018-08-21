/*
 * This file is part of RTRlib.
 *
 * This file is subject to the terms and conditions of the MIT license.
 * See the file LICENSE in the top level directory for more details.
 *
 * Website: http://rtrlib.realmv6.org/
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "alloc_utils.h"

#include "rtrlib/rtrlib_export.h"

void *(*MALLOC_PTR)(size_t size) = malloc;
void *(*REALLOC_PTR)(void *ptr, size_t size) = realloc;
void (*FREE_PTR)(void *ptr) = free;

/* cppcheck-suppress unusedFunction */
RTRLIB_EXPORT void lrtr_set_alloc_functions(
		void *(*malloc_function)(size_t size),
		void *(*realloc_function)(void *ptr, size_t size),
		void (free_function)(void *ptr))
{
	MALLOC_PTR = malloc_function;
	REALLOC_PTR = realloc_function;
	FREE_PTR = free_function;
}

inline void *lrtr_malloc(size_t size)
{
	return MALLOC_PTR(size);
}

inline void lrtr_free(void *ptr)
{
	return FREE_PTR(ptr);
}

inline void *lrtr_realloc(void *ptr, size_t size)
{
	return REALLOC_PTR(ptr, size);
}

char *lrtr_strdup(const char *string)
{
	assert(string);

	size_t length = strlen(string) + 1;
	char *new_string = lrtr_malloc(length);

	return new_string ? memcpy(new_string, string, length) : NULL;
}
