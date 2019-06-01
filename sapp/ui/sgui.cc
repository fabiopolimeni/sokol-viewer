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

#include "imgui.h"

#define SOKOL_IMGUI_IMPL
#include "sokol_imgui.h"

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
        desc->funcName(desc->user_data);\
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
    simgui_desc_t simgui_desc = {0};
    simgui_desc.sample_count = sample_count;
    simgui_desc.dpi_scale = dpi_scale;
    simgui_setup(&simgui_desc);
}

void sgui_shutdown() {
    simgui_shutdown();
    SGUI_CALL_DESC_FUNC(shutdown_cb)
}

void sgui_draw_menu() {
    if (ImGui::BeginMainMenuBar()) {
       SGUI_CALL_DESC_FUNC(menu_cb)
        ImGui::EndMainMenuBar();
    }
}

void sgui_draw(bool show_menu) {
    simgui_new_frame(sapp_width(), sapp_height(), 1.0/60.0);

    if (show_menu) {
        sgui_draw_menu();
    }
    
    SGUI_CALL_DESC_FUNC(draw_cb)
    simgui_render();
}

void sgui_event(const sapp_event* e) {
    simgui_handle_event(e);
}

#if defined(__cplusplus)
} // extern C
#endif
