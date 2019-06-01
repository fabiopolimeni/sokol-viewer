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
} sa_imgui_settings_t;

typedef struct {
    bool open;
} sa_imgui_log_t;

typedef struct {
    sa_imgui_window_t window;
    sa_imgui_input_t input;
    sa_imgui_stats_t stats;
    sa_imgui_settings_t settings;
    sa_imgui_log_t log;
} sa_imgui_t;

static void sa_imgui_draw_window_content(sa_imgui_t* ctx){

}

static void sa_imgui_draw_input_content(sa_imgui_t* ctx){

}

static void sa_imgui_draw_stats_content(sa_imgui_t* ctx){

}

static void sa_imgui_draw_settings_content(sa_imgui_t* ctx){

}

static void sa_imgui_draw_log_content(sa_imgui_t* ctx){

}

static void sa_imgui_draw_window_window(sa_imgui_t* ctx){
    if (!ctx->window.open) {
        return;
    }
	ImGui::SetNextWindowSize(ImVec2(540, 400), ImGuiCond_Once);
    if (ImGui::Begin("Window", &ctx->window.open, ImGuiWindowFlags_None)) {
        sa_imgui_draw_window_content(ctx);
    }
    ImGui::End();
}

static void sa_imgui_draw_input_window(sa_imgui_t* ctx){

}

static void sa_imgui_draw_stats_window(sa_imgui_t* ctx){

}

static void sa_imgui_draw_settings_window(sa_imgui_t* ctx){

}

static void sa_imgui_draw_log_window(sa_imgui_t* ctx){

}

static void sa_imgui_init(sa_imgui_t* ctx) {
    assert(ctx);
    ctx->window.open = false;
    ctx->input.open = false;
    ctx->stats.open = false;
    ctx->settings.open = false;
    ctx->log.open = false;
}

static void sa_imgui_discard(sa_imgui_t* ctx){

}

static void sa_imgui_draw(sa_imgui_t* ctx) {
    assert(ctx);
    sa_imgui_draw_window_window(ctx);
    sa_imgui_draw_input_window(ctx);
    sa_imgui_draw_stats_window(ctx);
    sa_imgui_draw_settings_window(ctx);
    sa_imgui_draw_log_window(ctx);
}

static sa_imgui_t sa_imgui;
static sgui_desc_t sgui_app;

static void __setup(void* user) {
    sa_imgui_init(&sa_imgui);
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
        ImGui::MenuItem("Settings", "Alt+E", &sa_imgui.settings.open);
        ImGui::MenuItem("Log", "Alt+L", &sa_imgui.log.open);
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
