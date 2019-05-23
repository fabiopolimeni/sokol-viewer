/*

Copyright (c) 2019, Fabio Polimeni
Copyright (c) 2011, Willem-Hendrik Thiart
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * The names of its contributors may not be used to endorse or promote
      products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL WILLEM-HENDRIK THIART, NOR FABIO POLIMENI, BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/**
 * @file texture_atlas.c
 * @brief A texture atlas for storing multiple textures efficiently for graphics
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "texture_atlas.h"
#include "linked_list_hashmap.h"

#ifndef TA_CALLOC
#define TA_CALLOC TA_CALLOC
#endif

#ifndef TA_FREE
#define TA_FREE free
#endif

typedef struct texture_s ta_texture_t;

struct texture_s {
    ta_texture_t *kids[2];
    ta_rect_t rect;
    int id;
};

typedef struct {

    /* for fast retrieval of texture coordinates */
    ll_hashmap_t *textures;

    ta_texture_t *root;

    /* count how many textures have been inserted */
    int ntextures;

    /* image handle is the texture we manipulate */
    int texture_handle;

    /* call back for writing pixels to texture handle */
    void (*write_pixels_to_texture_cb) (const void *pixels,
					const ta_rect_t * rect,
					const unsigned int texture);

    /* call back for creating texture handle */
    int (*create_texture_cb) (const int w, const int h);

    /* call back for destorying the texture handle */
    void (*destroy_texture_cb) (const unsigned int texture);
} ta_atlas_t;

static unsigned long __ulong_hash(const void *e1)
{
    const long i1 = (unsigned long) e1;
    assert(i1 >= 0);
    return i1;
}

static long __ulong_compare(const void *e1, const void *e2)
{
    const long i1 = (unsigned long) e1, i2 = (unsigned long) e2;
    return i1 - i2;
}

/**
 * Create a texture atlas.
 * @param width The size of this atlas
 * @param write_pixels_to_texture Callback for writing pixels to the atlas
 * @param create_texture_cb Callback for creating a new texture
 * @return a pointer to the newly allocated texture atlas */
void * ta_init(int const width,
    void (*write_pixels_to_texture) (
    const void * pixels,
    const ta_rect_t * rect,
    const unsigned int texture),
    int (*create_texture_cb) (
    const int w, const int h))
{
    ta_atlas_t *at;
    ta_texture_t *tex;

    at = TA_CALLOC(1, sizeof(ta_atlas_t));
    tex = at->root = TA_CALLOC(1, sizeof(ta_texture_t));
    tex->rect.x = 0;
    tex->rect.y = 0;
    tex->rect.w = width;
    tex->rect.h = width;
    at->textures = ll_hashmap_new(__ulong_hash, __ulong_compare, 11);
    at->write_pixels_to_texture_cb = write_pixels_to_texture;
    at->create_texture_cb = create_texture_cb;
    if (at->create_texture_cb)
    {
	at->texture_handle =
	    at->create_texture_cb(tex->rect.w, tex->rect.h);
    }

    return at;
}

static void __remove(ta_texture_t * tex)
{
    if (tex->kids[0]) {
        __remove(tex->kids[0]);
        tex->kids[0] = NULL;
    }

    if (tex->kids[1]) {
        __remove(tex->kids[1]);
        tex->kids[1] = NULL;
    }

    tex->rect.x = 0;
    tex->rect.y = 0;
    tex->rect.w = 0;
    tex->rect.h = 0;
    
    TA_FREE(tex);
}

void ta_destroy(void* att)
{
    assert(att);
    ta_atlas_t *at = att;

    assert(at->root);
    at->destroy_texture_cb(at->texture_handle);

    __remove(at->root);
    at->root = NULL;

    ll_hashmap_freeall(at->textures);
    at->ntextures = 0;

    TA_FREE(at);
}

/*----------------------------------------------------------------------------*/

#if 0
static void __print(ta_texture_t * tex, int depth)
{
    if (!tex)
	return;
    int ii;

    for (ii = 0; ii < depth; ii++)
	printf(" ");
    printf
	("%d,%d,%d,%d %d\n",
	 tex->rect.x, tex->rect.y, tex->rect.w, tex->rect.h, tex->id);
    __print(tex->kids[0], depth + 1);
    __print(tex->kids[1], depth + 1);
}
#endif

static ta_texture_t *__insert(ta_texture_t * tex,
    const int w, const int h, const int id)
{
    assert(tex);

    /* not leaf */
    if (tex->kids[0])
    {
	ta_texture_t *new;

	/* insert at this branch... */
	if ((new = __insert(tex->kids[0], w, h, id)))
	{
	    return new;
	}
	/* ...since that branch didn't work, try the other branch */
	else
	{
	    return __insert(tex->kids[1], w, h, id);
	}
    }
    /* is a leaf */
    else
    {
	/*  fits perfectly.. */
	if (tex->rect.w == w && tex->rect.h == h &&
	    /* ..and isn't in-use */
	    tex->id == 0)
	{
	    tex->id = id;
	    assert(!tex->kids[0]);
	    assert(!tex->kids[1]);
	    return tex;
	}
	/*  doesn't fit */
	else if (tex->rect.w < w || tex->rect.h < h || tex->id != 0)
	{
	    return NULL;
	}
	/* space to fit, but we need to sub-divide */
	else
	{
	    int dw, dh;

	    assert(!tex->kids[0]);
	    assert(!tex->kids[1]);

	    /* alloc memory for sub-divisions */
	    tex->kids[0] = TA_CALLOC(1, sizeof(ta_texture_t));
	    tex->kids[1] = TA_CALLOC(1, sizeof(ta_texture_t));

	    /* create sub-divisions */
	    dw = tex->rect.w - w;
	    dh = tex->rect.h - h;
	    if (dh < dw)
	    {
		tex->kids[0]->rect.x = tex->rect.x;
		tex->kids[0]->rect.y = tex->rect.y;
		tex->kids[0]->rect.w = w;
		tex->kids[0]->rect.h = tex->rect.h;
		tex->kids[1]->rect.x = tex->rect.x + w;
		tex->kids[1]->rect.y = tex->rect.y;
		tex->kids[1]->rect.w = tex->rect.w - w;
		tex->kids[1]->rect.h = tex->rect.h;
	    }
	    else
	    {
		tex->kids[0]->rect.x = tex->rect.x;
		tex->kids[0]->rect.y = tex->rect.y;
		tex->kids[0]->rect.w = tex->rect.w;
		tex->kids[0]->rect.h = h;
		tex->kids[1]->rect.x = tex->rect.x;
		tex->kids[1]->rect.y = tex->rect.y + h;
		tex->kids[1]->rect.w = tex->rect.w;
		tex->kids[1]->rect.h = tex->rect.h - h;
	    }

	    /* insert at the sub-division which is now the perfect size */
	    return __insert(tex->kids[0], w, h, id);
	}
    }
}

/**
 * Write pixels to atlas, return the 'virtual texture id'
 * @param att texture atlas
 * @param pixel_data pixels to write to texture
 * @param w width of pixel data
 * @param h height of pixel data
 * @return 'virtual texture id', otherwise 0 if there wasn't space on the atlas
 */
int ta_push_pixels(void *att,
    const void *pixel_data,
    const int w, const int h)
{
    ta_texture_t *tex;

    ta_atlas_t *at = att;

    unsigned long new_id;

    assert(at);

    new_id = at->ntextures + 1;
    //__print(at->root, 0);

    /* find a new slot on texture atlas */
    if (!(tex = __insert(at->root, w, h, new_id)))
    {
	return 0;
    }

    ll_hashmap_put(at->textures, (void *) new_id, (void *) tex);

    /* put these pixels onto the texture */
    if (at->write_pixels_to_texture_cb)
    {
	at->write_pixels_to_texture_cb(pixel_data, &tex->rect,
				       at->texture_handle);
    }

    at->ntextures += 1;

    return new_id;
}

/**
 * @param att texture atlas 
 * @return true if atlas contains texture id, otherwise false */
int ta_contains_texid(const void *att,
    const unsigned long texid)
{
    const ta_atlas_t *at = att;
    return NULL != ll_hashmap_get(at->textures, (void *) texid);
}

/**
 * Get coordinates using texture id
 * @param att texture atlas 
 * @param texid texture on texture atlas
 * @param begin start texture coordinates
 * @param end end texture coordinates
 */
void ta_get_coords_from_texid(void *att,
    const unsigned long texid,
    float* begin, float* end)
{
    const ta_atlas_t *at = att;

    ta_texture_t *tex;

    assert( ta_contains_texid(att, texid));

    tex = ll_hashmap_get(at->textures, (void *) texid);

    begin[0] = (float) tex->rect.x / at->root->rect.w;
    begin[1] = (float) tex->rect.y / at->root->rect.h;
    
    end[0] = begin[0] + (float) tex->rect.w / at->root->rect.w;
    end[1] = begin[1] + (float) tex->rect.h / at->root->rect.h;
}

/**
 * @param att texture atlas 
 * @return texture handle */
int ta_get_texture(void *att)
{
    const ta_atlas_t *at = att;
    return at->texture_handle;
}

/**
 * @param att texture atlas 
 * @return number of textures */
int ta_get_ntextures(void *att)
{
    const ta_atlas_t *at = att;
    return at->ntextures;
}
