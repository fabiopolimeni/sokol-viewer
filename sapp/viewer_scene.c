#include "viewer_scene.h"

#include <assert.h>
#include <string.h> // memcmp
#include <stdlib.h> // qsort

#if defined(__cplusplus)
extern "C" {
#endif

static const transform_t transform_default = {
    .position = {0.f,0.f,0.f},
    .scale = {1.f,1.f,1.f},
    .rotation = {0.f,0.f,0.f,1.f},
};

static const node_t node_empty = {
    .transform = {
        .position = {0.f,0.f,0.f},
        .scale = {1.f,1.f,1.f},
        .rotation = {0.f,0.f,0.f,1.f},
    },
    .model_id = {HANDLE_INVALID_ID},
    .parent_id = {HANDLE_INVALID_ID}
};

static bool node_is_empty(const node_t* node) {
    assert(node);
    return memcmp(node, &node_empty, sizeof(node_t)) == 0;
}

void scene_init(scene_t* scene) {
    assert(scene);

    scene->camera = (camera_t){
        .target = (vec3f_t){0.f,0.f,0.f},
        .eye_pos = (vec3f_t){0.f,0.f,10.f},
        .fov = 45.f,
        .near_plane = 0.0f,
        .far_plane = 100.0f
    };

    scene->light = (light_t){
        .plane = (vec4f_t){-1.f,-1.f,-.4f,0.f}
    };

    scene->root = smat4_identity();
    
    for (int32_t i = 0; i < SCENE_MAX_NODES; ++i) {
        scene->nodes[i] = node_empty;
    }
}

void scene_cleanup(scene_t* scene) {
    assert(scene);

    for (int32_t i = 0; i < SCENE_MAX_NODES; ++i) {
        scene->nodes[i] = node_empty;
    }
}

node_id_t scene_add_node(scene_t* scene, const node_desc_t* desc) {
    assert(scene && desc);

    node_id_t node_id = {
        .id = HANDLE_INVALID_ID
    };

    // search for an empty slot
    for (int32_t i = 0; i < SCENE_MAX_NODES; ++i) {

        // if we find an empty slot, store the index and break the loop
        if (node_is_empty(&scene->nodes[i])) {
            node_id.id = i;
            break;
        }
    }

    // no free slot available
    if (node_id.id == HANDLE_INVALID_ID) {
        return node_id;
    }

    // if the parent node is empty, then,
    // detach and add this node to root.
    node_id_t parent = {.id = HANDLE_INVALID_ID};
    if (handle_is_valid(desc->parent, SCENE_MAX_NODES)
        && !node_is_empty(&scene->nodes[desc->parent.id])) {
        parent = desc->parent;
    }

    node_t *node = &scene->nodes[node_id.id];
    *node = (node_t){
        .model_id = desc->model,
        .transform = desc->transform,
        .color = desc->color,
        .tile = desc->tile,
        .parent_id = parent
    };

    trace_printf(&node->trace, desc->label);
    return node_id;
}

void scene_remove_node(scene_t* scene, node_id_t node, bool recursive) {
    assert(scene);

    if (handle_is_valid(node, SCENE_MAX_NODES)) {
        node_t* node_ptr = &scene->nodes[node.id];
        if (!node_is_empty(node_ptr)) {
            node_id_t parent = node_ptr->parent_id;

            // search for all nodes that have this as parent
            for (int32_t i = 0; i < SCENE_MAX_NODES; ++i) {
                if (i != node.id) {
                    node_t* child_ptr = &scene->nodes[i];
                    if (child_ptr->parent_id.id == node.id) {           
                        if (recursive) {
                            // remove all the children if requested
                            scene_remove_node(scene, (node_id_t){.id=i}, true);
                        }
                        else {
                            // reattach the child to the parent of this node
                            child_ptr->parent_id = parent;
                        }
                    }
                }
            }

            // free node's slot
            *node_ptr = node_empty;
        }
    }
}

bool scene_node_is_alive(const scene_t* scene, node_id_t node) {
    assert(scene);

    if (handle_is_valid(node, SCENE_MAX_NODES)) {
        const node_t* node_ptr = &scene->nodes[node.id];
        return !node_is_empty(node_ptr);
    }

    return false;
}

typedef struct {
    mat4f_t pose;
    transform_t transform;
    vec4f_t color;
    vec4f_t tile;
    node_id_t node;
    node_id_t parent;
    model_id_t model;
} node_link_t;

typedef struct {
    instance_t instances[GEOMETRY_PASS_MAX_INSTANCES];
    int32_t instances_count;
} bucket_t;

static int32_t node_link_compare(const void* a, const void* b) {
    const node_link_t* a_link = a;
    const node_link_t* b_link = b;
    return a_link->parent.id - b_link->parent.id;
}

static void update_instances(const scene_t* scene, geometry_pass_t* pass) {
    node_link_t links[SCENE_MAX_NODES] = {0};
    int32_t nodes_count = 0;

    // copy nodes to the link array
    for (int32_t i = 0; i < SCENE_MAX_NODES; ++i) {
        const node_t* node_ptr = &scene->nodes[i];
        if (!node_is_empty(node_ptr)) {
            links[nodes_count] = (node_link_t){
                .transform = node_ptr->transform,
                .color = node_ptr->color,
                .tile = node_ptr->tile,
                .node = (node_id_t){.id=i},
                .model = node_ptr->model_id,
                .parent = node_ptr->parent_id
            };

            ++nodes_count;
        }
    }

    // early exit if no nodes in the scene
    if (nodes_count == 0) {
        return;
    }

    // sort link array by parent id, this way
    // the transformation of a parent will always be
    // up to date when computing ita children ones.
    qsort(links, nodes_count, sizeof(node_link_t), node_link_compare);

    // while traversing the links list bucket
    // bucket for later render procerssing
    bucket_t buckets[GEOMETRY_PASS_MAX_MODELS] = {0};

    // calculate affine transformation for each node
    for (int32_t l = 0; l < nodes_count; ++l) {
        node_link_t* link = &links[l];
    
        mat4f_t local_pose = transform_to_mat4(link->transform);

        // transform local pose into model space
        // by multiplying it by its parent pose
        if (!handle_is_valid(link->parent, SCENE_MAX_NODES)) {
            link->pose = smat4_multiply(scene->root, local_pose);
        }
        else {
            // because node links have been sorted by parent id
            // we need to find the node that corresponds to the
            // parent we are looking for
            for (int32_t n = 0; n < SCENE_MAX_NODES; ++n) {
                const node_link_t* elem = &links[n];
                if (elem->node.id == link->parent.id) {
                    link->pose = smat4_multiply(elem->pose, local_pose);
                    break;
                }
            }
        }

        // set instance data for render model
        assert(handle_is_valid(link->model, GEOMETRY_PASS_MAX_MODELS));
        bucket_t* bucket = &buckets[link->model.id];
        bucket->instances[bucket->instances_count++] = (instance_t){
            .color = link->color,
            .uv_scale_pan = link->tile,
            .pose = link->pose,
            .normal = smat4_transpose(smat4_inverse(link->pose))
        };
    }

    // upload instance data to geometry pass
    for (int32_t b = 0; b < GEOMETRY_PASS_MAX_MODELS; ++b) {
        const bucket_t* bucket = &buckets[b];
        if (bucket->instances_count > 0) {
            geometry_pass_update_model_instances(pass, (model_id_t){.id=b},
                bucket->instances, bucket->instances_count);
        }
    }
}

void scene_update_geometry_pass(const scene_t* scene, geometry_pass_t* pass) {
    assert(scene && pass);

    mat4f_t proj = smat4_identity();
    
    if (scene->camera.fov < 0) {
        proj = smat4_ortho(
            -scene->camera.width, scene->camera.width,
            -scene->camera.height, scene->camera.height,
            scene->camera.near_plane, scene->camera.far_plane);
    }
    else {
        proj = smat4_perspective_fov(scene->camera.fov,
            scene->camera.width, scene->camera.height,
            scene->camera.near_plane, scene->camera.far_plane);
    }

    mat4f_t view = smat4_look_at(
        scene->camera.eye_pos, scene->camera.target,
        (vec3f_t){0.f, 1.f, 0.f});

    pass->globals = (globals_t){
        .view_proj = smat4_multiply(proj, view),
        .ambient = (vec4f_t){
            .x = scene->light.color.x,
            .y = scene->light.color.y,
            .z = scene->light.color.z,
            .w = scene->light.intensity
        },
        .light = scene->light.plane,
        .eye_pos = scene->camera.eye_pos,
    };

    update_instances(scene, pass);
}

#if defined(__cplusplus)
} // extern "C"
#endif
