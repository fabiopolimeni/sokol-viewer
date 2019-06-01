#include "sokol_gfx.h"
#include "sokol_app.h"

#include "imgui.h"

#define SOKOL_GFX_IMGUI_IMPL
#include "sokol_gfx_imgui.h"

#include "sgui_gfx.h"

#if defined(__cplusplus)
extern "C" {
#endif

static sg_imgui_t sg_imgui;
static sgui_desc_t sgui_gfx;

static void sgui_gfx_setup(void* user) {
    sg_imgui_init(&sg_imgui);
}

static void sgui_gfx_discard(void* user) {
    sg_imgui_discard(&sg_imgui);
}

static void sgui_gfx_menu(void* user) {
    if (ImGui::BeginMenu("Graphics", true)) {
        ImGui::MenuItem("Buffers", 0, &sg_imgui.buffers.open);
        ImGui::MenuItem("Images", 0, &sg_imgui.images.open);
        ImGui::MenuItem("Shaders", 0, &sg_imgui.shaders.open);
        ImGui::MenuItem("Pipelines", 0, &sg_imgui.pipelines.open);
        ImGui::MenuItem("Passes", 0, &sg_imgui.passes.open);
        ImGui::MenuItem("Calls", 0, &sg_imgui.capture.open);
        ImGui::EndMenu();
    }
}

static void sgui_gfx_draw(void* user) {
    sg_imgui_draw(&sg_imgui);
}

const sgui_desc_t* sgui_gfx_get() {
    sgui_gfx.init_cb = sgui_gfx_setup;
    sgui_gfx.shutdown_cb = sgui_gfx_discard;
    sgui_gfx.draw_cb = sgui_gfx_draw;
    sgui_gfx.menu_cb = sgui_gfx_menu;
    sgui_gfx.user_data = NULL;
    return &sgui_gfx;
}

#if defined(__cplusplus)
} // extern C
#endif
