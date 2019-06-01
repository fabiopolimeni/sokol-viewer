#pragma once

#include <stdint.h>
#include "viewer_math.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    bool show_menu;
    bool show_ui;
    bool render_scene;
    
    uint8_t msaa_samples;

    vec2f_t mouse_pos;
    vec2f_t mouse_orbit_pos;
    vec2f_t mouse_panning_pos;
    vec2f_t mouse_scroll;
    float mouse_speed;
    bool mouse_button_pressed[3];
} app_t;

#if defined(__cplusplus)
} // extern "C" {
#endif
