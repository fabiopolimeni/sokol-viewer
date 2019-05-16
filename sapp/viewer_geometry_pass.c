#include "viewer_geometry_pass.h"
#include "viewer_log.h"

#include <stddef.h>
#include <assert.h>
#include <string.h> // memcmp

#define BUFFER_INDEX_VERTEX 0
#define BUFFER_INDEX_INSTANCE 1
#define UNIFORM_BLOCK_INDEX 0
#define VS_UNIFORM_INDEX_VIEW_PROJ 0
#define VS_UNIFORM_INDEX_LIGHT 1
#define VS_UNIFORM_INDEX_EYE_POS 2
#define FS_UNIFORM_INDEX_AMBIENT_SPEC 0
#define SAMPLER_INDEX_ALBEDO 0
#define ATTR_INDEX_VERTEX_POS 0
#define ATTR_INDEX_VERTEX_NORM 1
#define ATTR_INDEX_VERTEX_UV 2
#define ATTR_INDEX_INSTANCE_COLOR 3
#define ATTR_INDEX_INSTANCE_POSE 4     // mat4 takes 4 slots
#define ATTR_INDEX_INSTANCE_NORMAL 8   // mat4 takes 4 slots
#define RASTERIZER_MSAA_SAMPLES 1

#if defined(__cplusplus)
extern "C" {
#endif

extern const char* geometry_vs_src;
extern const char* geometry_fs_src;

typedef struct {
    mat4f_t view_proj;
    vec4f_t light;
    vec3f_t eye_pos;
} vs_uniforms_t;

typedef struct {
    vec4f_t ambient_spec;
} fs_uniforms_t;

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
    .ambient_spec = {0},
    .albedo_rough = -1,
    .trace = {0}
};

static bool material_is_empty(const material_t* material) {
    return memcmp(material, &empty_material, sizeof(material_t)) == 0;
}

const static group_t empty_group = {
    .mesh_id = {.id = HANDLE_INVALID_ID},
    .material_id = {.id = HANDLE_INVALID_ID}
};

static bool group_is_empty(const group_t* group) {
    return memcmp(group, &empty_group, sizeof(group_t)) == 0;
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
    trace_printf(&vb_trace, "%s-%s", mesh_desc->label, "vb");

    trace_t ib_trace;
    trace_printf(&ib_trace, "%s-%s", mesh_desc->label, "ib");

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

    // look up for the group, if exists, which
    // points to this mesh and invalidate it.
    for (int32_t i = 0; i < GEOMETRY_PASS_MAX_GROUPS; ++i) {
        group_t* group = &pass->groups[i];
        if (group->mesh_id.id == mesh.id) {
            geometry_pass_destroy_group(pass, (group_id_t){.id=i});
        }
    }
}

material_id_t geometry_pass_make_material(geometry_pass_t* pass,
    const material_desc_t* material_desc) {
    assert(pass && material_desc);
    assert(material_desc->width > 0);
    assert(material_desc->height > 0);
    
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
    trace_t im_trace;
    trace_printf(&im_trace, "%s-%s", material_desc->label, "image");

    // create the graphics material resources
    material_t mat = {
        .ambient_spec = material_desc->ambient_spec,
        .albedo_rough = sg_make_image(&(sg_image_desc){
            .width = material_desc->width,
            .height = material_desc->height,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .content.subimage[0][0] = {
                .ptr = material_desc->pixels,
                .size = sizeof(uint32_t) *
                    material_desc->width * material_desc->height
            },
            .label = im_trace.name
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

    static uint32_t checkerboard_pixels[4*4] = {
        0x99FFFFFF, 0xFF000000, 0x99FFFFFF, 0xFF000000,
        0xFF000000, 0x99FFFFFF, 0xFF000000, 0x99FFFFFF,
        0x99FFFFFF, 0xFF000000, 0x99FFFFFF, 0xFF000000,
        0xFF000000, 0x99FFFFFF, 0xFF000000, 0x99FFFFFF,
    };

    return geometry_pass_make_material(pass, &(material_desc_t) {
        .width = 4,
        .height = 4,
        .pixels = checkerboard_pixels,
        .ambient_spec = (vec4f_t){1.f,1.f,1.f,0.f},
        .label = "checkboard-material"
    });
}

void geometry_pass_destroy_material(geometry_pass_t* pass, material_id_t mat) {
    assert(pass);
    
    // mark the material slot free
    if (handle_is_valid(mat, GEOMETRY_PASS_MAX_MATERIALS)) {
        pass->materials[mat.id] = empty_material;
    }

    // find the relative group to invalidate
    for (int32_t i = 0; i < GEOMETRY_PASS_MAX_GROUPS; ++i) {
        group_t* group = &pass->groups[i];
        if (group->material_id.id == mat.id) {
            geometry_pass_destroy_group(pass, (group_id_t){.id=i});
        }
    }
}

group_id_t geometry_pass_create_group(geometry_pass_t* pass,
    const group_desc_t* group_desc) {
    assert(pass && group_desc);
    assert(group_desc->mesh.id != HANDLE_INVALID_ID);
    assert(group_desc->material.id != HANDLE_INVALID_ID);

    group_id_t group_id = {
        .id = HANDLE_INVALID_ID
    };

    // search for an empty slot
    for (int32_t i = 0; i < GEOMETRY_PASS_MAX_GROUPS; ++i) {
        // if we find an empty slot, store the index and break the loop
        if (group_is_empty(&pass->groups[i])) {
            group_id.id = i;
            break;
        }
    }

    // no free slot found
    if (group_id.id == HANDLE_INVALID_ID) {
        return group_id;
    }

    group_t group = {
        .mesh_id = group_desc->mesh,
        .material_id = group_desc->material
    };

    // store the label into the group's trace name
    trace_printf(&group.trace, "%s", group_desc->label);

    // create a drawcall
    draw_call_t* draw = &pass->render.draws[group_id.id];
    
    // zero initialise draw params
    draw->indices_offset = 0;
    draw->num_instances = 0;

    const mesh_t* mesh = &pass->meshes[group.mesh_id.id];

    draw->num_indices = mesh->num_elements;
    
    // update draw bindings
    draw->bindings.index_buffer = mesh->ibuf;
    draw->bindings.vertex_buffers[BUFFER_INDEX_VERTEX] = mesh->vbuf;
    draw->bindings.vertex_buffers[BUFFER_INDEX_INSTANCE] =
        sg_make_buffer(&(sg_buffer_desc) {
            .size = GEOMETRY_PASS_MAX_INSTANCES * sizeof(instance_t),
            .usage = SG_USAGE_STREAM,
            .label = "geometry-pass-instance-data"
        });

    const material_t* mat = &pass->materials[group.material_id.id];
    draw->bindings.fs_images[SAMPLER_INDEX_ALBEDO] = mat->albedo_rough;

    return group_id;
}

void geometry_pass_destroy_group(geometry_pass_t* pass, group_id_t group) {
    assert(pass);
    
    // mark the group slot free
    if (handle_is_valid(group, GEOMETRY_PASS_MAX_GROUPS)) {
        group_t* grp = &pass->groups[group.id];
        if (!group_is_empty(grp)) {
            *grp = empty_group;

            // destroy the drawcall and set the slot free
            draw_call_t* draw = &pass->render.draws[group.id];

            // the only buffer initialised by the group logic 
            // is the instance buffer, therfore, is the only 
            // we need to destroy here manually.
            sg_destroy_buffer(
                draw->bindings.vertex_buffers[BUFFER_INDEX_INSTANCE]);
            
            // reset draw call settings
            draw_call_reset(draw);
        }
    }
}

void geometry_pass_update_group(geometry_pass_t* pass,
    group_id_t group, const instance_t* instances, uint32_t count) {
    assert(pass && instances && count > 0);

    if (handle_is_valid(group, GEOMETRY_PASS_MAX_GROUPS)) {
        group_t* grp = &pass->groups[group.id];

        // check whether the group is valid,
        // skip and emit a warning otherwise.
        if (!handle_is_valid(grp->mesh_id, GEOMETRY_PASS_MAX_MESHES) ||
            handle_is_valid(grp->material_id, GEOMETRY_PASS_MAX_MATERIALS)) {
            LOG_WARN("WARN: Group (%s:%d) is not valid\n",
                grp->trace.name, group.id);
            return;
        }

        // if too many instances in the array, trim them off and warn
        if (count > GEOMETRY_PASS_MAX_INSTANCES) {
            LOG_WARN("WARN: Too many instances (%d) for group (%s:%d);"
                "only %d will be updated\n", count, grp->trace.name,
                group.id, GEOMETRY_PASS_MAX_INSTANCES);
            count = GEOMETRY_PASS_MAX_INSTANCES;
        }

        // upload instance data to render device
        draw_call_t* draw_call = &pass->render.draws[group.id];
        const sg_bindings* bindings = &draw_call->bindings;
        sg_update_buffer(bindings->vertex_buffers[BUFFER_INDEX_INSTANCE],
            instances, count * sizeof(instance_t));

        // update draw calls instances count
        draw_call->num_instances = count;
    }
}

static void renderer_pass_setup(const geometry_pass_t* geometry_pass,
    render_pass_t* render_pass) {
    
    // init pass action
    render_pass->pass_action = (sg_pass_action) {
        .colors[0] = { 
            .action=SG_ACTION_CLEAR,
            .val={0.6f, 0.8f, 0.0f, 1.0f}
        }
    };

    // init uniforms
    render_pass->uniforms.vs_ubo.index = UNIFORM_BLOCK_INDEX;
    render_pass->uniforms.vs_ubo.data = (uint8_t*)&geometry_pass->globals;
    render_pass->uniforms.vs_ubo.size =
        sizeof(mat4f_t) + sizeof(vec4f_t) + sizeof(vec3f_t);

    render_pass->uniforms.fs_ubo.index = UNIFORM_BLOCK_INDEX;
    render_pass->uniforms.fs_ubo.data = (uint8_t*)&geometry_pass->globals + 
        offsetof(globals_t, ambient_spec);
    render_pass->uniforms.fs_ubo.size = sizeof(vec4f_t);

    // init shaders
    render_pass->shader = sg_make_shader(&(sg_shader_desc) {
        .attrs = {
            [ATTR_INDEX_VERTEX_POS] = { .name="vertex_pos", .sem_name="POSITION" },
            [ATTR_INDEX_VERTEX_NORM] = { .name="vertex_norm", .sem_name="NORMAL" },
            [ATTR_INDEX_VERTEX_UV] = { .name="vertex_uv", .sem_name="UV" },
            [ATTR_INDEX_INSTANCE_COLOR] = { .name="instance_color", .sem_name="COLOR" },
            [ATTR_INDEX_INSTANCE_POSE] = { .name="instance_pose", .sem_name="POSE" },
            [ATTR_INDEX_INSTANCE_NORMAL] = { .name="instance_normal", .sem_name="INVTRANS_POSE" }
        },
        .vs = {
            .uniform_blocks[UNIFORM_BLOCK_INDEX] = {
                .size = render_pass->uniforms.vs_ubo.size,
                .uniforms = {
                    [VS_UNIFORM_INDEX_VIEW_PROJ] = { .name="view_proj", .type=SG_UNIFORMTYPE_MAT4 },
                    [VS_UNIFORM_INDEX_LIGHT] = { .name="light", .type=SG_UNIFORMTYPE_FLOAT4 },
                    [VS_UNIFORM_INDEX_EYE_POS] = { .name="eye_pos", .type=SG_UNIFORMTYPE_FLOAT3 }
                }
            },
            .source = geometry_vs_src
        },
        .fs = {
            .uniform_blocks[UNIFORM_BLOCK_INDEX] = {
                .size = render_pass->uniforms.fs_ubo.size,
                .uniforms = {
                    [FS_UNIFORM_INDEX_AMBIENT_SPEC] = { .name="ambient_spec", .type=SG_UNIFORMTYPE_FLOAT4 },
                }
            },
            .images[SAMPLER_INDEX_ALBEDO] = {
                .name = "albedo_rough",
                .type = SG_IMAGETYPE_2D
            },
            .source = geometry_fs_src
        },
        .label = "geometry-pass-shader"
    });

    // init pipeline
    render_pass->pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
        .shader = render_pass->shader,
        .layout = {
            .buffers[BUFFER_INDEX_VERTEX].step_func = SG_VERTEXSTEP_PER_VERTEX,
            .buffers[BUFFER_INDEX_INSTANCE].step_func = SG_VERTEXSTEP_PER_INSTANCE,
            .attrs = {
                [ATTR_INDEX_VERTEX_POS] = {.offset = offsetof(vertex_t, pos),.format = SG_VERTEXFORMAT_FLOAT3,.buffer_index = BUFFER_INDEX_VERTEX},
                [ATTR_INDEX_VERTEX_NORM] = {.offset = offsetof(vertex_t, norm),.format = SG_VERTEXFORMAT_FLOAT3,.buffer_index = BUFFER_INDEX_VERTEX},
                [ATTR_INDEX_VERTEX_UV] = {.offset = offsetof(vertex_t, uv),.format = SG_VERTEXFORMAT_FLOAT2,.buffer_index = BUFFER_INDEX_VERTEX},
                [ATTR_INDEX_INSTANCE_COLOR] = {.offset = offsetof(instance_t, color),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
                // 4x4 matrices will span 4 attribute slots
                [ATTR_INDEX_INSTANCE_POSE] = {.offset = offsetof(instance_t, pose),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
                [ATTR_INDEX_INSTANCE_POSE+1] = {.offset = offsetof(instance_t, pose) + (sizeof(mfloat_t) * 4),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
                [ATTR_INDEX_INSTANCE_POSE+2] = {.offset = offsetof(instance_t, pose) + (sizeof(mfloat_t) * 8),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
                [ATTR_INDEX_INSTANCE_POSE+3] = {.offset = offsetof(instance_t, pose) + (sizeof(mfloat_t) * 12),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
                // 4x4 matrices will span 4 attribute slots
                [ATTR_INDEX_INSTANCE_NORMAL] = {.offset = offsetof(instance_t, normal),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
                [ATTR_INDEX_INSTANCE_NORMAL+1] = {.offset = offsetof(instance_t, normal) + (sizeof(mfloat_t) * 4),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
                [ATTR_INDEX_INSTANCE_NORMAL+2] = {.offset = offsetof(instance_t, normal) + (sizeof(mfloat_t) * 8),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE},
                [ATTR_INDEX_INSTANCE_NORMAL+3] = {.offset = offsetof(instance_t, normal) + (sizeof(mfloat_t) * 12),.format = SG_VERTEXFORMAT_FLOAT4,.buffer_index = BUFFER_INDEX_INSTANCE}
            }
        },
        .index_type = SG_INDEXTYPE_UINT16,
        .depth_stencil = {
            .depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL,
            .depth_write_enabled = true,
        },
        .blend = {
            .depth_format = SG_PIXELFORMAT_DEPTHSTENCIL,
        },
        .rasterizer = {
            .cull_mode = SG_CULLMODE_BACK,
            .sample_count = RASTERIZER_MSAA_SAMPLES
        },
        .label = "geometry-pass-pipeline"
    });

    trace_printf(&render_pass->trace, "geometry-pass");
}

void render_pass_destroy(render_pass_t* render_pass) {
    assert(render_pass);

    sg_destroy_shader(render_pass->shader);
    sg_destroy_pipeline(render_pass->pipeline);

    memset(render_pass, 0, sizeof(render_pass_t));
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
    for (int32_t i = 0; i < GEOMETRY_PASS_MAX_GROUPS; ++i) {
        memcpy(&pass->groups[i], &empty_group, sizeof(group_t));
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

    // groups will be destroyed automatically when, either
    // the corresponding mesh, or material get destoried.

    // release render resources
    render_pass_destroy(&pass->render);
}

#if defined(__cplusplus)
} // extern "C"
#endif
