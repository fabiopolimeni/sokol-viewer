#pragma once
/*
    The typical debug UI overlay useful for most sokol-app samples
*/

typedef void (*sgui_cb)();

typedef struct {
    sgui_cb init_cb;
    sgui_cb shutdown_cb;
    sgui_cb menu_cb;
    sgui_cb draw_cb;
} sgui_desc_t;

#define SGUI_MAX_DESCRIPTORS 16

#include "sokol_app.h"

#if defined(__cplusplus)
extern "C" {
#endif

void sgui_setup(int sample_count, float dpi_scale, const sgui_desc_t** descs);
void sgui_shutdown(void);
void sgui_draw(void);
void sgui_event(const sapp_event* e);

#if defined(__cplusplus)
} // extern "C"
#endif
