#ifndef WMV_H
#define WMV_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define MAGIC 0x556e6974794653

enum COMPRESSION_MODE {
  COMPRESSION_NONE = 0,
  COMPRESSION_LZMA = 1,
  COMPRESSION_LZ4 = 2,
  COMPRESSION_LZ4HC = 3,
  COMPRESSION_LZHAM = 4,
  // not in unity defined
  COMPRESSION_LZFSE = 10,
  COMPRESSION_ZLIB = 11
};

struct unityfs_header_t {
  char signature[8];
  uint32_t version;
  uint32_t key;
  uint32_t flag;
  size_t bundleSize;
  uint32_t decompressedSize;
  uint32_t compressedSize;
  uint8_t flags[4];
};

int wmv_readHeader(FILE *, struct unityfs_header_t *);

#endif // WMV_H
