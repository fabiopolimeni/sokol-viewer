#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
    int x, y;
    int w, h;
} ta_rect_t;

void* ta_init(
    int width,
    void (*write_pixels_to_texture) (const void *pixels,
                                     const ta_rect_t * rect,
                                     const unsigned int texture),
    int (*create_texture_cb) (const int w,
                              const int h),
    void (*destroy_texture_cb) (const unsigned int texture)
);

void ta_destroy(
    void* att);

int ta_push_pixels(
    void* att,
    const void *pixel_data,
    int w,
    int h
);

int ta_contains_texid(
    const void* att,
    const unsigned long texid
);

void ta_get_coords_from_texid(
    const void* att,
    const unsigned long texid,
    float* begin,
    float* end
);

int ta_get_texture(
    const void* att
);

int ta_get_ntextures(
    const void* att
);

#if defined(__cplusplus)
// extern "C" {
#endif
