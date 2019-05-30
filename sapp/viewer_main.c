//------------------------------------------------------------------------------
//  viewer-sapp.c
//------------------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sokol_args.h"
#include "sokol_app.h"
#include "sokol_gfx.h"

#include "cute_path.h"

#include "ui/sgui.h"
#include "ui/sgui_gfx.h"

#include "viewer_log.h"
#include "viewer_math.h"
#include "viewer_render.h"
#include "viewer_scene.h"
#include "viewer_file.h"
#include "viewer_geometry_pass.h"
#include "viewer_memory.h"
#include "viewer_wavefront.h"

#define MSAA_SAMPLES 1
#define MAX_BOXES 10

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

static app_t app = {
    .show_menu = true,
    .show_ui = true,
    .render_scene = true,
    .msaa_samples = MSAA_SAMPLES,
    .mouse_pos = {0.f,0.f},
    .mouse_orbit_pos = {0.f,0.f},
    .mouse_panning_pos = {0.f,0.f},
    .mouse_speed = 0.01f,
    .mouse_button_pressed = {0}
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

static geometry_pass_t geometry_pass = {0};
static scene_t scene = {0};

static material_id_t default_mat_id = {HANDLE_INVALID_ID};

static mesh_id_t box_mesh_id = {HANDLE_INVALID_ID};
static model_id_t box_model_id = {HANDLE_INVALID_ID};
static node_id_t box_node_ids[MAX_BOXES];

static model_id_t wf_model_id = {HANDLE_INVALID_ID};
static node_id_t wf_node_id = {HANDLE_INVALID_ID};

static vec3f_t ambient_color = {0.4f, 0.6f, 0.7f};
//static vec3f_t ambient_color = {1.0f, 1.0f, 1.0f};

static void setup_render() {
    geometry_pass_init(&geometry_pass);

    default_mat_id = geometry_pass_get_default_material(&geometry_pass);
    
    box_mesh_id = geometry_pass_make_mesh_box(&geometry_pass,
        &(mesh_box_desc_t){
           .width = 1.f,
           .height = 1.f,
           .length = 1.f,
           .label = "box-mesh" 
        });

    box_model_id = geometry_pass_create_model(&geometry_pass, 
        &(model_desc_t){
            .mesh = box_mesh_id,
            .material = default_mat_id,
            .label = "box-group"
        });

    wf_model_id = (model_id_t) {HANDLE_INVALID_ID};
}

static void setup_camera() {
    scene.camera = (camera_t){
        .target = (vec3f_t){0.f, 5.f, 0.f},
        .eye_pos = (vec3f_t){10.f, 2.f, 10.f},
        .up_vec = (vec3f_t){0.f,1.f,0.f},
        .fov = 45.0f,
        .near_plane = 0.0f,
        .far_plane = 100.0f,
        .width = (float)sapp_width(),
        .height = (float)sapp_height()
    };
}

static void setup_lights() {
    scene.light = (light_t){
        .plane = (vec4f_t){-1.f, -1.f, -.4f, 0.f},
        .color = ambient_color,
        .intensity = 16.f
    };
}

static void setup_scene() {
    scene_init(&scene);

    setup_camera();
    setup_lights();
    
    scene.root = smat4_identity();
    
    memset(box_node_ids, HANDLE_INVALID_ID, sizeof(box_node_ids));
    wf_node_id = (model_id_t) {HANDLE_INVALID_ID};
}

static void update_scene() {
    scene_update_geometry_pass(&scene, &geometry_pass);
}

static void draw_scene() {
    render_pass_draw(&geometry_pass.render);
}

static void clear_scene() {
    scene_cleanup(&scene);
    memset(&scene, 0, sizeof(scene));
}

static void clear_render() {
    geometry_pass_cleanup(&geometry_pass);
    memset(&geometry_pass, 0, sizeof(geometry_pass));
}

static void reset_app() {
    clear_scene();
    clear_render();
    setup_render();
    setup_scene();
}

// return the index at which the node has
// been added into the box_node_ids array
static int32_t create_box(int32_t parent_id) {
    // find first available box slot
    int32_t box_id = HANDLE_INVALID_ID;
    for (int32_t b = 0; b < MAX_BOXES; ++b) {
        if (!handle_is_valid(box_node_ids[b], SCENE_MAX_NODES)) {
            box_id = b;
            break;
        }
    }

    if (box_id >= 0) {
        trace_t node_trace;
        trace_printf(&node_trace, "box-node-%d", box_id);

        node_id_t parent = {
            .id = HANDLE_INVALID_ID
        };

        if (parent_id > 0 && parent_id < MAX_BOXES) {
            parent = box_node_ids[parent_id];
        }

        box_node_ids[box_id] = scene_add_node(&scene, &(node_desc_t){
            .transform = (transform_t){
                .position = (vec3f_t){
                    .x = rnd(-6.0f, 6.0f),
                    .y = rnd(-3.0f, 3.0f),
                    .z = rnd(-3.0f, 3.0f)
                },
                .scale = (vec3f_t){
                    .x = rnd(0.5f, 2.0f),
                    .y = rnd(0.5f, 2.0f),
                    .z = rnd(0.5f, 2.0f)
                },
                .rotation = squat_from_euler((vec3f_t){
                    .x = rnd(0.f, MPI),
                    .y = rnd(0.f, MPI),
                    .z = rnd(0.f, MPI)
                })
            },
            .color = (vec4f_t){
                .x = rnd(0.2f, 1.0f),   // r
                .y = rnd(0.2f, 1.0f),   // g
                .z = rnd(0.2f, 1.0f),   // b
                .w = (float)(int)rnd(0.5f, 3.5f)    // uv's layer
            },
            .tile = (vec4f_t){
                .x = rnd(2.5f, 0.5f),  // scaling u
                .y = rnd(2.5f, 0.5f),  // scaling v
                .z = 0.0f,  // panning u
                .w = 0.0f   // panning v
            },
            .model = box_model_id,
            .parent = parent,
            .label = node_trace.name
        });

        LOG_INFO("INFO: Box (pos:%d, node:%d) added to the scene",
            box_id, box_node_ids[box_id].id);
    }

    return box_id;
}

// return the number of live box nodes
static int32_t destroy_box(int32_t box_id, bool recursive) {
    int32_t alive_boxes = 0;
    
    // if the passed id is < 0, then, search
    // for the first full slot to free
    if (box_id < 0) {
        for (int32_t b = 0; b < MAX_BOXES; ++b) {
            if (handle_is_valid(box_node_ids[b], SCENE_MAX_NODES)) {
                box_id = b;
                break;
            }
        }
    }

    if (box_id < MAX_BOXES) {
        scene_remove_node(&scene, box_node_ids[box_id], recursive);

        // iterate throught the box nodes to check whether
        // the pointed node is still alive or not, as it can
        // have been removed by the routine, and therefore it
        // needs to be cleared up from the boxes array.
        for (int32_t b = 0; b < MAX_BOXES; ++b) {
            if (!scene_node_is_alive(&scene, box_node_ids[b])) {
                box_node_ids[box_id].id = HANDLE_INVALID_ID;
            }
            else {
                ++alive_boxes;
            }
        }
    }

    return alive_boxes;
}

model_id_t load_wavefront_model(const char* filename) {
    assert(filename);

    file_t file_model = file_open(filename, FILE_OPEN_READ|FILE_OPEN_BINARY);
    if (!file_is_valid(file_model)) {
        return (model_id_t){.id=HANDLE_INVALID_ID};
    }
    
    model_id_t result_model_id = {.id=HANDLE_INVALID_ID};

    char* buffer_data = NULL;
    size_t buffer_size = 0;

    // load file content into a memory buffer
    if (FILE_READALL_OK == file_readall(
        file_model, &buffer_data, &buffer_size, memory_realloc)) {
        assert(buffer_size < INT32_MAX);

        trace_t wf_name;
        path_pop(filename, NULL, wf_name.name);
        path_pop_ext(wf_name.name, wf_name.name, NULL);

        // load wavefront model from memory buffer
        wavefront_model_t wf_model = {0};
        wavefront_result_t wf_result = wavefront_parse_obj(&(wavefront_data_t){
            .allocator = memory_realloc,
            .obj_data = buffer_data,
            .data_size = (int32_t)buffer_size,
            .atlas_width = 1024,
            .atlas_height = 1024,
            .import_options = WAVEFRONT_IMPORT_DEFAULT,
            .label = wf_name.name
        }, &wf_model);

        // accommodate for render model resources
        if (WAVEFRONT_RESULT_OK == wf_result) {
            result_model_id = wavefront_make_model(
                &geometry_pass, &wf_model);
            wavefront_release_obj(&wf_model);
        }
    }

    // release data read from file
    memory_realloc(buffer_data, 0);
    file_close(file_model);
    return result_model_id;
}

node_id_t add_wavefront_to_scene(model_id_t model_id) {
    return scene_add_node(&scene, &(node_desc_t){
        .transform = (transform_t){
            .position = svec3_zero(),
            .scale = svec3_one(),
            .rotation = squat_null()
        },
        .color = svec4(0.6f, 0.6f, 0.6f, 1.0f),
        .tile = (vec4f_t){
            .x = 1.0f,  // scaling u
            .y = 1.0f,  // scaling v
            .z = 0.0f,  // panning u
            .w = 0.0f   // panning v
        },
        .model = model_id,
        .parent = (node_id_t){.id=HANDLE_INVALID_ID},
        .label = "wavefront_node"
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

    const sgui_desc_t* sgui_descs[] = {
        sgui_gfx_get(),
        NULL
    };

    // it is important to initialise the gui BEFORE the graphics 
    sgui_setup(app.msaa_samples, sapp_dpi_scale(), sgui_descs);

    // init scene and graphics resources
    setup_render();
    setup_scene();
}

void frame(void) {
    update_scene();

    render_begin(&(clear_desc_t){
        .color = (vec4f_t){
            .x = ambient_color.x,
            .y = ambient_color.y,
            .z = ambient_color.z,
            .w = 1.0f
        },
        .depth = 1.f,
        .stencil = 0
    }, &(viewport_desc_t) {
        .x = 0, .y = 0,
        .w = sapp_width(),
        .h = sapp_height()
    });

    if (app.render_scene) {
        draw_scene();
    }

    if (app.show_ui) {
        sgui_draw(app.show_menu);
    }

    render_end();
}

void cleanup(void) {
    clear_scene();
    clear_render();

    sgui_shutdown();
    sg_shutdown();
    sargs_shutdown();
}

static void orbit_camera(vec2f_t mouse_pos, vec3f_t eye_target_vec) {
    float disp_x = (app.mouse_orbit_pos.x - mouse_pos.x) * app.mouse_speed;
    float disp_y = (mouse_pos.y - app.mouse_orbit_pos.y) * app.mouse_speed;

    mfloat_t eye_target_len = svec3_length(eye_target_vec);
    vec3f_t eye_target_xz = svec3(eye_target_vec.x, 0.0f, eye_target_vec.z);

    // compute the rotational vector
    // around the horizontal axis.
    vec3f_t eye_target_rot = squat_rotate_vec3(
        squat_from_axis_angle(
            scene.camera.up_vec, disp_x),
        eye_target_xz
    );
    
    // incorporate the vertical displacement,
    // it will automatically limit the angle
    // between -90 and 90 degrees.
    eye_target_rot.y = eye_target_vec.y + disp_y * MPI;

    // project the rotated eye-target vector onto
    // the imaginary sphere centered at zero and
    // of radius equal to the eye-target length.
    // normal(rotated eye position) * eye_target_len
    vec3f_t eye_target_proj = svec3_multiply_f(
        svec3_normalize(eye_target_rot),
        eye_target_len);

    scene.camera.eye_pos = svec3_add(
        scene.camera.target,
        eye_target_proj
    );

    // update orbiting mouse position
    app.mouse_orbit_pos = mouse_pos;
}

static void pan_camera(vec2f_t mouse_pos, vec3f_t eye_target_vec) {
    float disp_x = (app.mouse_panning_pos.x - mouse_pos.x) * app.mouse_speed;
    float disp_y = (mouse_pos.y - app.mouse_panning_pos.y) * app.mouse_speed;

    vec3f_t eye_target_norm = svec3_normalize(eye_target_vec);
    vec3f_t eye_target_xz = svec3(eye_target_vec.x, 0.0f, eye_target_vec.z);

    // move the position and target on the
    // Y along the up vector of the camera
    quat_t zenith_rot = squat_from_vec3(
        svec3_normalize(eye_target_xz),
        eye_target_norm
    );

    // rotated up camera vector
    vec3f_t up_vec = svec3_normalize(
        squat_rotate_vec3(
            zenith_rot,
            svec3_normalize(scene.camera.up_vec)
        ));

    // the cross product of 2 unit vectors is itself a unit vector
    vec3f_t right_vec = svec3_cross(eye_target_norm, up_vec);

    vec3f_t disp_x_vec = svec3_multiply_f(right_vec, -disp_x);
    vec3f_t disp_y_vec = svec3_multiply_f(up_vec, disp_y);
    vec3f_t disp_vec = svec3_add(disp_x_vec, disp_y_vec);

    scene.camera.eye_pos = svec3_add(scene.camera.eye_pos, disp_vec);
    scene.camera.target = svec3_add(scene.camera.target, disp_vec);;
    
    app.mouse_panning_pos = mouse_pos;
}

static void zoom_camera(vec2f_t mouse_scroll, vec3f_t eye_target_vec) {

    // avoid zero singularity
    if (svec3_length(eye_target_vec) > mouse_scroll.y + MFLT_EPSILON) {
        vec3f_t zoom_vec = svec3_multiply_f(
            svec3_normalize(eye_target_vec), -mouse_scroll.y);

        scene.camera.eye_pos = svec3_add(
            scene.camera.eye_pos, zoom_vec);
    }

    app.mouse_scroll = mouse_scroll;
}

static void move_camera_event(const sapp_event* ev) {
    bool mouse_btn_pressed = ev->type == SAPP_EVENTTYPE_MOUSE_DOWN;
    vec2f_t mouse_pos = svec2(ev->mouse_x, ev->mouse_y);

    if (ev->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
        app.mouse_button_pressed[SAPP_MOUSEBUTTON_LEFT] = mouse_btn_pressed;
    }
    
    if (ev->mouse_button == SAPP_MOUSEBUTTON_MIDDLE) {
        app.mouse_button_pressed[SAPP_MOUSEBUTTON_MIDDLE] = mouse_btn_pressed;
        app.mouse_panning_pos = mouse_pos;
    }
    
    if (ev->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {
        app.mouse_button_pressed[SAPP_MOUSEBUTTON_RIGHT] = mouse_btn_pressed;
        app.mouse_orbit_pos = mouse_pos;
    }

    if (ev->type == SAPP_EVENTTYPE_MOUSE_LEAVE) {
        app.mouse_button_pressed[SAPP_MOUSEBUTTON_LEFT] = false;
        app.mouse_button_pressed[SAPP_MOUSEBUTTON_MIDDLE] = false;
        app.mouse_button_pressed[SAPP_MOUSEBUTTON_RIGHT] = false;
    }

    // eye-target vector
    vec3f_t eye_target_vec = svec3_subtract(
        scene.camera.eye_pos, scene.camera.target);

    // orbiting
    if (app.mouse_button_pressed[SAPP_MOUSEBUTTON_RIGHT]) {
        orbit_camera(mouse_pos, eye_target_vec);   
    }
    
    // panning
    if (app.mouse_button_pressed[SAPP_MOUSEBUTTON_MIDDLE]) {
        pan_camera(mouse_pos, eye_target_vec);
    }

    // zooming
    if (ev->type == SAPP_EVENTTYPE_MOUSE_SCROLL) {
        zoom_camera(svec2(ev->scroll_x, ev->scroll_y), eye_target_vec);
    }

    app.mouse_pos = svec2(ev->mouse_x, ev->mouse_y);
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

    // reset scene (R)
    if ((ev->key_code == SAPP_KEYCODE_R)
        && (ev->type == SAPP_EVENTTYPE_KEY_DOWN)) {
        reset_app();
    }

    // reset camera (C)
    if ((ev->key_code == SAPP_KEYCODE_C)
        && (ev->type == SAPP_EVENTTYPE_KEY_DOWN)) {
        setup_camera();
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

    // create a new box node (B)
    if ((ev->key_code == SAPP_KEYCODE_B)
        && (ev->type == SAPP_EVENTTYPE_KEY_DOWN)) {
        create_box(HANDLE_INVALID_ID);
    }

    // add the wavefront model to the scene (W)
    if ((ev->key_code == SAPP_KEYCODE_W)
        && (ev->type == SAPP_EVENTTYPE_KEY_DOWN)) {

        // create model render resource
        if (!handle_is_valid(wf_model_id, GEOMETRY_PASS_MAX_MODELS)) {
            wf_model_id = load_wavefront_model(sargs_value_def("wf",
                "models/cyberpunk_bar/cyberpunk_bar.obj"));
        }

        // add the model to the scene
        if (handle_is_valid(wf_model_id, GEOMETRY_PASS_MAX_MODELS)
            && !handle_is_valid(wf_node_id, SCENE_MAX_NODES)) {
            wf_node_id = add_wavefront_to_scene(wf_model_id);
        }
    }

    move_camera_event(ev);
    sgui_event(ev);
}

void fail(const char* msg) {
    LOG_ERROR("FAIL: %s", msg);
    exit(EXIT_FAILURE);
}

sapp_desc sokol_main(int argc, char* argv[]) {
    // initialise arguments
    sargs_setup(&(sargs_desc) {
        .argc = argc,
        .argv = argv
    });

    if (!sargs_isvalid()) {
        LOG_ERROR("ERROR: Invalid command line\n");
        exit(EXIT_FAILURE);
    }

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
