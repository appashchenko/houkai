#ifndef RIFF_H
#define RIFF_H

#include <inttypes.h>
#include <stddef.h>

struct audio_header_t {
  uint32_t num_samples;
  uint16_t channels;
  uint32_t sample_rate;
  uint32_t bits_per_sample_avg;
  uint16_t bits_per_sample;
};

typedef struct riff_header_t riff_header_t;

void save_wem(void *data, size_t size, uint64_t id, char *path);
int wem2wav(void *data);
#endif

// # vim: ts=2 sw=2 expandtab
