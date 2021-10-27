#ifndef RIFF_H
#define RIFF_H

#include "riff.h"

#include "akpk.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/stat.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define WAVE 0x45564157
#define FMT0 0x20746D66

void save_wem(void *data, size_t size, uint64_t id, char *path) {
  char *fullpath = NULL;
  unsigned long len;
  int out;

  len = strlen(path) + (sizeof(id) << 2) + 1 + 1;

  fullpath = (char *)malloc(len);

  snprintf(fullpath, len, "%s/%lX.wem", path, id);

  out = open(fullpath, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

  if (out) {
    ssize_t ret = write(out, data, size);

    if (ret != (ssize_t)size) {
      char *err_str = strerror(errno);
      printf("Failed to save %s: %s\n", fullpath, err_str);
    }
    close(out);
  }

  free(fullpath);
}

/*int wem2wav(void *data) {
  uint32_t size;
  uint32_t chunk;
  uint32_t fmt_size;

  if (*((uint32_t *)data) != RIFF) {
    return -1;
  }

  data = (void *)((uintptr_t)data + 4);
  size = *((uint32_t *)data);

  data = (void *)((uintptr_t)data + 4);
  chunk = *((uint32_t *)data);

  if (chunk != WAVE) {
    char format[5] = {0};
    memcpy(&format, &chunk, 4);
    printf("Only WAVE audio format is supported (here %s)\n", format);
    return -1;
  }

  data = (void *)((uintptr_t)data + 4);
  chunk = *((uint32_t *)data);

  if (chunk != FMT0) {
    return -1;
  }

  data = (void *)((uintptr_t)data + 4);
  fmt_size = *((uint32_t *)data);

  // int out = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

  return 0;
}*/
#endif

// # vim: ts=2 sw=2 expandtab
