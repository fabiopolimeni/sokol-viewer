//------------------------------------------------------------------------------
//  viewer-sapp.c
//------------------------------------------------------------------------------
#include <stdlib.h>

#include "sokol_args.h"
#include "sokol_app.h"
#include "sokol_gfx.h"

#include "ui/sgui.h"
#include "ui/sgui_gfx.h"

#include "viewer-log.h"

#define MSAA_SAMPLES 1

typedef struct {
    sg_buffer vbuf;
    sg_buffer ibuf;
} mesh_t;

static sg_pass_action pass_action;

void init(void) {
    sg_setup(&(sg_desc) {
        .gl_force_gles2 = false,
    #if defined(SOKOL_METAL)
        .mtl_device = sapp_metal_get_device(),
        .mtl_renderpass_descriptor_cb = sapp_metal_get_renderpass_descriptor,
        .mtl_drawable_cb = sapp_metal_get_drawable,
    #elif defined(SOKOL_D3D11)
        .d3d11_device = sapp_d3d11_get_device(),
        .d3d11_device_context = sapp_d3d11_get_device_context(),
        .d3d11_render_target_view_cb = sapp_d3d11_get_render_target_view,
        .d3d11_depth_stencil_view_cb = sapp_d3d11_get_depth_stencil_view
    #endif
    });

    pass_action = (sg_pass_action) {
        .colors[0] = { 
            .action=SG_ACTION_CLEAR,
            .val={0.6f, 0.8f, 0.0f, 1.0f}
        }
    };
    
    const sgui_desc_t* sgui_descs[] = {
        sgui_gfx_get(),
        NULL
    };

    sgui_setup(MSAA_SAMPLES, sapp_dpi_scale(), sgui_descs);
}

void event(const sapp_event* ev) {
    if (ev->key_code == SAPP_KEYCODE_ESCAPE) {
        exit(EXIT_SUCCESS);
    }

    sgui_event(ev);
}

void frame(void) {
    sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());
    sgui_draw();
    sg_end_pass();
    sg_commit();
}

void cleanup(void) {
    sgui_shutdown();
    sg_shutdown();
}

void fail(const char* msg) {
    VIEWER_LOG_ERROR("FAIL: %s", msg);
}

sapp_desc sokol_main(int argc, char* argv[]) {
    return (sapp_desc) {
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event,
        .fail_cb = fail,
        .width = 800,
        .height = 540,
        .high_dpi = true,
        .gl_force_gles2 = false,
        .window_title = "Viewer (sokol app)",
    };
}
