#ifndef RIFF_H
#define RIFF_H

#include <inttypes.h>

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
    uint16_t extra_size;
    uint32_t channel_layout;
};

typedef struct riff_header_t riff_header_t;

#endif
