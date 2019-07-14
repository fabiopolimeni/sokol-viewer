#include "viewer_handle.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#if defined(__cplusplus)
extern "C" {
#endif

bool handle_is_valid(struct handle_t handle, int32_t max_count) {
    return (handle.id >= 0 && handle.id < max_count);
}

void trace_printf(trace_t* dst, const char* fmt, ...) {
    assert(dst);
    memset(dst->name, 0, TRACE_MAX_NAME_CHARS);
    if (fmt) {
        va_list va;
        va_start(va, fmt);
        #if defined(_MSC_VER)
        vsnprintf_s(dst->name, TRACE_MAX_NAME_CHARS,
            TRACE_MAX_NAME_CHARS, fmt, va);
        #else
        vsnprintf(dst->name, TRACE_MAX_NAME_CHARS, fmt, va);
        #endif
        va_end(va);

        // make the sting always null terminated
        dst->name[TRACE_MAX_NAME_CHARS-1] = 0;
    }
}

void trace_copy(trace_t* dst, const trace_t* src) {
    assert(dst && src);
    memcpy(dst->name, src->name, TRACE_MAX_NAME_CHARS);
}

#if defined(__cplusplus)
} // extern "C"
#endif
