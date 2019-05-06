#pragma once
/**
 * Define all necessary structure and function, which operate on 3D model space.
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
} vertex_t;

/* a mesh consists of a vertex- and index-buffer */
typedef struct {
    sg_buffer vbuf;
    sg_buffer ibuf;
    size_t num_elements;
} mesh_t;

/* state struct for the 3D object */
typedef struct {
    mat4f_t pose;
    vec4f_t color;
} shape_t;

mesh_t mesh_make_cube();

#if defined(__cplusplus)
} // extern "C"
#endif
