#include "akpk/akpk.h"
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#define DEBUG

int main(int argc, char *argv[]) {
  if (argc > 1) {
    int i;
    for (i = 1; i < argc; i++) {
      akpk_open(argv[i]);
    }
  } else {
    printf("Usage: houkai <file.pck>\n");
  }
  return 0;
}

/* vim: ts=2 sw=2 expandtab
*/

