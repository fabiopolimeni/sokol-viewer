#pragma once
/**
 * Global memory manager
 */

#include <stdint.h>
#include <stddef.h>

/**
 * Memory will always be allocated with MEMORY_DEFAULT_ALIGNMENT
 * if no alignment is given to memory allocation functions.
 */
#define MEMORY_DEFAULT_ALIGNMENT (16)

#ifdef __cplusplus
extern "C" {
#endif

void * memory_aligned_malloc(size_t sz, size_t al);
void * memory_aligned_calloc(size_t cnt, size_t sz, size_t al);
void * memory_aligned_realloc(void* ptr, size_t sz, size_t al);
void * memory_malloc(size_t sz);
void * memory_calloc(size_t cnt, size_t sz);
void * memory_realloc(void* ptr, size_t sz);
void memory_free(void* ptr);

/**
 * Generic simple and minimal allocator interface expected to work as `realloc`.
 * 
 * @param ptr If null, a new allocation of `size` will be made. If not null the
 *  memory pointed by `ptr` will be reallocated to accommodate at least `size`
 *  bytes.
 * @param size The size of the allocation. If equal 0, and `ptr` is not null,
 *  memory pointed by `ptr` will be released.
 * @return Pointer to the allocated memory. This value is valid to be the same
 *  as the given `ptr`.
 */
typedef void* (*memory_allocator_t)(void* ptr, size_t size);

#ifdef __cplusplus
}
#endif
