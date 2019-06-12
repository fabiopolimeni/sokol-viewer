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
#include "imgui_font.h"
#include "font_awesome_5.h"

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
    simgui_desc.no_default_font = true;
    simgui_setup(&simgui_desc);

    ImGuiIO& io = ImGui::GetIO();

    // framebuffer
    io.DisplayFramebufferScale = ImVec2(dpi_scale, dpi_scale);

    // font
    ImFontConfig font_config;
    font_config.FontDataOwnedByAtlas = false;
    font_config.OversampleH = 4;
    font_config.OversampleV = 4;
    //font_config.RasterizerMultiply = 1.5f;
    io.Fonts->AddFontFromMemoryTTF(
        dump_font, sizeof(dump_font), 14.0f, &font_config);

    // icons from Font Awesome
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF(
        "fonts/" FONT_ICON_FILE_NAME_FAS, 14.0f,
        &icons_config, icons_ranges);

    // create font texture for the custom font
    unsigned char* font_pixels;
    int font_width, font_height;
    io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_width, &font_height);
    sg_image_desc img_desc = { };
    img_desc.width = font_width;
    img_desc.height = font_height;
    img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    img_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    img_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    img_desc.min_filter = SG_FILTER_LINEAR;
    img_desc.mag_filter = SG_FILTER_LINEAR;
    img_desc.content.subimage[0][0].ptr = font_pixels;
    img_desc.content.subimage[0][0].size = font_width * font_height * 4;
    io.Fonts->TexID = (ImTextureID)(uintptr_t) sg_make_image(&img_desc).id;

    // style
    ImGuiStyle& style = ImGui::GetStyle();

    // style: rounding
    style.WindowRounding = 0.f;
    style.ChildRounding = 0.f;
    style.FrameRounding = 0.f;
    style.PopupRounding = 0.f;
    style.ScrollbarRounding = 0.f;
    style.GrabRounding = 0.f;
    style.TabRounding = 0.f;

    // style: padding and spacing
    style.WindowPadding = ImVec2(6.f, 6.f);
    style.FramePadding = ImVec2(4.f, 2.f);
    style.ItemSpacing = ImVec2(8.f, 4.f);
    style.ItemInnerSpacing = ImVec2(4.f, 4.f);
    style.TouchExtraPadding = ImVec2(2.f, 2.f);
    style.IndentSpacing = 20.f;
    style.ScrollbarSize = 6.f;
    style.GrabMinSize = 4.f;

    // style: borders
    style.WindowBorderSize = 0.f;
    style.ChildBorderSize = 0.f;
    style.PopupBorderSize = 0.f;
    style.FrameBorderSize = 0.f;
    style.TabBorderSize = 0.f;
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
