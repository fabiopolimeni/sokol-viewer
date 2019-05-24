/**
 * Global memory manager implementation
 */
#include "viewer_memory.h"
#include  <assert.h>

/**
 * Bit scan forward - Count trailing zeroes
 */
#if defined(__clang__) || defined(__GNUC__)
#if defined(_M_X64) || defined(__x86_64__)
#define _VIEWER_BSF(r, v) r = __builtin_ctz(v)
#else
#define _VIEWER_BSF(r, v) r = __builtin_ctzll(v)
#endif
#elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
#if defined(_MSC_VER)
#pragma warning(disable:4146)
#endif
#include <intrin.h>
#if defined(_M_X64) || defined(__x86_64__)
#define _VIEWER_BSF(r, v) _BitScanForward64((unsigned long*)&r, v)
#else
#define _VIEWER_BSF(r, v) _BitScanForward((unsigned long*)&r, v)
#endif
#endif

#define _IS_POWER_OF_TWO(a) ((a) ? !(a & (a - 1)) : 0)

/**
 * Provide a default implementation for allocations
 */
#if !defined(VIEWER_MALLOC) || !defined(VIEWER_FREE) || !defined(VIEWER_MEMSIZE)
#include <stdlib.h> // malloc, free
#define VIEWER_MALLOC(sz) malloc(sz)
#define VIEWER_FREE(ptr) free(ptr)

static inline size_t _memory_get_size(void* optr) {
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
    return _msize(optr);
#else
    return malloc_usable_size(optr);
#endif
}

#define VIEWER_MEMSIZE(ptr) _memory_get_size(ptr)
#endif // VIEWER_MALLOC/FREE

#if !defined(VIEWER_ALIGNED_MALLOC) \
    || !defined(VIEWER_ALIGNED_REALLOC) \
    || !defined(VIEWER_ALIGNED_FREE)

#include <string.h> // memmove, memset
#include <malloc.h>

static inline void* _memory_get_allocated_ptr(void* ptr) {
    return ((void**)ptr)[-1];
}

static void * _memory_alloc_aligned(size_t sz, size_t al) {
    // Alignment must be a power of two.
    assert(_IS_POWER_OF_TWO(al));

    if (!sz) {
        return NULL;
    }

    // We need extra bytes to store the
    // original value returned by malloc.
    if (al < sizeof(void *)) {
        al = sizeof(void *);
    }

    void *const malloc_ptr = VIEWER_MALLOC(sz + al);
    if (!malloc_ptr) {
        return NULL;
    }

    // Align to the requested value leaving room for the original malloc value.
    void *const aligned_ptr = (void *) (((uintptr_t) malloc_ptr + al) & -al);

    // Store the original malloc value where
    // it can be found by operator delete.
    ((void **) aligned_ptr)[-1] = malloc_ptr;

    return aligned_ptr;
}

static inline void _memory_free_aligned(void* ptr) {
    VIEWER_FREE(_memory_get_allocated_ptr(ptr));
}

static inline size_t _memory_get_available_memory(void* ptr) {
    void* optr = _memory_get_allocated_ptr(ptr);
    const size_t dsz = (uintptr_t)ptr - (uintptr_t)optr;
    return VIEWER_MEMSIZE(optr) - dsz;
}

static inline size_t _memory_alignment_of(uintptr_t uptr) {
    size_t out_align = 0;

    // BSF can't cope with 0
    if (uptr) {
        _VIEWER_BSF(out_align, uptr);
    }

    return 1ULL << out_align;
}

static void* _memory_realloc_aligned(void* ptr, size_t sz, size_t al)  {
    // Handle special cases
    if (!ptr) {
        return _memory_alloc_aligned(sz, al);
    }

    if (!sz) {
    	_memory_free_aligned(ptr);
    	return NULL;
    }

    // If pointer and size are valid, then, check the current state of the
    // given pointer, and, if the new size and alignment are less or equal
    // to the original allocated block size, then, just return the pointer.
    const size_t osz = _memory_get_available_memory(ptr);
    const size_t oal = _memory_alignment_of((uintptr_t)ptr);
    if (sz <= osz) {
        if (oal >= al) {
            return ptr;
        }
    }

    const size_t nal = (oal < al) ? al : oal;
    void* nptr = _memory_alloc_aligned(sz, nal);

    // Memory move can't cope with null pointers, as it would result into an
    // undefined  behaviour in release mode, or would result into an assert
    // in debug. Therefore, we need to intercept an eventual memory allocation
    // failure, and return a null pointer instead.
    if (nptr) {
        const size_t nsz = osz > sz ? sz : osz;
        if (!memmove(nptr, ptr, nsz)) {
            _memory_free_aligned(nptr);
        }

        _memory_free_aligned(ptr);
    }

    return nptr;
}

#define VIEWER_ALIGNED_MALLOC _memory_alloc_aligned
#define VIEWER_ALIGNED_REALLOC _memory_realloc_aligned
#define VIEWER_ALIGNED_FREE _memory_free_aligned

#endif // VIEWER_ALIGNED_MALLOC/REALLOC/FREE

void * memory_aligned_malloc(size_t sz, size_t al) {
    return VIEWER_ALIGNED_MALLOC(sz, al);
}

void * memory_aligned_calloc(size_t cnt, size_t sz, size_t al) {
    size_t size = cnt * sz;
    void* data = memory_aligned_malloc(size, al);
    if (data != NULL) {
        memset(data, 0, size);
    }

    return data;
}

void * memory_aligned_realloc(void* ptr, size_t sz, size_t al) {
    return VIEWER_ALIGNED_REALLOC(ptr, sz, al);
}

void * memory_malloc(size_t sz) {
    return memory_aligned_malloc(sz, MEMORY_DEFAULT_ALIGNMENT);
}

void * memory_calloc(size_t cnt, size_t sz) {
    return memory_aligned_calloc(cnt, sz, MEMORY_DEFAULT_ALIGNMENT);
}

void * memory_realloc(void* ptr, size_t sz) {
    return memory_aligned_realloc(ptr, sz, MEMORY_DEFAULT_ALIGNMENT);
}

void memory_free(void* ptr) {
    VIEWER_ALIGNED_FREE(ptr);
}
