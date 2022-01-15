#include "bkhd.h"
#include "didx.h"
#include "util.h"

void read_bkhd(void *data, ssize_t size, char *path) {
  void *body;
  struct bank_header *header = (struct bank_header *)data;
  uint32_t section_magic;
  const size_t data_offset =
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

  body = move_ptr(data, data_offset);
  section_magic = *((uint32_t *)body);

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
    printf("BKHD inside a BKHD#%u. Skip.\n", header->soundbank_id);
    break;
  default:
    fprintf(stderr, "!!!! Unknown BKHD section %u\n", section_magic);
    break;
  }
}

// vim: ts=2 sw=2 expandtab
