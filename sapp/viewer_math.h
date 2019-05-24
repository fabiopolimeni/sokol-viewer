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

quat_t squat_from_euler(vec3f_t euler);
vec3f_t squat_to_euler(quat_t rotation);

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
    float x;
    float y;
    float w;
    float h;
} rect_t;

#if defined(__cplusplus)
} // extern "C"
#endif
