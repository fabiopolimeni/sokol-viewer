#include "viewer_wavefront.h"
#include "viewer_file.h"
#include "viewer_memory.h"
#include "viewer_log.h"

#include "tinyobj_loader_c.h"

#include <assert.h>
#include <string.h>

#if defined(__cplusplus)
extern "C" {
#endif

static void __wf_print_shapes(tinyobj_shape_t* shapes, size_t num_shapes) {
    for (size_t i = 0; i < num_shapes; i++) {
        LOG_INFO("\tshape[%zd]: name=%s, face(offset=%d, length=%d)\n",
            i, shapes[i].name, shapes[i].face_offset, shapes[i].length);
    }
}

wavefront_result_t wavefront_parse_obj(const wavefront_data_t* data,
    wavefront_model_t* model) {
    assert(data && model);

    tinyobj_attrib_t attribs;
    
    tinyobj_shape_t* shapes = NULL;
    size_t num_shapes = 0;

    tinyobj_material_t* materials = NULL;
    size_t num_materials = 0;

    // load wavefront data
    {
        if (!(data->import_options & WAVEFRONT_IMPORT_TRIANGULATE)) {
            LOG_WARN("WARN: Non triangulated is not supported yet¬\n");
            return WAVEFRONT_RESULT_INVALID_OBJECT;
        }

        int32_t parse_result = tinyobj_parse_obj(
            &attribs, &shapes, &num_shapes, &materials,
            &num_materials, data->obj_data, data->data_size,
            TINYOBJ_FLAG_TRIANGULATE);

        if (parse_result != TINYOBJ_SUCCESS) {
            return WAVEFRONT_RESULT_INVALID_OBJECT;
        }

        LOG_INFO("Wavefront parsed object (shapes=%zd, materials=%zd)\n",
            num_shapes, num_materials);

        if (shapes != NULL && num_shapes > 0) {
            __wf_print_shapes(shapes, num_shapes);
        }
        else {
            return WAVEFRONT_RESULT_INVALID_OBJECT;
        }
    }

    assert(attribs.num_vertices && attribs.num_faces);

    wavefront_mesh_t* mesh = data->allocator(NULL, sizeof(wavefront_mesh_t));
    mesh->num_indices = attribs.num_faces;
    mesh->num_vertices = attribs.num_vertices;

    mesh->vertices = data->allocator(NULL, sizeof(vertex_t) * mesh->num_vertices);
    mesh->indices = data->allocator(NULL, sizeof(uint32_t) * mesh->num_indices);

    uint32_t i_idx = 0;
    uint32_t face_offset = 0;

    assert(attribs.num_face_num_verts > 0);
    uint32_t num_surface_lines = (uint32_t)attribs.num_face_num_verts;
    for (uint32_t i = 0; i < num_surface_lines; ++i) {

        // number of vertices per face, likely 3 or 6
        uint32_t surface_verts = (uint32_t)attribs.face_num_verts[i];
        
        // assume all triangle faces because
        // of TINYOBJ_FLAG_TRIANGULATE
        assert(surface_verts % 3 == 0);

        // e.g. 2 triangles per quad
        uint32_t num_surface_triangles = surface_verts / 3;
        for (uint32_t f = 0; f < num_surface_triangles; ++f) {
            uint32_t f_idx = 3 * f + face_offset;

            // read all 3 vertex info at once
            tinyobj_vertex_index_t idx0 = attribs.faces[f_idx + 0];
            tinyobj_vertex_index_t idx1 = attribs.faces[f_idx + 1];
            tinyobj_vertex_index_t idx2 = attribs.faces[f_idx + 2];

            // populate element indices array
            mesh->indices[i_idx++] = idx0.v_idx;
            mesh->indices[i_idx++] = idx1.v_idx;
            mesh->indices[i_idx++] = idx2.v_idx;

            // normals, and tex coordinates can be <= positions.
            // this allows us to duplicate normals and uv
            // into the mesh vertex buffer without taking
            // into consideration whether a normal or a set
            // of uv have already been read.
            assert(idx0.v_idx < mesh->num_vertices);
            assert(idx1.v_idx < mesh->num_vertices);
            assert(idx1.v_idx < mesh->num_vertices);

            // set vertices positions
            mesh->vertices[idx0.v_idx].pos = (vec3f_t){
                .x = attribs.vertices[3 * idx0.v_idx + 0],
                .y = attribs.vertices[3 * idx0.v_idx + 1],
                .z = attribs.vertices[3 * idx0.v_idx + 2]
            };
            mesh->vertices[idx1.v_idx].pos = (vec3f_t){
                .x = attribs.vertices[3 * idx1.v_idx + 0],
                .y = attribs.vertices[3 * idx1.v_idx + 1],
                .z = attribs.vertices[3 * idx1.v_idx + 2]
            };
            mesh->vertices[idx2.v_idx].pos = (vec3f_t){
                .x = attribs.vertices[3 * idx2.v_idx + 0],
                .y = attribs.vertices[3 * idx2.v_idx + 1],
                .z = attribs.vertices[3 * idx2.v_idx + 2]
            };

            // set normals if provided and the
            // flag on import options is not set
            if (attribs.num_normals &&
                !(data->import_options & WAVEFRONT_IMPORT_CALC_NORMALS)) {

                // if normals are provided, then all of them must be
                assert(idx0.vn_idx >= 0 && idx1.vn_idx >= 0 && idx2.vn_idx >= 0);
                mesh->vertices[idx0.v_idx].norm = (vec3f_t){
                    .x = attribs.normals[3 * idx0.vn_idx + 0],
                    .y = attribs.normals[3 * idx0.vn_idx + 1],
                    .z = attribs.normals[3 * idx0.vn_idx + 2]
                };
                mesh->vertices[idx1.v_idx].norm = (vec3f_t){
                    .x = attribs.normals[3 * idx1.vn_idx + 0],
                    .y = attribs.normals[3 * idx1.vn_idx + 1],
                    .z = attribs.normals[3 * idx1.vn_idx + 2]
                };
                mesh->vertices[idx2.v_idx].norm = (vec3f_t){
                    .x = attribs.normals[3 * idx2.vn_idx + 0],
                    .y = attribs.normals[3 * idx2.vn_idx + 1],
                    .z = attribs.normals[3 * idx2.vn_idx + 2]
                };
            }
            else {
                // set normals to zero, as they
                // will be computed later, once
                // all faces are set finally.
                mesh->vertices[idx0.v_idx].norm = svec3_zero();
                mesh->vertices[idx1.v_idx].norm = svec3_zero();
                mesh->vertices[idx2.v_idx].norm = svec3_zero();
            }

            // set texture coordinates
            if (attribs.num_texcoords > 0) {

                // if texcoords are provided, then all of them must be
                assert(idx0.vt_idx >= 0 && idx1.vt_idx >= 0 && idx2.vt_idx >= 0);
                mesh->vertices[idx0.v_idx].uv = (vec2f_t){
                    .x = attribs.texcoords[2 * idx0.vt_idx + 0],
                    .y = attribs.texcoords[2 * idx0.vt_idx + 1]
                };
                mesh->vertices[idx1.v_idx].uv = (vec2f_t){
                    .x = attribs.texcoords[2 * idx1.vt_idx + 0],
                    .y = attribs.texcoords[2 * idx1.vt_idx + 1]
                };
                mesh->vertices[idx2.v_idx].uv = (vec2f_t){
                    .x = attribs.texcoords[2 * idx2.vt_idx + 0],
                    .y = attribs.texcoords[2 * idx2.vt_idx + 1]
                };
            }
            else {
                // if no texture coordinates are provided,
                // then set uv to zero and recompute them
                // based on material colors, later in the
                // the process.
                mesh->vertices[idx0.v_idx].uv = svec2_zero();
                mesh->vertices[idx1.v_idx].uv = svec2_zero();
                mesh->vertices[idx2.v_idx].uv = svec2_zero();
            }

            // @todo: compute shapes and associate materials
            // @note: materials and shapes are associated to surfaces (i),
            //  not single triangles (f), as one surface line can contain at
            //  most one material, and being part of one object group at a time
        }

        face_offset += surface_verts;
    }

    assert(face_offset == mesh->num_indices);
    assert(i_idx == face_offset);

    if (face_offset > 0)
    {
        model->allocator = data->allocator;
        model->mesh = mesh;
        trace_printf(&model->trace, "%s", data->label);

        return WAVEFRONT_RESULT_OK;
    }

    return WAVEFRONT_RESULT_MESH_MALFORMED;
}

void wavefront_release_obj(wavefront_model_t* model) {
    assert(model && model->mesh);
    
    // release mesh resources
    model->allocator(model->mesh->indices, 0);
    model->allocator(model->mesh->vertices, 0);
    model->allocator(model->mesh, 0);
    model->mesh = NULL;

    // release material's images
    model->allocator(model->diffuseRGB_alphaA, 0);
    model->allocator(model->emissiveXYZ_specularW, 0);
    model->allocator(model->normalXY_dispZ_aoW, 0);

    // release shapes
    model->allocator(model->shapes, 0);
}

model_id_t wavefront_make_model(geometry_pass_t* pass,
    const wavefront_model_t* model) {
    assert(pass && model);

    return geometry_pass_create_model(pass, &(model_desc_t){
        .material = geometry_pass_get_default_material(pass),
        .mesh = geometry_pass_make_mesh(pass, &(mesh_desc_t){
            .vertices = model->mesh->vertices,
            .num_vertices = model->mesh->num_vertices,
            .indices = model->mesh->indices,
            .num_indices = model->mesh->num_indices,
            .label = model->trace.name
        }),
        .label = model->trace.name
    });
}

#if defined(__cplusplus)
}
#endif
