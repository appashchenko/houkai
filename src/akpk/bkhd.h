#ifndef BKHD_H
#define BKHD_H

#include "akpk.h"
#include <string.h>

struct bank_header_t {
  uint32_t magic;
  uint32_t size;
  uint32_t revision;
  uint32_t soundbank_id;
  uint32_t bank_id;
  uint32_t _reserved;
  uint32_t version;
};
typedef struct bank_header_t bank_header_t;

#endif

// # vim: ts=2 sw=2 expandtab
