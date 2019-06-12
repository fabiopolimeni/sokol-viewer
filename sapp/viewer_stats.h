#pragma once

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    float* update_times;
    float* render_times;
    
    float total_update_time;
    float total_render_time;
    float total_frames_time;
    
    uint32_t stored_frames;
    uint32_t max_frames;
} stats_t;

/**
 * Initialise the stats object.
 * 
 * @param[in] max_frames The number of frames to store
 */
void stats_init(stats_t* stats, uint32_t max_frames);
void stats_clean(stats_t* stats);

/**
 * Add one frame timing info to the stats object, and updates it.
 */
void stats_tick(stats_t* stats, float update_time, float render_time);

/**
 * Load into update/render arrays frame times.
 * Arrays must be able to store at least stats->max_frames slots.
 * 
 * @param[out] update Update timings array
 * @param[out] render Render timings array
 * @param[out] avg Average frames time over the stored frames
 * @return The number of frames stored
 */
uint32_t stats_get_timings(const stats_t* stats,
    float update_arr[], float render_arr[]);

#if defined(__cplusplus)
} // extern "C" {
#endif
