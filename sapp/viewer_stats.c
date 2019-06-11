#include "viewer_stats.h"
#include "viewer_memory.h"

#include <string.h>
#include <assert.h>

#if defined(__cplusplus)
extern "C" {
#endif

void stats_init(stats_t* stats, uint32_t max_frames) {
    assert(stats && max_frames > 0);

    memset(stats, 0, sizeof(stats_t));    
    stats->update_times = memory_calloc(max_frames, sizeof(float));
    stats->render_times = memory_calloc(max_frames, sizeof(float));
    stats->max_frames = max_frames;
}

void stats_clean(stats_t* stats) {
    assert(stats);
    
    memory_free(stats->update_times);
    memory_free(stats->render_times);

    memset(stats, 0, sizeof(stats_t));
}

void stats_tick(stats_t* stats, float update_time, float render_time) {
    assert(stats);

    uint32_t head_idx = stats->stored_frames % stats->max_frames;

    // just overwrite stale data
    stats->update_times[head_idx] = update_time;
    stats->render_times[head_idx] = render_time;

    stats->total_frames_time += update_time + render_time;

    uint32_t tail_idx = (head_idx + 1) % stats->max_frames;
    float tail_frame_time =
        stats->update_times[tail_idx] +
        stats->render_times[tail_idx];

    stats->total_frames_time -= tail_frame_time;
    
    stats->stored_frames++;
}

uint32_t stats_get_timings(const stats_t* stats,
    float update_arr[], float render_arr[]) {
    assert(stats && update_arr && render_arr);
    
    // There can only be 2 cases here, either the buffer did wrapped
    // up, or it did not. For the first stats->max_frames, it
    // would make sense to check, whether the number of stored frames
    // is less than stats->max_frames, but, once this number is 
    // surpassed, it would make sense no more. Therefore, we don't
    // care about this case in the first instace, simplifying the logic
    // and the calculations, taking the hit of somewhat scrwed average
    // timings for the first stats->max_frames ticks.

    uint32_t tail_idx = (stats->stored_frames) % stats->max_frames;

    uint32_t right_elems = (stats->max_frames - tail_idx);
    uint32_t right_size = right_elems * sizeof(float);
    
    // copy the right part of the ring buffer
    // to the left part of the output arrays.
    memcpy(update_arr, stats->update_times + tail_idx, right_size);
    memcpy(render_arr, stats->render_times + tail_idx, right_size);

    // copy the left part of the ring buffer into
    // the right part of the output arrays, starting
    // from the last tail element.
    uint32_t left_elems = stats->max_frames - right_elems;
    
    // zero checks are not necessary as memcpy deals with it
    uint32_t left_size = left_elems * sizeof(float);

    memcpy(update_arr + tail_idx, stats->update_times, left_size);
    memcpy(render_arr + tail_idx, stats->render_times, left_size);

    return left_elems + right_elems;
}

#if defined(__cplusplus)
} // extern "C" {
#endif

