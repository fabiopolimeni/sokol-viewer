#pragma once
/**
 * Simple file interface
 */

#include <stdbool.h>

#include "viewer_memory.h"

#define  FILE_READALL_OK          0  /* Success */
#define  FILE_READALL_INVALID    -1  /* Invalid parameters */
#define  FILE_READALL_ERROR      -2  /* Stream error */
#define  FILE_READALL_TOOMUCH    -3  /* Too much input */
#define  FILE_READALL_NOMEM      -4  /* Out of memory */

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
    FILE_OPEN_READ      = 0x00, // open the file to be read
    FILE_OPEN_WRITE     = 0x01, // open the file to be written
    FILE_OPEN_EOF       = 0x02, // open the file from the end of it
    FILE_OPEN_CREATE    = 0x04, // create the file if it doesn't exist
    FILE_OPEN_BINARY    = 0x08 // open in binary mode 
} file_open_options_t;

typedef struct {
    void* fd;
} file_t;

/**
 * Returns whether or not the filename points to an existing file.
 */
bool file_exists(const char* filename);

/**
 * Open a file with a combination of the file_open_action bitset.
 * 
 * @return opened stream file descriptor in case of success, null otherwise.
 */
file_t file_open(const char* filename, uint8_t options);

/**
 * Close the file handle pointed by file.
 */
void file_close(file_t file);

/**
 * Returns whether the file object is valid or not
 */
bool file_is_valid(file_t file);

/**
 * This function returns one of the FILE_READALL_ constant.
 * If the return value is zero == READALL_OK, then:
     (*dataptr) points to a dynamically allocated buffer, with
     (*sizeptr) chars read from the file.
     The buffer is allocated for one extra char, which is NUL,
     and automatically appended after the data.
    Initial values of (*dataptr) and (*sizeptr) are ignored.
*/
int32_t file_readall(
    file_t file, char **dataptr, size_t *sizeptr,
    memory_allocator_t allocator);

#if defined(__cplusplus)
} // extern "C" {
#endif
