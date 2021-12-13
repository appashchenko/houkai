#ifndef DIDX_H
#define DIDX_H

#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

struct didx_header_t {
  uint32_t magic;
  uint32_t enties_size;
};

struct didx_entry_t {
  uint32_t wem_id;
  uint32_t data_offset;
  uint32_t size;
};

struct didx_data_t {
  uint32_t magic;
  uint32_t size;
};

struct wem {
  uint32_t _riff;
};

struct wave_header_broken {
  uint32_t riff;
  uint32_t file_size;
  uint32_t type;
  uint32_t fmt;
  uint32_t bad_size;
};

struct bad_header {
  uint16_t unk1;
  uint16_t channels;
  uint32_t sample_rate;
};

struct good_header {
  uint32_t riff;
  uint32_t file_size;
  uint32_t type;
  uint32_t fmt;
  uint32_t size;
  uint16_t pcm;
  uint16_t channels;
  uint32_t unk1;
  uint32_t sample_rate;
  uint16_t bpsc;
  uint16_t bps;
};
typedef struct good_header good_header;

typedef struct didx_header_t didx_header_t;
typedef struct didx_entry_t didx_entry_t;
typedef struct didx_data_t didx_data_t;

int read_didx(void *data, ssize_t size, char *path);
#endif

/* vim: ts=2 sw=2 expandtab
*/
