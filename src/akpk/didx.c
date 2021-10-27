#include "didx.h"
#include "akpk.h"
#include "riff.h"
#include <limits.h>
#include <stdio.h>

extern char *sbdir;
extern lang_list_t languages;
extern char cur_path[255];

void read_didx(void *data, char *path) {
  didx_header_t *header = (didx_header_t *)data;
  didx_entry_t *entry =
      (didx_entry_t *)((uintptr_t)data + sizeof(didx_header_t));
  didx_entry_t *data_header =
      (didx_entry_t *)((uintptr_t)entry + header->enties_size);
  void *data_start = (void *)((uintptr_t)data_header + sizeof(didx_data_t));

  for (; entry < data_header; entry++) {
    void *wem = (void *)((uintptr_t)data_start + entry->data_offset);
    save_wem(wem, entry->size, entry->wem_id, path);
  }
}

// # vim: ts=2 sw=2 expandtab
