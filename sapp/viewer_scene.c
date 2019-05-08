#include "viewer_scene.h"

#include <assert.h>
#include <string.h> // memcmp

group_id_t scene_create_group(scene_t* scene, mesh_t mesh, material_t mat) {
    assert(scene);
    static const group_t empty_group = {0};

    // Iterate through the groups, if any of the group's 
    // memory is 0, then, this is an available group slot.
    for (int32_t i = 0; i < SCENE_MAX_GROUPS; ++i) {
        group_t *grp = &scene->groups[i];
        if (memcmp(grp, &empty_group, sizeof(group_t)) == 0) {
            grp->mesh = mesh;
            grp->mat = mat;
            return (group_id_t) {
                .id = i
            };
        }
    }
    
    // No available slot
    return (group_id_t) {
        .id = HANDLE_INVALID_ID
    };
}

void scene_destroy_group(scene_t* scene, group_id_t group) {
    assert(scene);
    if (group.id >= 0 && group.id < SCENE_MAX_GROUPS) {
        memset(&scene->groups[group.id], 0, sizeof(group_t));
    }
}

instance_id_t scene_add_instance(scene_t* scene,
    group_id_t group, mat4f_t pose, vec4f_t color) {
    assert(scene);
    static const instance_t empty_instance = {0};

    if (group.id >= 0 && group.id < SCENE_MAX_GROUPS) {
        group_t *grp = &scene->groups[group.id];

        // Iterate through all group instances, and if
        // one is empty, return it as the available one.
        for (int32_t i = 0; i < RENDERER_MAX_INSTANCES; ++i) {
            instance_t *inst = &grp->instances[i];
            if (memcmp(inst, &empty_instance, sizeof(instance_t)) == 0) {
                inst->color = color;
                inst->pose = pose;
                return (instance_id_t) {
                    .id = i
                };
            }
        }
    }
    
    // No available slot
    return (instance_id_t) {
        .id = HANDLE_INVALID_ID
    };
}

void scene_remove_instance(scene_t* scene,
    group_id_t group, instance_id_t instance) {
    assert(scene);

    if (group.id >= 0 && group.id < SCENE_MAX_GROUPS
        && instance.id >= 0 && instance.id > RENDERER_MAX_INSTANCES) {
        instance_t *inst = &scene->groups[group.id].instances[instance.id];
        memset(inst, 0, sizeof(instance_t));
    }
}
