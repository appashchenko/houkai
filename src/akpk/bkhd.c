#include "hirc.h"
#include "bkhd.h"

void read_bkhd(void *data) {
  bank_header_t *header = (bank_header_t *)data;

#ifdef VERBOSE
  printf("*  BKHD#%X from %X\n", header->bank_id, header->soundbank_id);
#endif
  data = (void *)((uintptr_t)data + sizeof(header->magic) +
                  sizeof(header->size) + header->size);

  uint32_t section_magic = *((uint32_t *)data);

  switch (section_magic) {
  case DIDX:
    puts("*   DIDX\n");
    read_didx(data);
    break;
  case HIRC:
    puts("*   HIRC\n");
    read_hirc(data);
    break;
  default:
    fprintf(stderr, "!!!! Unknown BKHD section %u\n", section_magic);
  }
}

// # vim: ts=2 sw=2 expandtab
