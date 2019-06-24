#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "viewer_handle.h"

#define LOG_MAX_ENTRIES 256

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
    log_level_trace,
    log_level_info,
    log_level_warn,
    log_level_error
} log_level_t;

typedef struct {
    const char* label;
    const char* file;
    time_t time;
    log_level_t level;
    uint32_t line;
    int32_t text_offset;
    uint32_t msg_len;
} log_entry_t;

typedef struct handle_t log_entry_id_t;

typedef struct {
    log_entry_t* entries[LOG_MAX_ENTRIES];
    char* text_buffer;
    uint32_t buffer_size;
    uint32_t num_entries;
} log_t;

typedef struct {
    char* buffer;
    uint32_t size;
} log_desc_t;

void log_init(log_t* log, const log_desc_t* desc);
void log_cleanup(log_t* log);

log_entry_id_t log_printf(log_t* log,
    const char* label, log_level_t level,
    const char* file, uint32_t line, const char* fmt, ...);

int32_t log_printf_append(log_t* log,
    log_entry_id_t entry, const char* fmt, ...);

typedef bool (*log_it_func)(void* user, const log_entry_t* entry);
void log_iterate(log_t* log, log_it_func cb, void* user);

#if defined(__cplusplus)
} // extern "C" {
#endif

#define LOG_TRACE_PRINTF(log, label, ...) \
    log_printf(log, label, log_level_trace, __FILE__, __LINE__, __VA_ARGS__);
#define LOG_INFO_PRINTF(log, label, ...) \
    log_printf(log, label, log_level_info, __FILE__, __LINE__, __VA_ARGS__);
#define LOG_WARN_PRINTF(log, label, ...) \
    log_printf(log, label, log_level_warn, __FILE__, __LINE__, __VA_ARGS__);
#define LOG_ERROR_PRINTF(log, label, ...) \
    log_printf(log, label, log_level_error, __FILE__, __LINE__, __VA_ARGS__);
#define LOG_APPEND(log, entry, ...) log_printf_append(log, entry, __VA_ARGS__);

#define LOG_INFO(...) fprintf(stdout, __VA_ARGS__)
#define LOG_WARN(...) fprintf(stderr, __VA_ARGS__)
#define LOG_ERROR(...) fprintf(stderr, __VA_ARGS__)
