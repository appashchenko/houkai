#include "akpk/akpk.h"
#include <stdio.h>

int main(int argv, char* argc[]) {
  if (argv > 1) {
    akpk_open(argc[1]);
  } else {
    printf("Usage: houkai file.pck\n");
    return 0;
  }
  return 0;
}

// # vim: ts=2 sw=2 expandtab
