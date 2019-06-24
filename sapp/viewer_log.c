#include "viewer_log.h"

#include <assert.h>
#include <string.h>

#if defined(__cplusplus)
extern "C" {
#endif

void log_init(log_t* log, const log_desc_t* desc) {
    assert(log && desc && desc->buffer && desc->size > 0);
    memset(log->entries, 0, sizeof(log_entry_t) * LOG_MAX_ENTRIES);
    log->num_entries = 0;
    log->text_buffer = desc->buffer;
    log->buffer_size = desc->size;
}

void log_cleanup(log_t* log) {
    assert(log);
    memset(log, 0, sizeof(log_t));
}

log_entry_id_t log_printf(log_t* log,
    const char* label, log_level_t level,
    const char* file, uint32_t line, const char* fmt, ...) {
    assert(log);
    return (log_entry_id_t) { .id =  HANDLE_INVALID_ID };
}

int32_t log_printf_append(log_t* log,
    log_entry_id_t entry, const char* fmt, ...) {
    assert(log);

    return 0;
}

void log_iterate(log_t* log, log_it_func cb, void* user) {

}

#if defined(__cplusplus)
} // extern "C" {
#endif
