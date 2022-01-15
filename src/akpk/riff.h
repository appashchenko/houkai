#ifndef RIFF_H
#define RIFF_H

#include <inttypes.h>
#include <stddef.h>

void save_wem(void *data, size_t size, uint64_t id, char *path);
void wem_info(void *data);
/*int wem2wav(void *data);*/
#endif

// vim: ts=2 sw=2 expandtab
