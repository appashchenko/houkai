#ifndef RIFF_H
#define RIFF_H

#include <inttypes.h>
#include <stddef.h>

struct riff_header_t {
  uint32_t riff;
  uint32_t size;
  uint32_t type;
  uint32_t fmt;
  uint32_t fmt_size;
  uint16_t format;
  uint16_t channels;
  uint32_t sample_rate;
  uint32_t average_bits_per_sample;
  uint16_t block_aligh;
  uint16_t bits_per_sample;
};

typedef struct riff_header_t riff_header_t;

int wem2wav(void *data, size_t size);

#endif

// # vim: ts=2 sw=2 expandtab
