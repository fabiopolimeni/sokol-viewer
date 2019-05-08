#pragma once
/**
 * Define all necessary structure and function, which operate on 3D model space.
 */

#include "viewer_handle.h"
#include "viewer_renderer.h"
#include "viewer_math.h"

#define SCENE_MAX_GROUPS 16     // Max number of groups per scene
#define SCENE_MAX_NODES 128     // Max number of objects per scene

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    mesh_t mesh;
    material_t mat;
    vec4f_t color;
} model_t;

typedef struct handle_t node_id_t;

typedef struct {
    mat4f_t transform;
    model_t model;
    node_id_t parent_id;
} node_t;

typedef struct {
    vec3f_t target;
    vec3f_t eye_pos;
    mfloat_t fov;
    mfloat_t near_plane;
    mfloat_t far_plane;
} camera_t;

typedef struct {
    vec4f_t plane;
} light_t;

typedef struct {
    group_t groups[SCENE_MAX_GROUPS];

    node_t nodes[SCENE_MAX_NODES];
    camera_t camera;
    light_t light;
    mat4f_t root;
} scene_t;

group_id_t scene_create_group(scene_t* scene, mesh_t mesh, material_t mat);
void scene_destroy_group(scene_t* scene, group_id_t group);

instance_id_t scene_add_instance(scene_t* scene,
    group_id_t group, mat4f_t pose, vec4f_t color);

void scene_remove_instance(scene_t* scene,
    group_id_t group, instance_id_t instance);

#if defined(__cplusplus)
} // extern "C"
#endif
