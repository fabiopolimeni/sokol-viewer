#include "sokol_app.h"
#include "sgui_app.h"

#include "../viewer_app.h"
#include "../viewer_memory.h"

#include "imgui.h"
#include "imgui_plot.h"

#include <stdlib.h>
#include <stdio.h>
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
    int32_t corner;
    float* update_times_arr;
    float* render_times_arr;
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
    
    ImGui::Separator();
    ImGui::Text("Window");
    ImGui::ColorEdit3("Background", &ctx->app->window_bkg_color.x);
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
            (int32_t)ctx->app->camera_orbit.x,
            (int32_t)ctx->app->camera_orbit.y);
        ImGui::TextDisabled("Panning: (%03d, %03d)",
            (int32_t)ctx->app->camera_panning.x,
            (int32_t)ctx->app->camera_panning.y);

        float speed = ctx->app->camera_speed * 10.f;
        ImGui::DragFloat("Speed", &speed, 0.01f, 0.0f, 1.0f);
        ctx->app->camera_speed = speed * .1f;
    }
}

static void sa_imgui_draw_stats_content(sa_imgui_t* ctx) {
    ImGui::Text("Stats");
    ImGui::Separator();

    float avg_time =
        ctx->app->stats.total_frames_time / ctx->app->stats.max_frames;

    int32_t fps = (avg_time > MFLT_EPSILON) ? (int32_t)1.f/avg_time : 0;

    ImGui::Text("Total Average: %3.1fms/%dfps", avg_time * 1000.f, fps);

    uint32_t n_frames = stats_get_timings(
        &ctx->app->stats,
        ctx->stats.update_times_arr,
        ctx->stats.render_times_arr);

    char label_time[32] = {0};

    float update_time = 
        ctx->app->stats.total_update_time / ctx->app->stats.max_frames;
    sprintf(label_time, "Update: %2.2fms", update_time * 1000.f);
    ImGui::PlotLines(label_time, ctx->stats.update_times_arr, n_frames,
        0, '\0', 0.0f, 0.033f);

    float render_time =
        ctx->app->stats.total_render_time / ctx->app->stats.max_frames;
    sprintf(label_time, "Render: %2.2fms", render_time * 1000.f);
    ImGui::PlotLines(label_time, ctx->stats.render_times_arr, n_frames,
        0, '\0', 0.0f, 0.033f);
        
    if (ImGui::BeginPopupContextWindow()) {
        if (ImGui::MenuItem("Custom",       NULL, ctx->stats.corner == -1))
            ctx->stats.corner = -1;
        if (ImGui::MenuItem("Top-left",     NULL, ctx->stats.corner == 0)) 
            ctx->stats.corner = 0;
        if (ImGui::MenuItem("Top-right",    NULL, ctx->stats.corner == 1)) 
            ctx->stats.corner = 1;
        if (ImGui::MenuItem("Bottom-left",  NULL, ctx->stats.corner == 2)) 
            ctx->stats.corner = 2;
        if (ImGui::MenuItem("Bottom-right", NULL, ctx->stats.corner == 3)) 
            ctx->stats.corner = 3;
        if (ctx->stats.open && ImGui::MenuItem("Close"))
            ctx->stats.open = false;
        ImGui::EndPopup();
    }
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

    const float DISTANCE = 10.0f;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 window_size(
        io.DisplaySize.x / io.DisplayFramebufferScale.x,
        io.DisplaySize.y / io.DisplayFramebufferScale.y);

    if (ctx->stats.corner != -1) {
        ImVec2 window_pos = ImVec2(
            (ctx->stats.corner & 1) ? window_size.x - DISTANCE : DISTANCE,
            (ctx->stats.corner & 2) ? window_size.y - DISTANCE : DISTANCE);

        ImVec2 window_pos_pivot = ImVec2(
            (ctx->stats.corner & 1) ? 1.0f : 0.0f,
            (ctx->stats.corner & 2) ? 1.0f : 0.0f);

        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    }

    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("Stats overlay", &ctx->stats.open,
        (ctx->stats.corner != -1 ? ImGuiWindowFlags_NoMove : 0) |
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav)) {
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
    ctx->log.open = false;

    // stats context
    ctx->stats.open = true;
    ctx->stats.corner = 3;
    ctx->stats.render_times_arr = (float*)memory_calloc(
        ctx->app->stats.max_frames, sizeof(float));
    ctx->stats.update_times_arr = (float*)memory_calloc(
        ctx->app->stats.max_frames, sizeof(float));
}

static void sa_imgui_discard(sa_imgui_t* ctx){
    memory_free(ctx->stats.render_times_arr);
    memory_free(ctx->stats.update_times_arr);
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
    sa_imgui.app = (app_t*)user;
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
