#define TINYOBJ_LOADER_C_IMPLEMENTATION

#include "../viewer_memory.h"

#define TINYOBJ_MALLOC memory_malloc
#define TINYOBJ_REALLOC memory_realloc
#define TINYOBJ_CALLOC memory_calloc
#define TINYOBJ_FREE memory_free
#include "tinyobj_loader_c.h"
