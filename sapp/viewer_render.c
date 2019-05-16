#include "viewer_render.h"

#include <assert.h>
#include <string.h> // memset

#if defined(__cplusplus)
extern "C" {
#endif

const static draw_call_t draw_call_empty = {0};

void draw_call_reset(draw_call_t* dc) {
    memcpy(dc, &draw_call_empty, sizeof(draw_call_t));
}

bool draw_call_is_empty(const draw_call_t* dc) {
    return memcmp(dc, &draw_call_empty, sizeof(draw_call_t)) == 0;
}

void render_draw(const viewport_desc_t* vp,
    const render_pass_t* passes, int32_t count) {
    assert(vp && passes && count > 0);

    sg_apply_viewport(vp->x, vp->y, vp->w, vp->h, true);

    for (int32_t i = 0; i < count; ++i) {
        const render_pass_t* pass = &passes[i];

        // pipeline
        sg_apply_pipeline(pass->pipeline);

        // vertex stage uniforms
        const ubo_t* vs_ubo = &pass->uniforms.vs_ubo;
        if (vs_ubo->data && vs_ubo->size > 0 && vs_ubo->index >= 0) {
            sg_apply_uniforms(SG_SHADERSTAGE_VS,
                vs_ubo->index, vs_ubo->data, vs_ubo->size);
        }

        // fragment stage uniforms
        const ubo_t* fs_ubo = &pass->uniforms.fs_ubo;
        if (fs_ubo->data && fs_ubo->size > 0 && fs_ubo->index >= 0) {
            sg_apply_uniforms(SG_SHADERSTAGE_FS,
                fs_ubo->index, fs_ubo->data, fs_ubo->size);
        }

        // iterate through the draw calls, apply
        // corresponding binding data, and draw
        for (int32_t j = 0; j < RENDER_PASS_MAX_DRAW_CALLS; ++j) {
            const draw_call_t* draw_call = &pass->draws[j];
            if (!draw_call_is_empty(draw_call)) {
                sg_apply_bindings(&draw_call->bindings);
                sg_draw(draw_call->indices_offset, draw_call->num_indices,
                    draw_call->num_instances);
            }
        }
    }
}

#if defined(__cplusplus)
} // extern "C"
#endif
