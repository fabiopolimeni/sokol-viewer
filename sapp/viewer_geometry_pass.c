#include "viewer_geometry_pass.h"
#include "viewer_log.h"
#include "shaders/geometry_pass.glsl.h"

#include <stddef.h>
#include <assert.h>
#include <string.h> // memcmp

#define BUFFER_INDEX_VERTEX 0
#define BUFFER_INDEX_INSTANCE 1
#define RASTERIZER_MSAA_SAMPLES 1

#if defined(__cplusplus)
extern "C" {
#endif

const static mesh_t empty_mesh = {
    .num_elements = 0,
    .vbuf = -1,
    .ibuf = -1,
    .trace = {0}
};

static bool mesh_is_empty(const mesh_t* mesh) {
    return memcmp(mesh, &empty_mesh, sizeof(mesh_t)) == 0;
}

const static material_t empty_material = {
    .albedo_transparency = -1,
    .emissive_specular = -1,
    .trace = {0}
};

static bool material_is_empty(const material_t* material) {
    return memcmp(material, &empty_material, sizeof(material_t)) == 0;
}

const static model_t empty_model = {
    .mesh_id = {.id = HANDLE_INVALID_ID},
    .material_id = {.id = HANDLE_INVALID_ID},
    .trace = {0}
};

static bool model_is_empty(const model_t* model) {
    return memcmp(model, &empty_model, sizeof(model_t)) == 0;
}

mesh_id_t geometry_pass_make_mesh(geometry_pass_t* pass, 
    const mesh_desc_t* mesh_desc) {
    assert(pass && mesh_desc);
    assert(mesh_desc->vertices && mesh_desc->num_vertices > 0);
    assert(mesh_desc->indices && mesh_desc->num_indices > 0);
    
    mesh_id_t mesh_id = {
        .id = HANDLE_INVALID_ID
    };

    // search for an empty slot
    for (int32_t i = 0; i < GEOMETRY_PASS_MAX_MESHES; ++i) {

        // if we find an empty slot, store the index and break the loop
        if (mesh_is_empty(&pass->meshes[i])) {
            mesh_id.id = i;
            break;
        }
    }

    // no free slot found
    if (mesh_id.id == HANDLE_INVALID_ID) {
        return mesh_id;
    }

    uint16_t indices_array_size = sizeof(uint16_t) * mesh_desc->num_indices;
    
    // create temporary buffer traces labels
    trace_t vb_trace;
    trace_printf(&vb_trace, "%s-%s", mesh_desc->label, "vertex-buffer");

    trace_t ib_trace;
    trace_printf(&ib_trace, "%s-%s", mesh_desc->label, "index-buffer");

    mesh_t mesh = {
        .vbuf = sg_make_buffer(&(sg_buffer_desc){
            .size = sizeof(vertex_t) * mesh_desc->num_vertices,
            .content = mesh_desc->vertices,
            .label = vb_trace.name
        }),

        .ibuf = sg_make_buffer(&(sg_buffer_desc){
            .type = SG_BUFFERTYPE_INDEXBUFFER,
            .size = indices_array_size,
            .content = mesh_desc->indices,
            .label = ib_trace.name
        }),

        .num_elements = indices_array_size / sizeof(uint16_t)
    };

    trace_printf(&mesh.trace, "%s", mesh_desc->label);

    pass->meshes[mesh_id.id] = mesh;
    return mesh_id;
}

mesh_id_t geometry_pass_make_mesh_box(geometry_pass_t* pass,
    const mesh_box_desc_t* box) {
    assert(pass && box);

    mfloat_t hw = box->width * .5f;
    mfloat_t hh = box->height * .5f;
    mfloat_t hl = box->length* .5f;

    vertex_t vertices[] = {
        //       position             normal              uv      
        { { -hw, -hh, -hl }, { 0.0, 0.0, -1.0 }, { 0.0f, 0.0f } },
        { {  hw, -hh, -hl }, { 0.0, 0.0, -1.0 }, { 1.0f, 0.0f } },
        { {  hw,  hh, -hl }, { 0.0, 0.0, -1.0 }, { 1.0f, 1.0f } },
        { { -hw,  hh, -hl }, { 0.0, 0.0, -1.0 }, { 0.0f, 1.0f } },

        { { -hw, -hh,  hl }, { 0.0, 0.0,  1.0 }, { 0.0f, 0.0f } },
        { {  hw, -hh,  hl }, { 0.0, 0.0,  1.0 }, { 1.0f, 0.0f } },
        { {  hw,  hh,  hl }, { 0.0, 0.0,  1.0 }, { 1.0f, 1.0f } },
        { { -hw,  hh,  hl }, { 0.0, 0.0,  1.0 }, { 0.0f, 1.0f } },
  
        { { -hw, -hh, -hl }, { -1.0, 0.0, 0.0 }, { 0.0f, 0.0f } },
        { { -hw,  hh, -hl }, { -1.0, 0.0, 0.0 }, { 1.0f, 0.0f } },
        { { -hw,  hh,  hl }, { -1.0, 0.0, 0.0 }, { 1.0f, 1.0f } },
        { { -hw, -hh,  hl }, { -1.0, 0.0, 0.0 }, { 0.0f, 1.0f } },
  
        { {  hw, -hh, -hl, }, { 1.0, 0.0, 0.0 }, { 0.0f, 0.0f } },
        { {  hw,  hh, -hl, }, { 1.0, 0.0, 0.0 }, { 1.0f, 0.0f } },
        { {  hw,  hh,  hl, }, { 1.0, 0.0, 0.0 }, { 1.0f, 1.0f } },
        { {  hw, -hh,  hl, }, { 1.0, 0.0, 0.0 }, { 0.0f, 1.0f } },
  
        { { -hw, -hh, -hl }, { 0.0, -1.0, 0.0 }, { 0.0f, 0.0f } },
        { { -hw, -hh,  hl }, { 0.0, -1.0, 0.0 }, { 1.0f, 0.0f } },
        { {  hw, -hh,  hl }, { 0.0, -1.0, 0.0 }, { 1.0f, 1.0f } },
        { {  hw, -hh, -hl }, { 0.0, -1.0, 0.0 }, { 0.0f, 1.0f } },
  
        { { -hw,  hh, -hl }, { 0.0,  1.0, 0.0 }, { 0.0f, 0.0f } },
        { { -hw,  hh,  hl }, { 0.0,  1.0, 0.0 }, { 1.0f, 0.0f } },
        { {  hw,  hh,  hl }, { 0.0,  1.0, 0.0 }, { 1.0f, 1.0f } },
        { {  hw,  hh, -hl }, { 0.0,  1.0, 0.0 }, { 0.0f, 1.0f } }
    };
    
    uint16_t indices[] = {
        0, 1, 2,  0, 2, 3,
        6, 5, 4,  7, 6, 4,
        8, 9, 10,  8, 10, 11,
        14, 13, 12,  15, 14, 12,
        16, 17, 18,  16, 18, 19,
        22, 21, 20,  23, 22, 20
    };

    return geometry_pass_make_mesh(pass, &(mesh_desc_t) {
        .vertices = vertices,
        .num_vertices = 24,
        .indices = indices,
        .num_indices = 36,
        .label = box->label
    });
}

void geometry_pass_destroy_mesh(geometry_pass_t* pass, mesh_id_t mesh) {
    assert(pass);

    // mark the mesh slot free
    if (handle_is_valid(mesh, GEOMETRY_PASS_MAX_MESHES)) {
        pass->meshes[mesh.id] = empty_mesh;
    }

    // look up for the model, if exists, which
    // points to this mesh and invalidate it.
    for (int32_t i = 0; i < GEOMETRY_PASS_MAX_MODELS; ++i) {
        model_t* model = &pass->models[i];
        if (model->mesh_id.id == mesh.id) {
            geometry_pass_destroy_model(pass, (model_id_t){.id=i});
        }
    }
}

material_id_t geometry_pass_make_material(geometry_pass_t* pass,
    const material_desc_t* material_desc) {
    assert(pass && material_desc);
    assert(material_desc->albedo);
    assert(material_desc->albedo->width > 0);
    assert(material_desc->albedo->height > 0);
    assert(material_desc->albedo->layers > 0);
    assert(material_desc->emissive);
    assert(material_desc->emissive->width > 0);
    assert(material_desc->emissive->height > 0);
    assert(material_desc->emissive->layers > 0);
    
    material_id_t mat_id = {
        .id = HANDLE_INVALID_ID
    };

    // search for an empty slot
    for (int32_t i = 0; i < GEOMETRY_PASS_MAX_MATERIALS; ++i) {
        // if we find an empty slot, store the index and break the loop
        if (material_is_empty(&pass->materials[i])) {
            mat_id.id = i;
            break;
        }
    }

    // no free slot found
    if (mat_id.id == HANDLE_INVALID_ID) {
        return mat_id;
    }

    // create temporary trace for image label
    trace_t im_albedo_trace;
    trace_printf(&im_albedo_trace,
        "%s-%s", material_desc->label, "image-at");

    trace_t im_emissive_trace;
    trace_printf(&im_emissive_trace,
        "%s-%s", material_desc->label, "image-es");

    // create graphics image resource
    material_t mat = {
        .albedo_transparency = sg_make_image(&(sg_image_desc){
            .type = SG_IMAGETYPE_ARRAY,
            .width = material_desc->albedo->width,
            .height = material_desc->albedo->height,
            .layers = material_desc->albedo->layers,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .content.subimage[0][0] = {
                .ptr = material_desc->albedo->pixels,
                .size = sizeof(uint32_t) *
                    material_desc->albedo->width *
                    material_desc->albedo->height *
                    material_desc->albedo->layers
            },
            .label = im_albedo_trace.name
        }),
        .emissive_specular = sg_make_image(&(sg_image_desc){
            .type = SG_IMAGETYPE_ARRAY,
            .width = material_desc->emissive->width,
            .height = material_desc->emissive->height,
            .layers = material_desc->emissive->layers,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .content.subimage[0][0] = {
                .ptr = material_desc->emissive->pixels,
                .size = sizeof(uint32_t) *
                    material_desc->emissive->width *
                    material_desc->emissive->height *
                    material_desc->emissive->layers
            },
            .label = im_emissive_trace.name
        })
    };

    // store the label into the material's trace name
    trace_printf(&mat.trace, "%s", material_desc->label);

    // assign the material to the free slot
    pass->materials[mat_id.id] = mat;
    return mat_id;
}

material_id_t geometry_pass_make_material_default(geometry_pass_t* pass) {
    assert(pass);

    static uint32_t color_alpha[4*4*4] = {
        // checkboard
        0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000,
        0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
        0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000,
        0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
        // horizontal lines
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        // vertical lines
        0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
        0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
        0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
        0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
        // v-zag
        0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
        0x00000000, 0x00000000, 0xFFFFFFFF, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
    };

    static uint32_t emissive_specular[4*4*1] = {
        0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
        0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
        0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
        0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000,
    };

    return geometry_pass_make_material(pass, &(material_desc_t) {
        .albedo = &(image_desc_t) {
            .width = 4,
            .height = 4,
            .layers = 4,
            .pixels = color_alpha
        },
        .emissive = &(image_desc_t) {
            .width = 4,
            .height = 4,
            .layers = 1,
            .pixels = emissive_specular
        },
        .label = "default-material"
    });
}

void geometry_pass_destroy_material(geometry_pass_t* pass, material_id_t mat) {
    assert(pass);
    
    // mark the material slot free
    if (handle_is_valid(mat, GEOMETRY_PASS_MAX_MATERIALS)) {
        pass->materials[mat.id] = empty_material;
    }

    // find the relative model to invalidate
    for (int32_t i = 0; i < GEOMETRY_PASS_MAX_MODELS; ++i) {
        model_t* model = &pass->models[i];
        if (model->material_id.id == mat.id) {
            geometry_pass_destroy_model(pass, (model_id_t){.id=i});
        }
    }
}

model_id_t geometry_pass_create_model(geometry_pass_t* pass,
    const model_desc_t* model_desc) {
    assert(pass && model_desc);
    assert(model_desc->mesh.id != HANDLE_INVALID_ID);
    assert(model_desc->material.id != HANDLE_INVALID_ID);

    model_id_t model_id = {
        .id = HANDLE_INVALID_ID
    };

    // search for an empty slot
    for (int32_t i = 0; i < GEOMETRY_PASS_MAX_MODELS; ++i) {
        // if we find an empty slot, store the index and break the loop
        if (model_is_empty(&pass->models[i])) {
            model_id.id = i;
            break;
        }
    }

    // no free slot found
    if (model_id.id == HANDLE_INVALID_ID) {
        return model_id;
    }

    model_t model = {
        .mesh_id = model_desc->mesh,
        .material_id = model_desc->material
    };

    // store the label into the model's trace name
    trace_printf(&model.trace, "%s", model_desc->label);

    // store model into the array
    pass->models[model_id.id] = model;

    // create a drawcall
    draw_call_t* draw = &pass->render.draws[model_id.id];
    
    // zero initialise draw params
    draw->indices_offset = 0;
    draw->num_instances = 0;

    const mesh_t* mesh = &pass->meshes[model.mesh_id.id];

    draw->num_indices = mesh->num_elements;

    trace_t id_trace;
    trace_printf(&id_trace, "%s-%s", model_desc->label, "instance-buffer");
    
    // update draw bindings
    draw->bindings.index_buffer = mesh->ibuf;
    draw->bindings.vertex_buffers[BUFFER_INDEX_VERTEX] = mesh->vbuf;
    draw->bindings.vertex_buffers[BUFFER_INDEX_INSTANCE] =
        sg_make_buffer(&(sg_buffer_desc) {
            .size = GEOMETRY_PASS_MAX_INSTANCES * sizeof(instance_t),
            .usage = SG_USAGE_STREAM,
            .label = id_trace.name
        });

    const material_t* mat = &pass->materials[model.material_id.id];
    draw->bindings.fs_images[SLOT_albedo_transparency] = mat->albedo_transparency;
    draw->bindings.fs_images[SLOT_emissive_specular] = mat->emissive_specular;

    return model_id;
}

void geometry_pass_destroy_model(geometry_pass_t* pass, model_id_t model) {
    assert(pass);
    
    // mark the model slot free
    if (handle_is_valid(model, GEOMETRY_PASS_MAX_MODELS)) {
        model_t* model_ptr = &pass->models[model.id];
        if (!model_is_empty(model_ptr)) {
            *model_ptr = empty_model;

            // destroy the drawcall and set the slot free
            draw_call_t* draw = &pass->render.draws[model.id];

            // the only buffer initialised by the model logic 
            // is the instance buffer, therfore, is the only 
            // we need to destroy here manually
            sg_destroy_buffer(
                draw->bindings.vertex_buffers[BUFFER_INDEX_INSTANCE]);
            
            // reset draw call settings
            draw_call_reset(draw);
        }
    }
}

void geometry_pass_update_model_instances(geometry_pass_t* pass,
    model_id_t model, const instance_t* instances, uint32_t count) {
    assert(pass && instances && count > 0);

    if (handle_is_valid(model, GEOMETRY_PASS_MAX_MODELS)) {
        model_t* model_ptr = &pass->models[model.id];

        // check whether the model is valid,
        // skip and emit a warning otherwise
        if (!handle_is_valid(model_ptr->mesh_id, GEOMETRY_PASS_MAX_MESHES) ||
            !handle_is_valid(model_ptr->material_id, GEOMETRY_PASS_MAX_MATERIALS)) {
            LOG_WARN("WARN: Group (%s:%d) is not valid\n",
                model_ptr->trace.name, model.id);
            return;
        }

        // if too many instances in the array,
        // then trim them off and issue a warn
        if (count > GEOMETRY_PASS_MAX_INSTANCES) {
            LOG_WARN("WARN: Too many instances (%d) for model (%s:%d);"
                "only %d will be updated\n", count, model_ptr->trace.name,
                model.id, GEOMETRY_PASS_MAX_INSTANCES);
            count = GEOMETRY_PASS_MAX_INSTANCES;
        }

        // upload instance data to render device
        draw_call_t* draw_call = &pass->render.draws[model.id];
        const sg_bindings* bindings = &draw_call->bindings;
        sg_update_buffer(bindings->vertex_buffers[BUFFER_INDEX_INSTANCE],
            instances, count * sizeof(instance_t));

        // update draw calls instances count
        draw_call->num_instances = count;
    }
}

static void renderer_pass_setup(const geometry_pass_t* geometry_pass,
    render_pass_t* render_pass) {
    
    // init uniforms
    render_pass->uniforms.vs_ubo.index = SLOT_vs_params;
    render_pass->uniforms.vs_ubo.data = (uint8_t*)&geometry_pass->globals;
    render_pass->uniforms.vs_ubo.size = sizeof(vs_params_t);

    render_pass->uniforms.fs_ubo.index = SLOT_fs_params;
    render_pass->uniforms.fs_ubo.data = (uint8_t*)&geometry_pass->globals + 
        offsetof(globals_t, ambient);
    render_pass->uniforms.fs_ubo.size = sizeof(fs_params_t);

    // init shaders
    render_pass->shader = sg_make_shader(geometry_pass_shader_desc());

    // init pipeline
    render_pass->pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
        .shader = render_pass->shader,
        .layout = {
            .buffers[BUFFER_INDEX_VERTEX].step_func = SG_VERTEXSTEP_PER_VERTEX,
            .buffers[BUFFER_INDEX_INSTANCE].step_func = SG_VERTEXSTEP_PER_INSTANCE,
            .attrs = {
                [ATTR_geo_vs_vertex_pos] = {.offset = offsetof(vertex_t, pos),.format = SG_VERTEXFORMAT_FLOAT3,.buffer_index = BUFFER_INDEX_VERTEX},
                [ATTR_geo_vs_vertex_norm] = {.offset = offsetof(vertex_t, norm),.format = SG_VERTEXFORMAT_FLOAT3,.buffer_index = BUFFER_INDEX_VERTEX},
                [ATTR_geo_vs_vertex_uv] = {.offset = offsetof(vertex_t, uv),.format = SG_VERTEXFORMAT_FLOAT2,.buffer_index = BUFFER_INDEX_VERTEX},
                [ATTR_geo_vs_instance_color] = {.offset = offsetof(instance_t, color),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
                [ATTR_geo_vs_instance_tile] = {.offset = offsetof(instance_t, uv_scale_pan),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
                // 4x4 matrices will span 4 attribute slots
                [ATTR_geo_vs_instance_pose] = {.offset = offsetof(instance_t, pose),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
                [ATTR_geo_vs_instance_pose+1] = {.offset = offsetof(instance_t, pose) + (sizeof(mfloat_t) * 4),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
                [ATTR_geo_vs_instance_pose+2] = {.offset = offsetof(instance_t, pose) + (sizeof(mfloat_t) * 8),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
                [ATTR_geo_vs_instance_pose+3] = {.offset = offsetof(instance_t, pose) + (sizeof(mfloat_t) * 12),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
                // 4x4 matrices will span 4 attribute slots
                [ATTR_geo_vs_instance_normal] = {.offset = offsetof(instance_t, normal),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
                [ATTR_geo_vs_instance_normal+1] = {.offset = offsetof(instance_t, normal) + (sizeof(mfloat_t) * 4),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
                [ATTR_geo_vs_instance_normal+2] = {.offset = offsetof(instance_t, normal) + (sizeof(mfloat_t) * 8),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
                [ATTR_geo_vs_instance_normal+3] = {.offset = offsetof(instance_t, normal) + (sizeof(mfloat_t) * 12),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE}
            }
        },
        .index_type = SG_INDEXTYPE_UINT16,
        .depth_stencil = {
            .depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL,
            .depth_write_enabled = true,
        },
        .blend = {
            .enabled = true,
            .depth_format = SG_PIXELFORMAT_DEPTHSTENCIL,
            .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
            .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
            .src_factor_alpha = SG_BLENDFACTOR_ONE,
            .dst_factor_alpha = SG_BLENDFACTOR_ZERO,
        },
        .rasterizer = {
            .cull_mode = SG_CULLMODE_BACK,
            .sample_count = RASTERIZER_MSAA_SAMPLES
        },
        .label = "geometry-pass-pipeline"
    });

    trace_printf(&render_pass->trace, "geometry-pass");
}

void geometry_pass_init(geometry_pass_t* pass) {
    assert(pass);

    // mark all mesh slots empty
    for (int32_t i = 0; i < GEOMETRY_PASS_MAX_MESHES; ++i) {
        memcpy(&pass->meshes[i], &empty_mesh, sizeof(mesh_t));
    }

    // mark all material slots empty
    for (int32_t i = 0; i < GEOMETRY_PASS_MAX_MATERIALS; ++i) {
        memcpy(&pass->materials[i], &empty_material, sizeof(material_t));
    }

    // mark all instance slots empty
    for (int32_t i = 0; i < GEOMETRY_PASS_MAX_MODELS; ++i) {
        memcpy(&pass->models[i], &empty_model, sizeof(model_t));
    }

    // setup the render pass
    renderer_pass_setup(pass, &pass->render);
}

void geometry_pass_cleanup(geometry_pass_t* pass) {
    assert(pass);

    // iterate through all mesh slots, and, if full, destory it
    for (int32_t i = 0; i < GEOMETRY_PASS_MAX_MESHES; ++i) {
        if (!mesh_is_empty(&pass->meshes[i])) {
            geometry_pass_destroy_mesh(pass, (mesh_id_t){.id=i});
        }
    }
    
    // iterate through all material slots, and, if full, destory it
    for (int32_t i = 0; i < GEOMETRY_PASS_MAX_MATERIALS; ++i) {
        if (!material_is_empty(&pass->materials[i])) {
            geometry_pass_destroy_material(pass, (material_id_t){.id=i});
        }
    }

    // models will be destroyed automatically when either
    // the corresponding mesh, or material, get destoried.

    // release render resources
    sg_destroy_shader(pass->render.shader);
    sg_destroy_pipeline(pass->render.pipeline);

    memset(&pass->render, 0, sizeof(render_pass_t));
}

#if defined(__cplusplus)
} // extern "C"
#endif
