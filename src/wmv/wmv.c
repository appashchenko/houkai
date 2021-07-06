#include "wmv.h"

int wmv_readHeader(FILE* archive, struct unityfs_header_t* header) {
  if (archive) {
    rewind(archive);
    fread(header, sizeof(struct unityfs_header_t), 1, archive);

    return 1;
  }
  return 0;
}
