#include "sokol_app.h"
#include "sgui_app.h"
#include "../viewer_app.h"

#include "imgui.h"

#include <stdlib.h>
#include <assert.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    bool open;
} sa_imgui_window_t;

typedef struct {
    bool open;
} sa_imgui_input_t;

typedef struct {
    bool open;
} sa_imgui_stats_t;


typedef struct {
    bool open;
} sa_imgui_log_t;

typedef struct {
    sa_imgui_window_t window;
    sa_imgui_input_t input;
    sa_imgui_stats_t stats;
    sa_imgui_log_t log;
    app_t* app;
} sa_imgui_t;

static void sa_imgui_draw_window_content(sa_imgui_t* ctx){
    assert(ctx && ctx->app);
    ImGui::Text("Framebuffer");
    ImGui::Separator();
    ImGui::TextDisabled("Width: %d", sapp_width());
    ImGui::TextDisabled("Height: %d", sapp_height());
    ImGui::TextDisabled("DPI scale: %.2f", sapp_dpi_scale());
    ImGui::TextDisabled("Swap interval: %d", ctx->app->swap_interval);
    ImGui::TextDisabled("MSAA samples: %d", ctx->app->msaa_samples);
}

static void sa_imgui_draw_input_content(sa_imgui_t* ctx){
    assert(ctx && ctx->app);
    if (ImGui::CollapsingHeader("Mouse")) {
        ImGui::TextDisabled("Position: (%04d, %04d)",
            (int32_t)ctx->app->mouse_pos.x,
            (int32_t)ctx->app->mouse_pos.y);
        ImGui::TextDisabled("Scrolling: (%03d, %03d)",
            (int32_t)ctx->app->mouse_scroll.x,
            (int32_t)ctx->app->mouse_scroll.y);
        ImGui::TextDisabled("Buttons:");
        //ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
        bool add_separator = false;
        if (ctx->app->mouse_button_pressed[SAPP_MOUSEBUTTON_LEFT]) {
            ImGui::SameLine();
            ImGui::TextDisabled("LEFT");
            add_separator = true;
        }
        if (ctx->app->mouse_button_pressed[SAPP_MOUSEBUTTON_MIDDLE]) {
            ImGui::SameLine();
            ImGui::TextDisabled("%c MIDDLE", (add_separator) ? '|' : ' ');
            add_separator = true;
        }
        if (ctx->app->mouse_button_pressed[SAPP_MOUSEBUTTON_RIGHT]) {
            ImGui::SameLine();
            ImGui::TextDisabled("%c RIGHT", (add_separator) ? '|' : ' ');
        }
    }

    if (ImGui::CollapsingHeader("Camera")) {
        ImGui::TextDisabled("Orbiting: (%04d, %04d)",
            (int32_t)ctx->app->mouse_orbit_pos.x,
            (int32_t)ctx->app->mouse_orbit_pos.y);
        ImGui::TextDisabled("Panning: (%03d, %03d)",
            (int32_t)ctx->app->mouse_panning_pos.x,
            (int32_t)ctx->app->mouse_panning_pos.y);

        float speed = ctx->app->mouse_speed * 10.f;
        ImGui::DragFloat("Speed", &speed, 0.01f, 0.0f, 1.0f);
        ctx->app->mouse_speed = speed * .1f;
    }
}

static void sa_imgui_draw_stats_content(sa_imgui_t* ctx){

}

static void sa_imgui_draw_log_content(sa_imgui_t* ctx){

}

static void sa_imgui_draw_window_window(sa_imgui_t* ctx){
    if (!ctx->window.open) {
        return;
    }

	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Once);
    if (ImGui::Begin("Window", &ctx->window.open,
        ImGuiWindowFlags_AlwaysAutoResize)) {
        sa_imgui_draw_window_content(ctx);
    }
    ImGui::End();
}

static void sa_imgui_draw_input_window(sa_imgui_t* ctx){
    if (!ctx->input.open) {
        return;
    }
    
    ImGui::SetNextWindowSize(ImVec2(200, 220), ImGuiCond_Once);
    if (ImGui::Begin("Input", &ctx->input.open, ImGuiWindowFlags_None)) {
        sa_imgui_draw_input_content(ctx);
    }
    ImGui::End();
}

static void sa_imgui_draw_stats_window(sa_imgui_t* ctx){
    if (!ctx->stats.open) {
        return;
    }
    
    ImGui::SetNextWindowSize(ImVec2(350, 220), ImGuiCond_Once);
    if (ImGui::Begin("Stats", &ctx->stats.open, 
        ImGuiWindowFlags_AlwaysAutoResize)) {
        sa_imgui_draw_stats_content(ctx);
    }
    ImGui::End();
}

static void sa_imgui_draw_log_window(sa_imgui_t* ctx){
    if (!ctx->log.open) {
        return;
    }
    
    ImGui::SetNextWindowSize(ImVec2(600, 260), ImGuiCond_Once);
    if (ImGui::Begin("Log", &ctx->log.open, 
        ImGuiWindowFlags_AlwaysAutoResize)) {
        sa_imgui_draw_log_content(ctx);
    }
    ImGui::End();
}

static void sa_imgui_draw_settings_menu(sa_imgui_t* ctx){
    assert(ctx&& ctx->app);
    if (ImGui::BeginMenu("Settings")) {
        ImGui::MenuItem("Show Menu", "Alt+M", &ctx->app->show_menu);
        ImGui::MenuItem("Show UI", "Ctrl+G", &ctx->app->show_ui);
        ImGui::MenuItem("Render Scene", "Ctrl+R", &ctx->app->render_scene);
        ImGui::EndMenu();
    }
}

static void sa_imgui_init(sa_imgui_t* ctx) {
    assert(ctx);
    ctx->window.open = false;
    ctx->input.open = false;
    ctx->stats.open = false;
    ctx->log.open = false;
}

static void sa_imgui_discard(sa_imgui_t* ctx){

}

static void sa_imgui_draw(sa_imgui_t* ctx) {
    assert(ctx);
    sa_imgui_draw_window_window(ctx);
    sa_imgui_draw_input_window(ctx);
    sa_imgui_draw_stats_window(ctx);
    sa_imgui_draw_log_window(ctx);
}

static sa_imgui_t sa_imgui;
static sgui_desc_t sgui_app;

static void __setup(void* user) {
    sa_imgui_init(&sa_imgui);
    sa_imgui.app = (app_t*)user;
}

static void __discard(void* user) {
    sa_imgui_discard(&sa_imgui);
}

static void __menu(void* user) {
    bool exit_app = false;

    if (ImGui::BeginMenu("App", true)) {
        ImGui::MenuItem("Window", "Alt+W", &sa_imgui.window.open);
        ImGui::MenuItem("Input", "Alt+I", &sa_imgui.input.open);
        ImGui::MenuItem("Stats", "Alt+S", &sa_imgui.stats.open);
        ImGui::MenuItem("Log", "Alt+L", &sa_imgui.log.open);
        ImGui::Separator();
        sa_imgui_draw_settings_menu(&sa_imgui);
        ImGui::MenuItem("Exit", "Esc", &exit_app);
        ImGui::EndMenu();
    }

    if (exit_app) {
        exit(EXIT_SUCCESS);
    }
}

static void __draw(void* user) {
    sa_imgui_draw(&sa_imgui);
}

const sgui_desc_t* sgui_app_get(void* app) {
    sgui_app.init_cb = __setup;
    sgui_app.shutdown_cb = __discard;
    sgui_app.menu_cb = __menu;
    sgui_app.draw_cb = __draw;
    sgui_app.user_data = app;
    return &sgui_app;
}

#if defined(__cplusplus)
} // extern "C" {
#endif
