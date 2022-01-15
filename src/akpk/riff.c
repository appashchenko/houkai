#ifndef RIFF_H
#define RIFF_H

#define _DEFAULT_SOURCE
#include "riff.h"
#include "akpk.h"
#include "util.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <memory.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define USERGRPFLAGS S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP

#define WAVE 0x45564157
#define FMT0 0x20746D66

/* http://soundfile.sapp.org/doc/WaveFormat */

enum chunk_type {
  /*RIFF_FMT  = 0x666d7420,*/
  RIFF_FMT = 0x20746d66,
  RIFF_WAVE = 0x45564157,
  RIFF_DATA = 0x61746164,
  RIFF_LIST = 0x5453494C,
  RIFF_CUE = 0x20657563,
  RIFF_JUNK = 0x4B4E554A,
  RIFF_AKD = 0x20646B61, /* some dummy padding. idk */
  RIFF_LPMS = 0x6C706D73,
};

void save_wem(void *data, size_t size, uint64_t id, char *path) {
  char fullpath[PATH_MAX];
  unsigned long len;
  int out;
  struct stat sib;

  /* separator/id/.wem/terminator */
  len = strlen(path) + 1 + 16 + 4 + 1;

  snprintf(fullpath, len, "%s/%08lX.wem", path, id);

  if (stat(fullpath, &sib) == 0) {
    if (sib.st_size != size) {
      printf("File %s already exists but have different size.\n", fullpath);
      snprintf(fullpath, len + 1, "%s/%08lX-.wem", path, id);
      printf("Save as %s.\n", fullpath);
    } else {
      return;
    }
  }

  out = open(fullpath, O_CREAT | O_WRONLY, USERGRPFLAGS);

  if (out) {
    ssize_t ret = write(out, data, size);

    if (ret != (ssize_t)size) {
      char *err_str = strerror(errno);
      printf("Failed to save %s: %s\n", fullpath, err_str);
    }
    close(out);
  }
}

enum codec_t { CODEC_PCM, CODEC_WAVE };
typedef enum codec_t codec_t;

struct riff_chunk {
  uint32_t chunk_id;
  uint32_t size;
  uint32_t type;
};

struct fmt_chunk {
  uint32_t chunk_id;
  uint32_t size;
  uint16_t format_tag;
  uint16_t channels;
  uint32_t sample_rate;
  uint32_t avg_byte_rate;
  uint16_t block_size;
  uint16_t bits_per_sample;
};

struct fmt_extra_chunk {
  uint32_t extra_size;
  union {
    uint16_t valid_bits_per_sample;
    uint16_t samples_per_block;
    uint16_t _reserved;
  } channel_layout;
  uint32_t channel_mask;
  struct {
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    uint32_t data4;
    uint32_t data5;
  } guid;
};

struct data_chunk {
  uint32_t chunk_id;
  uint32_t size;
};

struct cue_chunk {
  uint32_t chunk_id;
  uint32_t size;
  uint32_t points;
};

struct cue_point {
  uint32_t chunk_id;
  uint32_t position;
  uint32_t fcc_chunk;
  uint32_t chunk_start;
  uint32_t block_start;
  uint32_t offset;
};

struct list_chunk {
  uint32_t chunk_id;
  uint32_t size;
  uint32_t adtl;
};

struct labl {
  uint32_t chunk_id;
  uint32_t size;
  uint32_t cue_point_id;
};

struct junk_chunk {
  uint32_t chunk_id;
  uint32_t size;
  /* uint8_t data[size]*/
};

struct lpms_chunk {
  uint32_t chunk_id;
  uint32_t size;
  uint32_t manufacturer;
  uint32_t product;
  uint32_t sample_period;
  uint32_t midi_unity_note;
  uint32_t midi_pitch_fraction;
  uint32_t smpte_format;
  uint32_t smpte_offset;
};

void wem_info(void *data) {
  uint32_t chunk;
  struct riff_chunk *riff;
  void *pos = data;

  riff = (struct riff_chunk *)pos;

  if (riff->chunk_id != RIFF) {
    printf("Error while reading WEM: RIFF expected.\n");
    return;
  }

  pos = (void *)((uintptr_t)pos + sizeof(*riff));

  while (pos < (void *)((uintptr_t)data + riff->size)) {
    chunk = *(uint32_t *)pos;

    switch (chunk) {
    case RIFF_FMT: {
      struct fmt_chunk *fmt = (struct fmt_chunk *)pos;
      pos = move_ptr(pos, fmt->size + 4 + 4);
    } break;
    case RIFF_DATA: {
      struct data_chunk *data = (struct data_chunk *)pos;
      pos = move_ptr(pos, 4 + 4 + data->size);
    } break;
    case RIFF_CUE: {
      struct cue_chunk *cue = (struct cue_chunk *)pos;
      pos = move_ptr(pos, 4 + 4 + cue->size);
      /* cue points wanna see? */
    } break;
    case RIFF_LIST: {
      struct list_chunk *list = (struct list_chunk *)pos;
      pos = move_ptr(pos, 4 + 4 + list->size);
    } break;
    case RIFF_AKD:
    case RIFF_JUNK: {
      struct junk_chunk *junk = (struct junk_chunk *)pos;
      pos = move_ptr(pos, 4 + 4 + junk->size);
    } break;
    case RIFF_LPMS: {
      struct lpms_chunk *lpms = (struct lpms_chunk *)pos;
      pos = move_ptr(pos, 4 + 4 + lpms->size);
    } break;
    default:
      printf("Unknown chunk: %X\n", chunk);
      pos = move_ptr(pos, 4);
      break;
    }
  }
}
#endif

// vim: ts=2 sw=2 expandtab
