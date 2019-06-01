#pragma once
/*
    The typical debug UI overlay useful for most sokol-app samples
*/
#include "sokol_app.h"

#define SGUI_MAX_DESCRIPTORS 16

#if defined(__cplusplus)
extern "C" {
#endif

typedef void (*sgui_cb)(void* user);

typedef struct {
    sgui_cb init_cb;
    sgui_cb shutdown_cb;
    sgui_cb menu_cb;
    sgui_cb draw_cb;
    void* user_data;
} sgui_desc_t;

void sgui_setup(int sample_count, float dpi_scale, const sgui_desc_t** descs);
void sgui_shutdown();
void sgui_draw(bool show_menu);
void sgui_event(const sapp_event* e);

#if defined(__cplusplus)
} // extern "C"
#endif
