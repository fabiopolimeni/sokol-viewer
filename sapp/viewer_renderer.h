#pragma once
/**
 * High level renderer functionalities and resources
 */
#include "sokol_gfx.h"
#include "viewer_math.h"
#include "viewer_handle.h"

#define RENDERER_MAX_INSTANCES 64  // Max number of instances per group

#if defined(__cplusplus)
extern "C" {
#endif

/* vertex positions and normals for simple point lighting */
typedef struct {
    vec3f_t pos;
    vec3f_t norm;
    vec2f_t uv;
} vertex_t;

/* a mesh consists of a vertex- and index-buffer */
typedef struct {
    sg_buffer vbuf;
    sg_buffer ibuf;
    int32_t num_elements;
} mesh_t;

typedef struct {
    sg_image albedo;
} material_t;

/* state struct for the 3D object */
typedef struct {
    vec4f_t color;
    mat4f_t pose;
    mat4f_t normal;
} instance_t;

typedef struct handle_t group_id_t;
typedef struct handle_t instance_id_t;

typedef struct {
    mesh_t mesh;
    material_t mat;
    instance_t instances[RENDERER_MAX_INSTANCES];
} group_t;

typedef struct {
    mat4f_t view_proj;
    vec4f_t light;
    vec3f_t eye_pos;
} uniforms_t;

typedef struct {
    sg_pass_action pass_action;
    sg_shader shader;
    sg_bindings bindings;
    sg_pipeline pipeline;
} renderer_t;

mesh_t mesh_make_cube();
void mesh_destroy(mesh_t mesh);

#if defined(__cplusplus)
} // extern "C"
#endif
