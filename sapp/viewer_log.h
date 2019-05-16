#include <stdio.h>
#include <stdarg.h>

#define LOG_INFO(...) fprintf(stdout, __VA_ARGS__)
#define LOG_WARN(...) fprintf(stderr, __VA_ARGS__)
#define LOG_ERROR(...) fprintf(stderr, __VA_ARGS__)
