/*
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
DISCLAIMED. IN NO EVENT SHALL WILLEM-HENDRIK THIART BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "bip_buffer.h"

/* for memcpy */
#include <string.h>
#include <assert.h>

#if defined(__cplusplus)
extern "C" {
#endif

int bipbuf_unused(const bipbuf_t* me) {
    if (1 == me->b_inuse)
        /* distance between region B and region A */
        return me->a_start - me->b_end;
    else
        return me->size - me->a_end;
}

int bipbuf_capacity(const bipbuf_t* me) {
    return me->size;
}

int bipbuf_used(const bipbuf_t* me) {
    return (me->a_end - me->a_start) + me->b_end;
}

bipbuf_t* bipbuf_init(void* data, const unsigned int size) {
    assert(size >= sizeof(bipbuf_t));

    bipbuf_t *me = data;
    me->a_start = me->a_end = me->b_end = 0;
    me->size = size - sizeof(bipbuf_t);
    me->b_inuse = 0;

    return me;
}

int bipbuf_is_empty(const bipbuf_t* me) {
    return me->a_start == me->a_end;
}

/* find out if we should turn on region B
 * ie. is the distance from A to buffer's end less than B to A? */
static void __check_for_switch_to_b(bipbuf_t* me) {
    if (me->size - me->a_end < me->a_start - me->b_end)
        me->b_inuse = 1;
}

int bipbuf_push(bipbuf_t* me, const unsigned char *data, const int size) {
    /* not enough space */
    if (bipbuf_unused(me) < size)
        return 0;

    if (1 == me->b_inuse) {
        memcpy(me->data + me->b_end, data, size);
        me->b_end += size;
    }
    else {
        memcpy(me->data + me->a_end, data, size);
        me->a_end += size;
    }

    __check_for_switch_to_b(me);
    return size;
}

unsigned char *bipbuf_peek(const bipbuf_t* me, const unsigned int size) {
    /* make sure we can actually peek at this data */
    if (me->size < me->a_start + size)
        return NULL;

    if (bipbuf_is_empty(me))
        return NULL;

    return (unsigned char*)me->data + me->a_start;
}

unsigned char *bipbuf_pop(bipbuf_t* me, const unsigned int size) {
    if (bipbuf_is_empty(me))
        return NULL;

    /* make sure we can actually poll this data */
    if (me->size < me->a_start + size)
        return NULL;

    void *end = me->data + me->a_start;
    me->a_start += size;

    /* we seem to be empty.. */
    if (me->a_start == me->a_end)
    {
        /* replace a with region b */
        if (1 == me->b_inuse)
        {
            me->a_start = 0;
            me->a_end = me->b_end;
            me->b_end = me->b_inuse = 0;
        }
        else
            /* safely move cursor back to the start because we are empty */
            me->a_start = me->a_end = 0;
    }

    __check_for_switch_to_b(me);
    return end;
}

#if defined(__cplusplus)
} // extern "C" {
#endif
