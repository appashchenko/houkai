#ifndef BKHD_H
#define BKHD_H

#include "akpk.h"
#include <string.h>

struct bkhd_header_t {
  uint32_t magic;
  uint32_t size;
};
typedef struct bkhd_header_t bkhd_header_t;

struct bkhd_info_t {
  uint32_t revision;
  uint32_t soundbank_id;
  uint32_t bank_id;
  uint32_t _reserved;
  uint32_t version;
};
typedef struct bkhd_info_t bkhd_info_t;

void parse_bkhd32(const soundbank_entry32_t* sb, void* data);

#endif
