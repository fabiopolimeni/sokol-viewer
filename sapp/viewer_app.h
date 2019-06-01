#pragma once

#include <stdint.h>
#include "viewer_math.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    uint8_t show_menu: 1;
    uint8_t show_ui: 1;
    uint8_t render_scene: 1;
    uint8_t msaa_samples: 4;

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
