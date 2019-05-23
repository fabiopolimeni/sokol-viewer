#pragma once
 /**
  * Wavefront obj loader
  */

#include "viewer_geometry_pass.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    vertex_t* vertices;
    int32_t num_vertices;
    uint16_t* indices;
    uint16_t num_indices;
} wavefront_mesh_t;

typedef struct {
    const void* obj_data;
    int32_t data_size;
} wavefront_data_t;

typedef struct {
    wavefront_mesh_t* meshes;
    const vec4f_t* colors;
    const char** tex_names;
    const char** shape_names;
    int32_t num_shapes;
} wavefront_obj_t;

wavefront_obj_t wavefront_load_obj(const wavefront_data_t* data);

typedef struct {
    uint16_t width;
    uint16_t height;
    uint32_t* pixels;
} wavefront_image_t;

typedef struct {
    const wavefront_mesh_t* mesh;
    const wavefront_image_t* diffuseRGB_alphaA;
    const wavefront_image_t* emissiveXYZ_specularW;
    const wavefront_image_t* normalXY_dispZ_aoW;
} wavefront_shape_t;

typedef struct {
    const wavefront_shape_t* shapes;
    int32_t num_shapes;
    const char* label;
} wavefront_model_t;

// because each model can store only one texture per object,
// if a multiple texture/material object is provided, then,
// these need to be combined into a single atlas texture,
// and vertices' uvs need to be adjusted accordingly.
model_id_t wavefront_make_model(geometry_pass_t* pass,
    const wavefront_model_t* model);

void wavefront_release_obj(wavefront_obj_t* obj);

#if defined(__cplusplus)
}
#endif
