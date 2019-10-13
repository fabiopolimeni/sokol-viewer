#pragma once
/**
 * Simple path interface
 */

#include <string.h>
#include <stdint.h>

#define PATH_MAX_PATH 1024
#define PATH_MAX_EXT 32

#if defined(__cplusplus)
extern "C" {
#endif

// Copies path to out, but not the extension. Places a nul terminator in out.
// Returns the length of the string in out, excluding the nul byte.
// Length of copied output can be up to PATH_MAX_PATH. Can also copy the file
// extension into ext, up to PATH_MAX_EXT.
int32_t path_pop_ext(const char* path, char* out, char* ext);

// Copies path to out, but excludes the final file or folder from the output.
// If the final file or folder contains a period, the file or folder will
// still be appropriately popped. If the path contains only one file or folder,
// the output will contain a period representing the current directory. All
// outputs are nul terminated.
// Returns the length of the string in out, excluding the nul byte.
// Length of copied output can be up to PATH_MAX_PATH.
// Optionally stores the popped filename in pop. pop can be NULL.
// out can also be NULL.
int32_t path_pop(const char* path, char* out, char* pop);

// Concatenates path_b onto the end of path_a. Will not write beyond 
// max_buffer_length. Places a single '/' character between path_a and path_b.
// Does no other "intelligent" manipulation of path_a and path_b; 
// it's a basic strcat kind of function.
void path_concat(const char* path_a, const char* path_b, char* out,
	int32_t max_buffer_length);

#if defined(__cplusplus)
} // extern "C" {
#endif
