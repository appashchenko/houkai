#include "akpk/akpk.h"
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>

#define DEBUG

enum { NORMAL, ANALYZE } mode = NORMAL;


int main(int argc, char *argv[]) {
  if (argc > 1) {
    akpk_open(argv[1]);
  } else {
    printf("Usage: houkai <file.pck>\n");
  }
  return 0;
}

// # vim: ts=2 sw=2 expandtab
