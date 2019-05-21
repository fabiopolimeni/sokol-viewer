#include "sokol_gfx.h"
#include "sokol_app.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui/cimgui.h"

#define SOKOL_GFX_CIMGUI_IMPL
#include "sokol_gfx_cimgui.h"

#include "sgui_gfx.h"

#if defined(__cplusplus)
extern "C" {
#endif

static sg_cimgui_t sg_cimgui;
static sgui_desc_t sgui_gfx;

static void sgui_gfx_setup() {
    sg_cimgui_init(&sg_cimgui);
}

static void sgui_gfx_discard() {
    sg_cimgui_discard(&sg_cimgui);
}

static void sgui_gfx_menu() {
    if (igBeginMenu("Graphics", true)) {
        igMenuItemBoolPtr("Buffers", 0, &sg_cimgui.buffers.open, true);
        igMenuItemBoolPtr("Images", 0, &sg_cimgui.images.open, true);
        igMenuItemBoolPtr("Shaders", 0, &sg_cimgui.shaders.open, true);
        igMenuItemBoolPtr("Pipelines", 0, &sg_cimgui.pipelines.open, true);
        igMenuItemBoolPtr("Passes", 0, &sg_cimgui.passes.open, true);
        igMenuItemBoolPtr("Calls", 0, &sg_cimgui.capture.open, true);
        igEndMenu();
    }
}

static void sgui_gfx_draw() {
    sg_cimgui_draw(&sg_cimgui);
}

const sgui_desc_t* sgui_gfx_get() {
    sgui_gfx.init_cb = sgui_gfx_setup;
    sgui_gfx.shutdown_cb = sgui_gfx_discard;
    sgui_gfx.draw_cb = sgui_gfx_draw;
    sgui_gfx.menu_cb = sgui_gfx_menu;
    return &sgui_gfx;
}

#if defined(__cplusplus)
} // extern C
#endif
