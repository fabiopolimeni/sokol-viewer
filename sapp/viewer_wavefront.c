#include "viewer_wavefront.h"
#include "viewer_file.h"
#include "viewer_memory.h"

#include <assert.h>
#include <string.h>

#if defined(__cplusplus)
extern "C" {
#endif

wavefront_result_t wavefront_parse_obj(const wavefront_data_t* data,
    wavefront_model_t* out) {
    return WAVEFRONT_RESULT_OK;
}

void wavefront_release_obj(wavefront_model_t* model) {

}

model_id_t wavefront_make_model(geometry_pass_t* pass,
    const wavefront_model_t* model) {
    return (model_id_t){.id=HANDLE_INVALID_ID};
}

#if defined(__cplusplus)
}
#endif
