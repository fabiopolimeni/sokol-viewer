#include "viewer_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* Size of each input chunk to be
   read and allocate for. */
#ifndef  READALL_CHUNK
#define  READALL_CHUNK  (512*4*8)   // 5kB
#endif

#if defined(__cplusplus)
extern "C" {
#endif

bool file_exists(const char* filename) {
    FILE* fd = fopen(filename, "rb");
    if (fd) {
        fclose(fd);
        return true;
    }

    return false;
}

file_t file_open(const char* filename, uint8_t options) {
    FILE* fd = NULL;

    // this is the only case not taken into account by the fopen, that is,
    // when we want to read from a file, and create it if it doesn't exist.
    if ((options & FILE_OPEN_CREATE|FILE_OPEN_READ) == options) {
        FILE* temp_fd = fopen(filename, "rb");
        if (!temp_fd) {
            temp_fd = fopen(filename, "wb");
        }
        
        fclose(temp_fd);
    }

    int32_t chid = 0;
    char opts[4] = {0};

    if (options & FILE_OPEN_READ) {
        if (options & FILE_OPEN_EOF) {
            opts[0] = 'a';
        }
        else {
            opts[0] = 'r';
        }
        
        chid += 1;
    }

    if (options & FILE_OPEN_WRITE) {
        opts[chid] = chid ? '+' : 'w';
        chid += 1;
    }

    if (options & FILE_OPEN_BINARY) {
        opts[chid] = 'b';
    }

    return (file_t) {.fd = fopen(filename, opts)};
}

void file_close(file_t file) {
    if (file.fd) {
        fclose((FILE*)file.fd);
    }
}

bool file_is_valid(file_t file) {
    return file.fd != NULL;
}

int32_t file_readall(file_t file, char **dataptr, size_t *sizeptr,
    memory_allocator_t allocator) {
    char  *data = NULL, *temp;
    size_t size = 0;
    size_t used = 0;
    size_t n;

    FILE* in = file.fd;

    /* None of the parameters can be NULL. */
    if (in == NULL || dataptr == NULL || sizeptr == NULL)
        return FILE_READALL_INVALID;

    /* A read error already occurred? */
    if (ferror(in))
        return FILE_READALL_ERROR;

    while (1) {
        if (used + READALL_CHUNK + 1 > size) {
            size = used + READALL_CHUNK + 1;

            /* Overflow check. Some ANSI C compilers
               may optimize this away, though. */
            if (size <= used) {
                allocator(data, 0);
                return FILE_READALL_TOOMUCH;
            }

            temp = allocator(data, size);
            if (temp == NULL) {
                allocator(data, 0);
                return FILE_READALL_NOMEM;
            }
            data = temp;
        }

        n = fread(data + used, 1, READALL_CHUNK, in);
        if (n == 0)
            break;

        used += n;
    }

    if (ferror(in)) {
        allocator(data, 0);
        return FILE_READALL_ERROR;
    }

    temp = allocator(data, used + 1);
    if (temp == NULL) {
        allocator(data, 0);
        return FILE_READALL_NOMEM;
    }

    data = temp;
    data[used] = '\0';

    *dataptr = data;
    *sizeptr = used;

    return FILE_READALL_OK;
}

#if defined(__cplusplus)
} //extern "C" {
#endif
