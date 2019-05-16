#pragma once
/**
 * Define all necessary structure and function, which operate on 3D model space.
 */

#include "viewer_handle.h"
#include "viewer_math.h"
#include "viewer_geometry_pass.h"

#define SCENE_MAX_NODES 128     // Max number of objects per scene

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct handle_t node_id_t;

typedef struct {
    group_id_t group_id;
} model_t;

typedef struct {
    vec3f_t position;
    vec3f_t scale;
    quat_t rotation;
} transform_t;

typedef struct {
    transform_t transform;
    vec4f_t color;
    model_t model;
    node_id_t parent_id;
    trace_t trace;
} node_t;

typedef struct {
    vec3f_t target;
    vec3f_t eye_pos;
    mfloat_t fov;   // if < 0 => ortho projection
    mfloat_t width;
    mfloat_t height;
    mfloat_t near_plane;
    mfloat_t far_plane;
} camera_t;

typedef struct {
    vec4f_t plane;
    vec3f_t ambient;
    mfloat_t specular;
} light_t;

typedef struct {
    node_t nodes[SCENE_MAX_NODES];
    camera_t camera;
    light_t light;
    mat4f_t root;
} scene_t;

void scene_init(scene_t* scene);
void scene_cleanup(scene_t* scene);

typedef struct {
    transform_t transform;
    vec4f_t color;
    group_id_t group;
    node_id_t parent;
    const char* label;
} node_desc_t;

node_id_t scene_add_node(scene_t* scene, const node_desc_t* desc);
void scene_remove_node(scene_t* scene, node_id_t node);

void scene_update_geometry_pass(const scene_t* scene, geometry_pass_t* pass);

#if defined(__cplusplus)
} // extern "C"
#endif
