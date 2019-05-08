#pragma once
/**
 * Common way to manage handles
 */

#define HANDLE_INVALID_ID (-1)

#include <stdint.h>
struct handle_t {
    int32_t id;
};
