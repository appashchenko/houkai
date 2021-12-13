#include "didx.h"
#include "akpk.h"
#include "riff.h"
#include <limits.h>
#include <stdio.h>

extern char *sbdir;
extern lang_list_t languages;
extern char cur_path[255];

int read_didx(void *data, ssize_t size, char *path) {
  didx_header_t *header = (didx_header_t *)data;
  ssize_t rem_size =
      size - (ssize_t)sizeof(didx_header_t) - header->enties_size;

  if (rem_size <= 0) {
    goto fail;
  }

  didx_entry_t *entry =
      (didx_entry_t *)((uintptr_t)data + sizeof(didx_header_t));
  didx_entry_t *data_header =
      (didx_entry_t *)((uintptr_t)entry + header->enties_size);
  void *data_start = (void *)((uintptr_t)data_header + sizeof(didx_data_t));

  for (; entry < data_header; entry++) {

    rem_size = rem_size - entry->size;
    if (rem_size < 0) {
      printf("last entry size is %u\n", entry->size);
      goto fail;
    }

    void *wem = (void *)((uintptr_t)data_start + entry->data_offset);
    save_wem(wem, entry->size, entry->wem_id, path);
  }

  return 0;
fail:
  fprintf(stderr, "DIDX read out of boundaries by %ld bytes.\n", rem_size);
  return -1;
}

/* vim: ts=2 sw=2 expandtab
*/
