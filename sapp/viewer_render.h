#pragma once
/**
 * High level renderer functionalities and resources
 */

#include "sokol_gfx.h"
#include "viewer_handle.h"
#include "viewer_math.h"

#define RENDER_PASS_MAX_DRAW_CALLS 16
#define RENDER_CTX_MAX_PASSES 8

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    int32_t indices_offset;
    int32_t num_indices;
    int32_t num_instances;
    sg_bindings bindings;
} draw_call_t;

void draw_call_reset(draw_call_t* dc);
bool draw_call_is_empty(const draw_call_t* dc);

typedef struct {
    const uint8_t* data;
    int32_t size;
    int32_t index;
} ubo_t;

typedef struct {
    ubo_t vs_ubo;
    ubo_t fs_ubo;
} uniforms_t;

typedef struct {
    sg_shader shader;
    sg_pipeline pipeline;
    uniforms_t uniforms;
    draw_call_t draws[RENDER_PASS_MAX_DRAW_CALLS];
    trace_t trace;
} render_pass_t;

typedef struct {
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
} viewport_desc_t;

typedef struct {
    vec4f_t color;
    float depth;
    uint8_t stencil;
} clear_desc_t;

void render_begin(const clear_desc_t* clear, const viewport_desc_t* vp);
void render_pass_draw(const render_pass_t* pass);
void render_end();

#if defined(__cplusplus)
} // extern "C"
#endif
