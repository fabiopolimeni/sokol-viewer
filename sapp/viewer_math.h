#pragma once
/**
 * Define math struct.
 */

#include "mathc.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct vec2i vec2i_t;
typedef struct vec3i vec3i_t;
typedef struct vec4i vec4i_t;
typedef struct mat2i mat2i_t;
typedef struct mat3i mat3i_t;
typedef struct mat4i mat4i_t;

typedef struct vec2 vec2f_t;
typedef struct vec3 vec3f_t;
typedef struct vec4 vec4f_t;
typedef struct mat2 mat2f_t;
typedef struct mat3 mat3f_t;
typedef struct mat4 mat4f_t;

typedef struct quat quat_t;

vec3f_t squat_axis(quat_t rotation);
quat_t squat_from_euler(vec3f_t euler);
vec3f_t squat_to_euler(quat_t rotation);
vec3f_t squat_rotate_vec3(quat_t rotation, vec3f_t vector);

typedef struct {
    vec3f_t position;
    vec3f_t scale;
    quat_t rotation;
} transform_t;

mat4f_t transform_to_mat4(transform_t transform);

typedef struct {
    vec3f_t center;
    vec3f_t extents;
} box_t;

typedef struct {
    vec2f_t min;
    vec2f_t max;
} aabb_t;

typedef struct {
    vec3f_t center;
    mfloat_t radius;
} sphere_t;

// nx + ny + nz + d = 0
typedef struct {
    vec3f_t normal;
    mfloat_t distance;
} plane_t;

vec3f_t plane_project_point(plane_t plane, vec3f_t point);

typedef struct {
    mfloat_t x;
    mfloat_t y;
    mfloat_t w;
    mfloat_t h;
} rect_t;

#if defined(__cplusplus)
} // extern "C"
#endif
