//------------------------------------------------------------------------------
//  cdbgui.cc
//  Implementation file for the generic debug UI overlay, using the
//  C sokol_cimgui.h, sokol_gfx_cimgui.h and the C Dear ImGui bindings
//  cimgui.h
//------------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>

#include "sokol_gfx.h"
#include "sokol_app.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui/cimgui.h"

#define SOKOL_CIMGUI_IMPL
#include "sokol_cimgui.h"

#include "sgui.h"

#if defined(__cplusplus)
extern "C" {
#endif

static sgui_desc_t sgui_descs[SGUI_MAX_DESCRIPTORS] = {0};
static size_t sgui_desc_count = 0;

#define SGUI_CALL_DESC_FUNC(funcName) {\
for(size_t i = 0; i < sgui_desc_count; ++i) {\
    const sgui_desc_t* desc = &sgui_descs[i];\
    if (desc->funcName)\
        desc->funcName();\
}}

void sgui_setup(int sample_count, float dpi_scale, const sgui_desc_t** descs) {    
    fprintf(stdout, "SGUI: Init (samples=%d, scale=%.2f)\n",
        sample_count, dpi_scale);

    if (descs == NULL) {
        return;
    }

    // copy descriptors into global registers
    sgui_desc_count = 0;
    for (sgui_desc_count = 0;
        sgui_desc_count < SGUI_MAX_DESCRIPTORS; ++sgui_desc_count) {
        // break if encounter a null descriptor
        if (descs[sgui_desc_count] == NULL) {
            break;
        }

        sgui_descs[sgui_desc_count] = *(descs[sgui_desc_count]);
    }

    fprintf(stdout, "SGUI: Added %lld sgui_desc_t\n", sgui_desc_count);
    SGUI_CALL_DESC_FUNC(init_cb)

    // setup the sokol-imgui utility header
    scimgui_desc_t scimgui_desc = {0};
    scimgui_desc.sample_count = sample_count;
    scimgui_desc.dpi_scale = dpi_scale;
    scimgui_setup(&scimgui_desc);
}

void sgui_shutdown() {
    scimgui_shutdown();
    SGUI_CALL_DESC_FUNC(shutdown_cb)
}

void sgui_draw_menu() {
    bool exit_app = false;
    if (igBeginMainMenuBar()) {
        if (igBeginMenu("Viewer", true)) {

            igMenuItemBoolPtr("Exit", "Esc", &exit_app, true);
            igEndMenu();
        }
        
        SGUI_CALL_DESC_FUNC(menu_cb)
        igEndMainMenuBar();
    }
           
    if (exit_app) {
        exit(EXIT_SUCCESS);
    }
}

void sgui_draw(bool show_menu) {
    scimgui_new_frame(sapp_width(), sapp_height(), 1.0/60.0);

    if (show_menu) {
        sgui_draw_menu();
    }
    
    SGUI_CALL_DESC_FUNC(draw_cb)
    scimgui_render();
}

void sgui_event(const sapp_event* e) {
    scimgui_handle_event(e);
}

#if defined(__cplusplus)
} // extern C
#endif
