#ifndef RIFF_H
#define RIFF_H

#include <stdbool.h>
#define _DEFAULT_SOURCE
#include "riff.h"
#include "akpk.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define WAVE 0x45564157
#define FMT0 0x20746D66

enum chunk_type {
  WMA_RIFF = 0x46464952,
  WMA_FMT = 0x666d7420,
  WMA_WAVE = 0x57415645,
  WMA_DATA = 0x64617461,
  WMA_LIST = 0x5453494C
};


void save_wem(void *data, size_t size, uint64_t id, char *path) {
  char *fullpath = NULL;
  unsigned long len;
  int out;
  struct stat sib;

  len = strlen(path) + (sizeof(id) << 2) + 1 + 1;

  fullpath = (char *)malloc(len);

  if (fullpath == NULL) {
    fprintf(stderr, "Could not allocate memory for full file path: %s\n",
            strerror(errno));
    return;
  }

  snprintf(fullpath, len, "%s/%lX.wem", path, id);


  if (stat(fullpath, &sib) == 0) {
    if (sib.st_size == size) {
      printf("File %s already exists and have same size. Skip.\n", fullpath);
      return;
    } else {
      printf("File %s already exists but have different size. Override.\n", fullpath);
    }
  }

  out =
      open(fullpath, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

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

struct waveformatex {
  uint16_t tag;
  uint16_t channels;
  uint32_t samples_per_sec;
  uint32_t average_bps;
  uint16_t block_align;
  uint16_t bps;
  uint16_t extra_size;
};

struct waveformatextensible {
  union {
    uint16_t valid_bps;
    uint16_t samples_per_block;
    uint16_t _reserved;
  } samples;
  uint32_t channel_mask;
  uint64_t subformat;
};

enum codec_t { CODEC_NONE, CODEC_PCM, CODEC_WAVE };
typedef enum codec_t codec_t;

/*
int wem2wav(void *data) {
  uint32_t size;
  uint32_t chunk;

  void *fmt;
  uint32_t fmt_size;
  codec_t codec = CODEC_NONE;
  uint16_t format;
  uint16_t channels;
  uint32_t sample_rate;
  uint32_t average_bps;
  uint16_t block_align;
  uint16_t bps;
  uint16_t extra_size;

  off_t offset = 0;

  void *p = data;

  if (*((uint32_t *)data) != RIFF) {
    printf("Error while reading WEM: RIFF expected.\n");
    return -1;
  }

  p = (void *)((uintptr_t)p + 4);
  size = *((uint32_t *)p);

  while (offset < size) {
    p = (void *)((uintptr_t)p + offset);

    chunk = *(uint32_t *)p;

    switch (chunk) {
    case WMA_WAVE:
      codec = CODEC_WAVE;
      offset += 4;
      break;
    case WMA_FMT:
      fmt = p;
      fmt_size = *(uint32_t *)((uintptr_t)p + 4);
      format = *(uint16_t *)((uintptr_t)p + 8);
      channels = *(uint16_t *)((uintptr_t)p + 10);
      sample_rate = *(uint32_t *)((uintptr_t)p + 12);
      average_bps = *(uint32_t *)((uintptr_t)p + 16);
      block_align = *(uint16_t *)((uintptr_t)p + 20);
      bps = *(uint16_t *)((uintptr_t)p + 22);
      extra_size = *(uint16_t *)((uintptr_t)p + 24);

      offset += fmt_size;
      break;
    default:
      offset += 4;
      break;
    }
  }

  data = (void *)((uintptr_t)data + 4);
  chunk = *((uint32_t *)data);

  return 0;
}*/
#endif

/* vim: ts=2 sw=2 expandtab
 */
