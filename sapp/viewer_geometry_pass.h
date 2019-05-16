#pragma once
 /**
  * Geometry render pass
  */

#include "sokol_gfx.h"
#include "viewer_math.h"
#include "viewer_handle.h"
#include "viewer_render.h"

#define GEOMETRY_PASS_MAX_MESHES 32     // max number of meshes per pass
#define GEOMETRY_PASS_MAX_MATERIALS 16  // max number of materials per pass
#define GEOMETRY_PASS_MAX_INSTANCES 64  // max number of instances per group
#define GEOMETRY_PASS_MAX_GROUPS RENDER_PASS_MAX_DRAW_CALLS

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct handle_t mesh_id_t;
typedef struct handle_t material_id_t;
typedef struct handle_t group_id_t;

// -----------------------------------------------------------------------------
// Structures
// -----------------------------------------------------------------------------

// vertex positions and normals for simple point lighting
typedef struct {
    vec3f_t pos;
    vec3f_t norm;
    vec2f_t uv;
} vertex_t;

 // a mesh consists of a vertex- and index-buffer  
typedef struct {
    uint16_t num_elements;
    sg_buffer vbuf;
    sg_buffer ibuf;
    trace_t trace;
} mesh_t;

typedef struct {
    vec4f_t ambient_spec;   // rgb: ambient color, a: specular power
    sg_image albedo_rough;  // rgb: albedo, a: roughness
    trace_t trace;
} material_t;

typedef struct {
    vec4f_t color;
    mat4f_t pose;
    mat4f_t normal;
} instance_t;

typedef struct {
    mesh_id_t mesh_id;
    material_id_t material_id;
    trace_t trace;
} group_t;

typedef struct {
    mat4f_t view_proj;
    vec4f_t light;
    vec3f_t eye_pos;
    vec4f_t ambient_spec;
} globals_t;

typedef struct {
    mesh_t meshes[GEOMETRY_PASS_MAX_MESHES];
    material_t materials[GEOMETRY_PASS_MAX_MATERIALS];
    group_t groups[GEOMETRY_PASS_MAX_GROUPS];
    globals_t globals;
    render_pass_t render;
} geometry_pass_t;

// -----------------------------------------------------------------------------
// Intrface
// -----------------------------------------------------------------------------

typedef struct {
    const vertex_t* vertices;
    int32_t num_vertices;
    const uint16_t* indices;
    uint16_t num_indices;
    const char* label;
} mesh_desc_t;

mesh_id_t geometry_pass_make_mesh(geometry_pass_t* pass, 
    const mesh_desc_t* mesh_desc);

typedef struct {
    mfloat_t width;
    mfloat_t height;
    mfloat_t length;
    const char* label;
} mesh_box_desc_t;

mesh_id_t geometry_pass_make_mesh_box(geometry_pass_t* pass,
    const mesh_box_desc_t* box);

void geometry_pass_destroy_mesh(geometry_pass_t* pass, mesh_id_t mesh);

// pixel format is assumed RGBA8
// the size will be computed as:
// width*height*sizeof(uint32_t)
typedef struct {
    uint16_t width;
    uint16_t height;
    uint32_t* pixels;
    vec4f_t ambient_spec;
    const char* label;
} material_desc_t;

material_id_t geometry_pass_make_material(geometry_pass_t* pass,
    const material_desc_t* material_desc);

material_id_t geometry_pass_make_material_default(geometry_pass_t* pass);

void geometry_pass_destroy_material(geometry_pass_t* pass, material_id_t mat);

typedef struct {
    mesh_id_t mesh;
    material_id_t material;
    const char* label;
} group_desc_t;

group_id_t geometry_pass_create_group(geometry_pass_t* pass,
    const group_desc_t* group_desc);

void geometry_pass_destroy_group(geometry_pass_t* pass, group_id_t group);

void geometry_pass_update_group(geometry_pass_t* pass,
    group_id_t group, const instance_t* instances, uint32_t count);

void geometry_pass_init(geometry_pass_t* pass);

void geometry_pass_cleanup(geometry_pass_t* pass);

#if defined(__cplusplus)
}
#endif
