#include "riff.h"

#include "akpk.h"
#include <memory.h>

#define WAVE 0x45564157
#define FMT0 0x20746D66

int wem2wav(void* data) {
  uint32_t size;
  uint32_t chunk;
  uint32_t fmt_size;

  if (*((uint32_t*)data) != RIFF) {
    return -1;
  }

  data = (void*)((uintptr_t)data + 4);
  size = *((uint32_t*)data);

  data = (void*)((uintptr_t)data + 4);
  chunk = *((uint32_t*)data);

  if (chunk != WAVE) {
    char format[5] = {0};
    memcpy(&format, &chunk, 4);
    printf("Only WAVE audio format is supported (here %s)\n", format);
    return -1;
  }

  data = (void*)((uintptr_t)data + 4);
  chunk = *((uint32_t*)data);

  if (chunk != FMT0) {
    return -1;
  }

  data = (void*)((uintptr_t)data + 4);
  fmt_size = *((uint32_t*)data);

  // int out = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

  return 0;
}

// # vim: ts=2 sw=2 expandtab
