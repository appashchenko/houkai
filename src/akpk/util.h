#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdint.h>

static inline void* move_ptr(void* ptr, long offset) {
  return (void*)((uintptr_t)ptr + offset);
}

#endif

// vim: ts=2 sw=2 expandtab
