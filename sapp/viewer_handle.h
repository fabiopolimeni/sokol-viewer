#pragma once
/**
 * Common way to manage handles
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define HANDLE_INVALID_ID (-1)

#if defined(__cplusplus)
extern "C" {
#endif

struct handle_t {
    int32_t id;
};

bool handle_is_valid(struct handle_t handle, int32_t max_count);

#define TRACE_MAX_NAME_CHARS 32

typedef struct {
    char name[TRACE_MAX_NAME_CHARS];
} trace_t;

void trace_printf(trace_t* dst, const char* fmt, ...);
void trace_copy(trace_t* dst, const trace_t* src);

#if defined(__cplusplus)
}
#endif
