//#include "hirc.h"
#include "bkhd.h"
#include "didx.h"

void read_bkhd(void *data, char *path) {
  bank_header_t *header = (bank_header_t *)data;

  data = (void *)((uintptr_t)data + sizeof(header->magic) +
                  sizeof(header->size) + header->size);

  uint32_t section_magic = *((uint32_t *)data);

  switch (section_magic) {
  case DIDX:
    read_didx(data, path);
    break;
  case HIRC:
    // read_hirc(data, path);
    break;
  default:
    fprintf(stderr, "!!!! Unknown BKHD section %u\n", section_magic);
  }
}

// # vim: ts=2 sw=2 expandtab
