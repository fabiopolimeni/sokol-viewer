#include <stdio.h>
#include <stdarg.h>

#define VIEWER_LOG_INFO(...) fprintf(stdout, __VA_ARGS__)
#define VIEWER_LOG_ERROR(...) fprintf(stderr, __VA_ARGS__)
