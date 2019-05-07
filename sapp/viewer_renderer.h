#pragma once
/**
 * High level renderer functionalities and resources
 */
#include "sokol_gfx.h"
#include "viewer_math.h"

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
    size_t num_elements;
} mesh_t;

typedef struct {
    sg_image albedo;
} material_t;

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
