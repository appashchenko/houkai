#include "didx.h"
#include "akpk.h"
#include "riff.h"
#include "util.h"
#include <limits.h>
#include <stdio.h>

extern char *sbdir;
extern struct lang *languages;
extern char cur_path[255];

int read_didx(void *data, ssize_t size, char *path) {
  struct didx_entry *entry;
  struct didx_entry *data_header;
  void *data_start;
  void *wem;

  struct didx_header *header = (struct didx_header *)data;
  ssize_t rem_size =
      size - (ssize_t)sizeof(struct didx_header) - header->enties_size;

  if (rem_size <= 0) {
    goto fail;
  }

  entry = move_ptr(data, sizeof(*header));
  data_header = move_ptr(entry, header->enties_size);
  data_start = move_ptr(data_header, sizeof(struct didx_data));

  for (; entry < data_header; entry++) {
    rem_size = rem_size - entry->size;
    if (rem_size < 0) {
      printf("last entry size is %u\n", entry->size);
      goto fail;
    }

    wem = move_ptr(data_start, entry->data_offset);
    wem_info(wem);
    save_wem(wem, entry->size, entry->wem_id, path);
  }

  return 0;
fail:
  fprintf(stderr, "DIDX read out of boundaries by %ld bytes.\n", rem_size);
  return -1;
}

// vim: ts=2 sw=2 expandtab
