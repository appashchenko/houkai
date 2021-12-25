#ifndef DIDX_H
#define DIDX_H

#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

struct didx_header {
  uint32_t magic;
  uint32_t enties_size;
};

struct didx_entry {
  uint32_t wem_id;
  uint32_t data_offset;
  uint32_t size;
};

struct didx_data {
  uint32_t magic;
  uint32_t size;
};

int read_didx(void *data, ssize_t size, char *path);
#endif

/* vim: ts=2 sw=2 expandtab
 */
