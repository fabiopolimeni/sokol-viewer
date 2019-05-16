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
#include "viewer_render.h"
#include "viewer_scene.h"
#include "viewer_geometry_pass.h"

#define MSAA_SAMPLES 1

typedef struct {
    uint8_t show_menu: 1;
    uint8_t show_ui: 1;
    uint8_t render_scene: 1;
    uint8_t msaa_samples: 4;
} app_t;

static app_t app = {
    .show_menu = true,
    .show_ui = true,
    .render_scene = true,
    .msaa_samples = MSAA_SAMPLES
};


// create a checkerboard texture 
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

static render_pass_t render_pass = {0};
// static uniforms_t uniforms = {0};
// static scene_t scene = {0};
// static group_id_t cubes_group;

// #define MAX_CUBE_INSTANCES 5
// static mesh_t cube_mesh = {0};
// static material_t cube_mat = {0};
// static instance_id_t cube_ids[MAX_CUBE_INSTANCES];

// static void setup_scene() {
//     scene.camera = (camera_t){
//         .target = (vec3f_t){0.f, 0.f, 0.f},
//         .eye_pos = (vec3f_t){0.f, 0.f, 10.f},
//         .fov = 45.0f,
//         .near_plane = 0.0f,
//         .far_plane = 100.0f
//     };

//     scene.light.plane = (vec4f_t){-1.f, -1.f, -.4f, 0.f};
//     scene.root = smat4_identity();

//     // create the cube scene group
//     cube_mesh = mesh_make_cube();
//     cube_mat.albedo = sg_make_image(&(sg_image_desc){
//         .width = 4,
//         .height = 4,
//         .pixel_format = SG_PIXELFORMAT_RGBA8,
//         .content.subimage[0][0] = {
//             .ptr = checkerboard_pixels,
//             .size = sizeof(checkerboard_pixels)
//         },
//         .label = "checkerboard-texture"
//     });

//     cubes_group = scene_create_group(&scene, cube_mesh, cube_mat);

//     // create several cube instances
//     for (int32_t i = 0; i < MAX_CUBE_INSTANCES; ++i) {
//         mat4f_t pose = smat4_translate(smat4_identity(), 
//             (vec3f_t) {
//                 .x = rnd(-6.0f, 6.0f),
//                 .y = rnd(-3.0f, 3.0f),
//                 .z = rnd(-3.0f, 3.0f)
//             });

//         vec4f_t color = (vec4f_t) {
//             .x = rnd(0.0f, 1.0f),
//             .y = rnd(0.0f, 1.0f),
//             .z = rnd(0.0f, 1.0f),
//             .w = 1.0f
//         };

//         cube_ids[i] = scene_add_instance(&scene, cubes_group, pose, color);
//     }
// }

// static void setup_renderer() {
//     render_pass.pass_action = (sg_pass_action) {
//         .colors[0] = { 
//             .action=SG_ACTION_CLEAR,
//             .val={0.6f, 0.8f, 0.0f, 1.0f}
//         }
//     };

//     render_pass.shader = sg_make_shader(&(sg_shader_desc) {
//         .attrs = {
//             [ATTR_INDEX_VERTEX_POS] = { .name="vertex_pos", .sem_name="POSITION" },
//             [ATTR_INDEX_VERTEX_NORM] = { .name="vertex_norm", .sem_name="NORMAL" },
//             [ATTR_INDEX_VERTEX_UV] = { .name="vertex_uv", .sem_name="UV" },
//             [ATTR_INDEX_INSTANCE_COLOR] = { .name="instance_color", .sem_name="COLOR" },
//             [ATTR_INDEX_INSTANCE_POSE] = { .name="instance_pose", .sem_name="POSE" },
//             [ATTR_INDEX_INSTANCE_NORMAL] = { .name="instance_normal", .sem_name="INVTRANS_POSE" }
//         },
//         .vs = {
//             .uniform_blocks[0] = {
//                 .size = sizeof(uniforms_t),
//                 .uniforms = {
//                     [UNIFORM_INDEX_VIEW_PROJ] = { .name="view_proj", .type=SG_UNIFORMTYPE_MAT4 },
//                     [UNIFORM_INDEX_LIGHT] = { .name="light", .type=SG_UNIFORMTYPE_FLOAT4 },
//                     [UNIFORM_INDEX_EYE_POS] = { .name="eye_pos", .type=SG_UNIFORMTYPE_FLOAT3 }
//                 }
//             },
//             .source = vs_src
//         },
//         .fs = {
//             .images[SAMPLER_INDEX_ALBEDO] = {
//                 .name = "albedo_tex",
//                 .type = SG_IMAGETYPE_2D
//             },
//             .source = fs_src
//         },
//         .label = "phong-instancing-shader"
//     });

//     render_pass.bindings = (sg_bindings) {
//         // mesh vertex and index buffers
//         .vertex_buffers[BUFFER_INDEX_VERTEX] = cube_mesh.vbuf,
//         .index_buffer = cube_mesh.ibuf,
    
//         // instance buffer goes into vertex-buffer-slot 
//         .vertex_buffers[BUFFER_INDEX_INSTANCE] =
//             sg_make_buffer(&(sg_buffer_desc) {
//                 .size = MAX_CUBE_INSTANCES * sizeof(instance_t),
//                 .usage = SG_USAGE_STREAM,
//                 .label = "instance-data"
//         }),

//         // sampler bindings 
//         .fs_images[SAMPLER_INDEX_ALBEDO] = cube_mat.albedo
//     };

//     render_pass.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
//         .shader = render_pass.shader,
//         .layout = {
//             .buffers[BUFFER_INDEX_VERTEX].step_func = SG_VERTEXSTEP_PER_VERTEX,
//             .buffers[BUFFER_INDEX_INSTANCE].step_func = SG_VERTEXSTEP_PER_INSTANCE,
//             .attrs = {
//                 [ATTR_INDEX_VERTEX_POS] = {.offset = offsetof(vertex_t, pos),.format = SG_VERTEXFORMAT_FLOAT3,.buffer_index = BUFFER_INDEX_VERTEX},
//                 [ATTR_INDEX_VERTEX_NORM] = {.offset = offsetof(vertex_t, norm),.format = SG_VERTEXFORMAT_FLOAT3,.buffer_index = BUFFER_INDEX_VERTEX},
//                 [ATTR_INDEX_VERTEX_UV] = {.offset = offsetof(vertex_t, uv),.format = SG_VERTEXFORMAT_FLOAT2,.buffer_index = BUFFER_INDEX_VERTEX},
//                 [ATTR_INDEX_INSTANCE_COLOR] = {.offset = offsetof(instance_t, color),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
//                 // 4x4 matrices will span 4 attribute slots
//                 [ATTR_INDEX_INSTANCE_POSE] = {.offset = offsetof(instance_t, pose),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
//                 [ATTR_INDEX_INSTANCE_POSE+1] = {.offset = offsetof(instance_t, pose) + (sizeof(mfloat_t) * 4),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
//                 [ATTR_INDEX_INSTANCE_POSE+2] = {.offset = offsetof(instance_t, pose) + (sizeof(mfloat_t) * 8),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
//                 [ATTR_INDEX_INSTANCE_POSE+3] = {.offset = offsetof(instance_t, pose) + (sizeof(mfloat_t) * 12),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
//                 // 4x4 matrices will span 4 attribute slots
//                 [ATTR_INDEX_INSTANCE_NORMAL] = {.offset = offsetof(instance_t, normal),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
//                 [ATTR_INDEX_INSTANCE_NORMAL+1] = {.offset = offsetof(instance_t, normal) + (sizeof(mfloat_t) * 4),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
//                 [ATTR_INDEX_INSTANCE_NORMAL+2] = {.offset = offsetof(instance_t, normal) + (sizeof(mfloat_t) * 8),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
//                 [ATTR_INDEX_INSTANCE_NORMAL+3] = {.offset = offsetof(instance_t, normal) + (sizeof(mfloat_t) * 12),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE}
//             }
//         },
//         .index_type = SG_INDEXTYPE_UINT16,
//         .depth_stencil = {
//             .depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL,
//             .depth_write_enabled = true,
//         },
//         .blend = {
//             .depth_format = SG_PIXELFORMAT_DEPTHSTENCIL,
//         },
//         .rasterizer = {
//             .cull_mode = SG_CULLMODE_BACK,
//             .sample_count = app.msaa_samples
//         },
//         .label = "phong-instancing-pipeline"
//     });
// }

// void update_scene() {
    
//     // @todo: update scene params 
// }

// void render_scene() {
//     const int w = sapp_width();
//     const int h = sapp_height();

//     mat4f_t proj = smat4_perspective_fov(
//         scene.camera.fov, (float)w, (float)h,
//         scene.camera.near_plane, scene.camera.far_plane);

//     mat4f_t view = smat4_look_at(
//         scene.camera.eye_pos, scene.camera.target,
//         (vec3f_t){0.f, 1.f, 0.f});

//     uniforms = (uniforms_t) {
//         .view_proj = smat4_multiply(proj, view),
//         .light = scene.light.plane,
//         .eye_pos = scene.camera.eye_pos
//     };

//     // update instance model, and normal, matrices 
//     instance_t instances[MAX_CUBE_INSTANCES] = {0};
//     const group_t* cubes = &scene.groups[cubes_group.id];
//     for (size_t i = 0; i < MAX_CUBE_INSTANCES; ++i) {
//         const instance_t* cube = &cubes->instances[i];
//         instance_t *instance = &instances[i];
//         instance->color = cube->color;
//         instance->pose = smat4_multiply(scene.root, cube->pose);
//         instance->normal = smat4_transpose(smat4_inverse(instance->pose));
//     }
    
//     // @todo: update instance data of all render groups
    
//     // upload instance data to render
//     sg_update_buffer(
//         render_pass.bindings.vertex_buffers[BUFFER_INDEX_INSTANCE],
//         instances, MAX_CUBE_INSTANCES * sizeof(instance_t));

//     // update render pass dynamics 
//     sg_apply_viewport(0, 0, w, h, true);
//     sg_apply_pipeline(render_pass.pipeline);
//     sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &uniforms, sizeof(uniforms_t));

//     // @todo: apply binding and draw elements for each render group
//     sg_apply_bindings(&render_pass.bindings);

//     // render the cubes group 
//     sg_draw(0, cube_mesh.num_elements, MAX_CUBE_INSTANCES);
// }

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

    const sgui_desc_t* sgui_descs[] = {
        sgui_gfx_get(),
        NULL
    };

    // it is important to initialise the gui BEFORE the graphics 
    sgui_setup(app.msaa_samples, sapp_dpi_scale(), sgui_descs);

    // init graphics resources and params 
    //setup_scene();
    //setup_renderer();
}

void frame(void) {
    //update_scene();

    sg_begin_default_pass(
        &render_pass.pass_action,
        sapp_width(), sapp_height());

    if (app.render_scene) {
        //render_scene();
    }

    if (app.show_ui) {
        sgui_draw(app.show_menu);
    }

    sg_end_pass();
    sg_commit();
}

void cleanup(void) {
    // sg_destroy_buffer(render_pass.bindings.vertex_buffers[BUFFER_INDEX_INSTANCE]);
    // sg_destroy_buffer(render_pass.bindings.vertex_buffers[BUFFER_INDEX_VERTEX]);
    // sg_destroy_buffer(render_pass.bindings.index_buffer);
    // sg_destroy_image(render_pass.bindings.fs_images[SAMPLER_INDEX_ALBEDO]);

    sgui_shutdown();
    sg_shutdown();
}

void event(const sapp_event* ev) {
    // exit application
    if ((ev->key_code == SAPP_KEYCODE_ESCAPE)
        && (ev->type == SAPP_EVENTTYPE_KEY_DOWN)) {
        cleanup();
        exit(EXIT_SUCCESS);
    }

    // toggle menu visibility (Alt)
    if (ev->modifiers & SAPP_MODIFIER_ALT) {
        app.show_menu = !app.show_menu;
    }

    // toggle UI visibility (Ctrl+G)
    if ((ev->key_code == SAPP_KEYCODE_G)
        && (ev->type == SAPP_EVENTTYPE_KEY_DOWN)
        && (ev->modifiers & SAPP_MODIFIER_CTRL)) {
        app.show_ui = !app.show_ui;
    }

    // toggle render scene visibility (Ctrl+R)
    if ((ev->key_code == SAPP_KEYCODE_R)
        && (ev->type == SAPP_EVENTTYPE_KEY_DOWN)
        && (ev->modifiers & SAPP_MODIFIER_CTRL)) {
        app.render_scene = !app.render_scene;
    }

    sgui_event(ev);
}

void fail(const char* msg) {
    LOG_ERROR("FAIL: %s", msg);
    exit(EXIT_FAILURE);
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
