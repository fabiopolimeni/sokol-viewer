#pragma once
/*
    The typical debug UI overlay useful for most sokol-app samples
*/
#if defined(USE_DBG_UI)
#include "sokol_app.h"
#if defined(__cplusplus)
extern "C" {
#endif
extern void sgui_setup(int sample_count, float dpi_scale);
extern void sgui_shutdown(void);
extern void sgui_draw(void);
extern void sgui_event(const sapp_event* e);
#if defined(__cplusplus)
} // extern "C"
#endif
#else
static inline void sgui_setup(int sample_count, float dpi_scale) { (void)(sample_count); }
static inline void sgui_shutdown(void) { }
static inline void sgui_draw(void) { }
static inline void sgui_event(const sapp_event* e) { (void)(e); }
#endif
