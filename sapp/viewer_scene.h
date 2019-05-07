#pragma once
/**
 * Define all necessary structure and function, which operate on 3D model space.
 */
#include "viewer_renderer.h"
#include "viewer_math.h"

#define SCENE_MAX_INSTANCES 64  // Max number of instances per group
#define SCENE_MAX_GROUPS 16     // Max number of groups per scene
#define SCENE_INVALID_ID (-1)

#if defined(__cplusplus)
extern "C" {
#endif

/* state struct for the 3D object */
typedef struct {
    vec4f_t color;
    mat4f_t pose;
} instance_t;

typedef struct { int32_t id; } group_id_t;
typedef struct { int32_t id; } instance_id_t;

typedef struct {
    mesh_t mesh;
    material_t mat;
    instance_t instances[SCENE_MAX_INSTANCES];
} group_t;

typedef struct {
    mat4f_t proj;
    mat4f_t view;
} camera_t;

typedef struct {
    vec4f_t plane;
} light_t;

typedef struct {
    group_t groups[SCENE_MAX_GROUPS];
    mat4f_t transform;
    camera_t camera;
    light_t light;
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
