#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Bip buffer structure */
typedef struct
{
    unsigned long int size;

    /* region A */
    unsigned int a_start, a_end;

    /* region B */
    unsigned int b_end;

    /* is B inuse? */
    int b_inuse;

    unsigned char data[];
} bipbuf_t;

/**
 * Initialise a bip buffer. Use memory provided by user.
 * No memory allocations are performed.
 *
 * @param[in] data Memory buffer pointer
 * @param[in] size The size of the whole data buffer
 * 
 * @note: The capacity if the buffer will always be somewhat
 *  smaller than the size param, because it will store the
 *  the header of this bip buffer. */
bipbuf_t* bipbuf_init(void* data, const unsigned int size);

/**
 * @param[in] data The data to be committed to the buffer
 * @param[in] size The size of the data to be committed
 * @return number of bytes committed */
int bipbuf_push(bipbuf_t *me, const unsigned char *data, const int size);

/**
 * Look at data. Don't move cursor
 *
 * @param[in] len The length of the data to be peeked
 * @return data on success, NULL if we can't peek this much data */
unsigned char *bipbuf_peek(const bipbuf_t* me, const unsigned int len);

/**
 * Get pointer to data to read. Move the cursor on.
 *
 * @param[in] size The length of the data to be polled
 * @return pointer to data, NULL if we can't pop this much data */
unsigned char *bipbuf_pop(bipbuf_t* me, const unsigned int size);

/**
 * @return the size of the bipbuffer */
int bipbuf_capacity(const bipbuf_t* me);

/**
 * @return 1 if buffer is empty; 0 otherwise */
int bipbuf_is_empty(const bipbuf_t* me);

/**
 * @return how much space we have assigned */
int bipbuf_used(const bipbuf_t* cb);

/**
 * @return bytes of unused space */
int bipbuf_unused(const bipbuf_t* me);

#if defined(__cplusplus)
} // extern "C" {
#endif
