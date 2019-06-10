#pragma once

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
 *
 * No malloc()s are performed.
 *
 * @param[in] size The size of the array */
bipbuf_t* bipbuf_init(void* data, const unsigned int size);

/**
 * @param[in] data The data to be offered to the buffer
 * @param[in] size The size of the data to be offered
 * @return number of bytes offered */
int bipbuf_push(bipbuf_t *me, const unsigned char *data, const int size);

/**
 * Look at data. Don't move cursor
 *
 * @param[in] len The length of the data to be peeked
 * @return data on success, NULL if we can't peek at this much data */
unsigned char *bipbuf_peek(const bipbuf_t* me, const unsigned int len);

/**
 * Get pointer to data to read. Move the cursor on.
 *
 * @param[in] size The length of the data to be polled
 * @return pointer to data, NULL if we can't poll this much data */
unsigned char *bipbuf_pop(bipbuf_t* me, const unsigned int size);

/**
 * @return the size of the bipbuffer */
int bipbuf_size(const bipbuf_t* me);

/**
 * @return 1 if buffer is empty; 0 otherwise */
int bipbuf_is_empty(const bipbuf_t* me);

/**
 * @return how much space we have assigned */
int bipbuf_used(const bipbuf_t* cb);

/**
 * @return bytes of unused space */
int bipbuf_unused(const bipbuf_t* me);
