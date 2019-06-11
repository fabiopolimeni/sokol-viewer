#pragma once

#include <stdint.h>

#include "viewer_math.h"
#include "viewer_stats.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    bool show_menu;
    bool show_ui;
    bool render_scene;
    
    uint8_t msaa_samples;
    uint8_t swap_interval;

    vec2f_t camera_orbit;
    vec2f_t camera_panning;
    float camera_speed;

    vec2f_t mouse_pos;
    vec2f_t mouse_scroll;
    bool mouse_button_pressed[3];

    stats_t stats;
} app_t;

#if defined(__cplusplus)
} // extern "C" {
#endif
