#pragma once
 /**
  * Wavefront obj loader
  */

#include "viewer_geometry_pass.h"
#include "viewer_memory.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    vertex_t* vertices;
    int32_t num_vertices;
    uint32_t* indices;
    uint32_t num_indices;
} wavefront_mesh_t;

typedef struct {
    box_t bbox;
    rect_t image_tile;
    uint32_t base_vertex_id;
    uint32_t num_vertices;
    uint32_t base_face_id;
    uint32_t num_faces;
    trace_t trace;
} wavefront_shape_t;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint32_t* pixels;
} wavefront_image_t;

typedef struct {
    memory_allocator_t allocator;
    wavefront_mesh_t* mesh;
    wavefront_image_t* diffuseRGB_alphaA;
    wavefront_image_t* emissiveXYZ_specularW;
    wavefront_image_t* normalXY_dispZ_aoW;
    wavefront_shape_t* shapes;
    int32_t num_shapes;
    trace_t trace;
} wavefront_model_t;

typedef enum {
    WAVEFRONT_IMPORT_AS_IS              = 0x00,
    WAVEFRONT_IMPORT_TRIANGULATE        = 0x01,
    WAVEFRONT_IMPORT_CALC_NORMALS       = 0x02,
    WAVEFRONT_IMPORT_FLIP_NORMALS       = 0x04,
    WAVEFRONT_IMPORT_IGNORE_TEXTURES    = 0x08,
    WAVEFRONT_IMPORT_REWIND_FACES       = 0x10,
    WAVEFRONT_IMPORT_DEFAULT            = 
       WAVEFRONT_IMPORT_TRIANGULATE
} wavefront_import_options_t;

typedef struct {
    memory_allocator_t allocator;
    const void* obj_data;
    int32_t data_size;
    int32_t atlas_width;
    int32_t atlas_height;
    uint8_t import_options;
    const char* label;
} wavefront_data_t;

typedef enum {
    WAVEFRONT_RESULT_OK,
    WAVEFRONT_RESULT_ATLAS_OUT_OF_SIZE,
    WAVEFRONT_RESULT_FACES_OUT_OF_RANGE,
    WAVEFRONT_RESULT_MESH_MALFORMED,
    WAVEFRONT_RESULT_INVALID_OBJECT
} wavefront_result_t;

wavefront_result_t wavefront_parse_obj(const wavefront_data_t* data,
    wavefront_model_t* out);

void wavefront_release_obj(wavefront_model_t* obj);

// because each model can store only one texture per object,
// if a multiple texture/material object is provided, then,
// these need to be combined into a single atlas texture,
// and vertices' uvs need to be adjusted accordingly.
model_id_t wavefront_make_model(geometry_pass_t* pass,
    const wavefront_model_t* model);

#if defined(__cplusplus)
}
#endif
