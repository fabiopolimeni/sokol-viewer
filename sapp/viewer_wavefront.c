#include "viewer_wavefront.h"

#include <assert.h>
#include <string.h>

#if defined(__cplusplus)
extern "C" {
#endif

wavefront_obj_t wavefront_load_obj(const wavefront_data_t* data) {
    return (wavefront_obj_t){0};
}

model_id_t wavefront_make_model(geometry_pass_t* pass,
    const wavefront_model_t* model) {
    return (model_id_t){.id=HANDLE_INVALID_ID};
}

void wavefront_release_obj(wavefront_obj_t* obj) {

}

#if defined(__cplusplus)
}
#endif
