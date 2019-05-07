//------------------------------------------------------------------------------
//  viewer-sapp.c
//------------------------------------------------------------------------------
#include <stdlib.h>

#include "sokol_args.h"
#include "sokol_app.h"
#include "sokol_gfx.h"

#include "ui/sgui.h"
#include "ui/sgui_gfx.h"

#include "viewer_log.h"
#include "viewer_math.h"
#include "viewer_renderer.h"
#include "viewer_scene.h"
#include "viewer_shaders.h"

#define MSAA_SAMPLES 1

typedef struct {
    uint8_t show_menu: 1;
    uint8_t show_ui: 1;
    uint8_t msaa_samples: 4;
} app_t;

static app_t app = {
    .show_menu = true,
    .show_ui = true,
    .msaa_samples = MSAA_SAMPLES
};

#define BUFFER_VERTEX_INDEX 0
#define BUFFER_INSTANCE_INDEX 1

static renderer_t renderer = {0};
static scene_t scene = {0};
static group_id_t cubes_group;

#define MAX_CUBE_INSTANCES 3
static mesh_t cube_mesh = {0};
static material_t cube_mat = {0};
static instance_id_t cube_instances[MAX_CUBE_INSTANCES];

/* create a checkerboard texture */
static uint32_t checkerboard_pixels[4*4] = {
    0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF000000,
    0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF,
    0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF, 0xFF000000,
    0xFF000000, 0xFFFFFFFF, 0xFF000000, 0xFFFFFFFF,
};

static inline uint32_t xorshift32(void) {
    static uint32_t x = 0x12345678;
    x ^= x<<13;
    x ^= x>>17;
    x ^= x<<5;
    return x;
}

static inline float rnd(float min_val, float max_val) {
    return ((((float)(xorshift32() & 0xFFFF)) / 0x10000)
        * (max_val - min_val)) + min_val;
}

static void setup_scene() {
    const int w = sapp_width();
    const int h = sapp_height();

    scene.camera.proj = smat4_perspective_fov(
        45.0f, (float)w, (float)h, 0.01f, 100.0f);
    scene.camera.view = smat4_look_at(
        (vec3f_t){0.f, 0.f, 30.f},
        (vec3f_t){0.f, 0.f, 0.f},
        (vec3f_t){0.f, 1.f, 0.f});
    scene.transform = smat4_identity();
    scene.light.plane = (vec4f_t){-1.f, -1.f, 0.f, 0.f};

    // Create the cube scene group
    cube_mesh = mesh_make_cube();
    cube_mat.albedo = sg_make_image(&(sg_image_desc){
        .width = 4,
        .height = 4,
        .content.subimage[0][0] = {
            .ptr = checkerboard_pixels,
            .size = sizeof(checkerboard_pixels)
        },
        .label = "checkerboard-texture"
    });

    cubes_group = scene_create_group(&scene, cube_mesh, cube_mat);

    // Create several cube instances
    for (int32_t i = 0; i < MAX_CUBE_INSTANCES; ++i) {
        cube_instances[i] = scene_add_instance(
            &scene, cubes_group, smat4_identity(), (vec4f_t) {
                .x = rnd(0.0f, 1.0f),
                .y = rnd(0.0f, 1.0f),
                .z = rnd(0.0f, 1.0f),
                .w = 1.0f
            });
    }
}

static void setup_renderer() {
    renderer.pass_action = (sg_pass_action) {
        .colors[0] = { 
            .action=SG_ACTION_CLEAR,
            .val={0.6f, 0.8f, 0.0f, 1.0f}
        }
    };

    renderer.shader = sg_make_shader(&(sg_shader_desc) {
        .attrs = {
            [0] = { .name="vertex_pos", .sem_name="POSITION" },
            [1] = { .name="vertex_norm", .sem_name="NORMAL" },
            [2] = { .name="vertex_uv", .sem_name="UV" },
            [3] = { .name="instance_color", .sem_name="COLOR" },
            [4] = { .name="instance_pose", .sem_name="POSE" }
        },
        .vs = {
            .uniform_blocks[0] = {
                .size = sizeof(scene.camera)
                    + sizeof(scene.transform)
                    + sizeof(scene.light),
                .uniforms = {
                    [0] = { .name="camera_proj", .type=SG_UNIFORMTYPE_MAT4 },
                    [1] = { .name="camera_view", .type=SG_UNIFORMTYPE_MAT4 },
                    [2] = { .name="scene_model", .type=SG_UNIFORMTYPE_MAT4 },
                    [3] = { .name="light", .type=SG_UNIFORMTYPE_FLOAT4 },
                }
            },
            .source = vs_src
        },
        .fs = {
            .images[0] = { .name="albedo_tex", .type = SG_IMAGETYPE_2D },
            .source = fs_src
        },
        .label = "phong-instancing-shader"
    });

    renderer.bindings = (sg_bindings) {
        /* mesh vertex and index buffers */
        .vertex_buffers[BUFFER_VERTEX_INDEX] = cube_mesh.vbuf,
        .index_buffer = cube_mesh.ibuf,
    
        /* instance buffer goes into vertex-buffer-slot */
        .vertex_buffers[BUFFER_INSTANCE_INDEX] =
            sg_make_buffer(&(sg_buffer_desc) {
                .size = MAX_CUBE_INSTANCES * sizeof(instance_t),
                .usage = SG_USAGE_STREAM,
                .label = "instance-data"
        })
    };

    renderer.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
        .shader = renderer.shader,
        .layout = {
            .buffers[BUFFER_VERTEX_INDEX].step_func = SG_VERTEXSTEP_PER_VERTEX,
            .buffers[BUFFER_INSTANCE_INDEX].step_func = SG_VERTEXSTEP_PER_INSTANCE,
            .attrs = {
                [0] = {
                    .offset = offsetof(vertex_t, pos),
                    .format = SG_VERTEXFORMAT_FLOAT3,
                    .buffer_index = BUFFER_VERTEX_INDEX
                },
                [1] = {
                    .offset = offsetof(vertex_t, norm),
                    .format = SG_VERTEXFORMAT_FLOAT3,
                    .buffer_index = BUFFER_VERTEX_INDEX
                },
                [2] = {
                    .offset = offsetof(vertex_t, uv),
                    .format = SG_VERTEXFORMAT_FLOAT2,
                    .buffer_index = BUFFER_VERTEX_INDEX
                },
                [3] = {
                    .offset = offsetof(instance_t, color),
                    .format = SG_VERTEXFORMAT_FLOAT4,
                    .buffer_index = BUFFER_INSTANCE_INDEX
                },
                // 4x4 matrices will span 4 attribute slots
                [4] = {
                    .offset = offsetof(instance_t, pose),
                    .format = SG_VERTEXFORMAT_FLOAT4,
                    .buffer_index = BUFFER_INSTANCE_INDEX
                },
                [5] = {
                    .offset = offsetof(instance_t, pose) + (sizeof(mfloat_t) * 4),
                    .format = SG_VERTEXFORMAT_FLOAT4,
                    .buffer_index = BUFFER_INSTANCE_INDEX
                },
                [6] = {
                    .offset = offsetof(instance_t, pose) + (sizeof(mfloat_t) * 8),
                    .format = SG_VERTEXFORMAT_FLOAT4,
                    .buffer_index = BUFFER_INSTANCE_INDEX
                },
                [7] = {
                    .offset = offsetof(instance_t, pose) + (sizeof(mfloat_t) * 12),
                    .format = SG_VERTEXFORMAT_FLOAT4,
                    .buffer_index = BUFFER_INSTANCE_INDEX
                }
            }
        },
        .index_type = SG_INDEXTYPE_UINT16,
        .depth_stencil = {
            .depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL,
            .depth_write_enabled = true,
        },
        .blend = {
            .depth_format = SG_PIXELFORMAT_DEPTH,
        },
        .rasterizer = {
            .cull_mode = SG_CULLMODE_BACK,
            .sample_count = app.msaa_samples
        },
        .label = "phong-instancing-pipeline"
    });
}

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

    // Init graphics resources and params
    setup_scene();
    setup_renderer();

    const sgui_desc_t* sgui_descs[] = {
        sgui_gfx_get(),
        NULL
    };

    sgui_setup(app.msaa_samples, sapp_dpi_scale(), sgui_descs);
}

void frame(void) {
    sg_begin_default_pass(
        &renderer.pass_action,
        sapp_width(), sapp_height());

    if (app.show_ui) {
        sgui_draw(app.show_menu);
    }

    sg_end_pass();
    sg_commit();
}

void event(const sapp_event* ev) {
    // Exit application
    if ((ev->key_code == SAPP_KEYCODE_ESCAPE)
        && (ev->type == SAPP_EVENTTYPE_KEY_DOWN)) {
        exit(EXIT_SUCCESS);
    }

    // Toggle menu visibility
    if (ev->modifiers & SAPP_MODIFIER_ALT) {
        app.show_menu = !app.show_menu;
    }

    // Toggle UI visibility
    if ((ev->key_code == SAPP_KEYCODE_G)
        && (ev->type == SAPP_EVENTTYPE_KEY_DOWN)
        && (ev->modifiers & SAPP_MODIFIER_CTRL)) {
        app.show_ui = !app.show_ui;
    }

    sgui_event(ev);
}

void cleanup(void) {
    sgui_shutdown();
    sg_shutdown();
}

void fail(const char* msg) {
    LOG_ERROR("FAIL: %s", msg);
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
