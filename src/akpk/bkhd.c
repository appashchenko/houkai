#include "bkhd.h"
#include "didx.h"

void read_bkhd(void *data, ssize_t size, char *path) {
  bank_header_t *header = (bank_header_t *)data;

  size_t data_offset =
      sizeof(header->magic) + sizeof(header->size) + header->size;

  ssize_t rem_size = size - data_offset;

  /* soundbank can be empty */
  if (data_offset >= size) {
    /* normally this snould not be happened */
    if (data_offset > size) {
      fprintf(stderr, "BKHD#%X section has wrong size (%lu >= %lu).\n",
            header->soundbank_id, data_offset, size);
    }
    return;
  }

  void *body = (void *)((uintptr_t)data + (uint64_t)data_offset);

  uint32_t section_magic = *((uint32_t *)body);

  switch (section_magic) {
  case DIDX:
    read_didx(body, rem_size, path);
    break;
  case HIRC:
    /* read_hirc(body, path); */
    break;
  case 0:
    /* empty soundbank. strange thing. ignore */
    break;
  case INIT:
    /* wwise init bank. some game parameters here. ignore */
    break;
  case BKHD:
    /* read_bkhd(body, path); */
    break;
  default:
    fprintf(stderr, "!!!! Unknown BKHD section %u\n", section_magic);
    break;
  }
}

/* vim: ts=2 sw=2 expandtab
*/
