#define _DEFAULT_SOURCE
#include "akpk.h"
#include "bkhd.h"
#include "didx.h"
#include "riff.h"
#include "sys/sysinfo.h"
#include "util.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/limits.h>
#include <memory.h>
#include <semaphore.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <threads.h>
#include <uchar.h>
#include <unistd.h>

static int stream_fd = 0;
static char *get_language_str(struct lang *languages, uint32_t id);
static int read_soundbank(uint64_t id, ssize_t sz, uint32_t len, char *path);
static char *basename(const char *filepath);
static char *read16to8(char16_t *string);
static void *sbbuf = NULL;

void akpk_open(const char *filepath) {
  struct stat sib;
  void *header_data = NULL;
  void *header_data_ptr = NULL;
  struct akpk_header header;
  char *base = NULL;
  struct lang *languages = NULL;
  unsigned long base_len;
  uint32_t i;
  int ret;

  if ((stream_fd = open(filepath, O_RDONLY, O_NOFOLLOW)) == 0) {
    fprintf(stderr, "Failed to open file %s. Exit\n", filepath);
    goto clean;
  }

  if (fstat(stream_fd, &sib) == -1) {
    perror("Failed to get file info");
    goto clean;
  }

  if (S_ISREG(sib.st_mode) != 1) {
    fprintf(stderr, "%s is not a regular file. Exit\n", filepath);
    goto clean;
  }

  if (read(stream_fd, &header, sizeof(header)) != sizeof(header)) {
    fprintf(stderr, "Failed to read file header. Exit\n");
    goto clean;
  }

  if (header.magic != AKPK) {
    fprintf(stderr, "File is not a valid AKPK archive. Exit\n");
    goto clean;
  }

  if (header.version != 1) {
    fprintf(stderr, "Supported version 1 but file version is %u. Exit\n",
            header.version);
    goto clean;
  }

  base = basename(filepath);

  if (base == NULL) {
    fprintf(stderr, "Could not allocate memory for basename: %s\n",
            strerror(errno));
    goto clean;
  }

  ret = mkdir(base, S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH | S_IROTH);
  if (ret != 0 && errno != EEXIST) {
    char *error = strerror(ret);
    fprintf(stderr, "Failed to create directory %s\n%s\n", base, error);
    goto clean;
  }

  base_len = strlen(base);

  /* Read remaining header data (languages map, soundbanks look-up tables) */
  {
    uint32_t size = header.lang_map_size + header.sb_lut_size +
                    header.stm_lut_size + header.ext_lut_size;
    header_data = malloc(size);
    if (header_data == NULL) {
      fprintf(stderr, "Could not allocate memory PCK header: %s\n",
              strerror(errno));
      goto clean;
    }

    header_data_ptr = header_data;

    if (read(stream_fd, header_data, size) != size) {
      perror("Failed to read AKPK info header: ");
      goto clean;
    }
  }

  /* Read the languages map */
  {
    uint32_t count = *((uint32_t *)header_data_ptr);

    if (count > 0) {
      struct lang_entry *lang_map;
      unsigned long total_len;
      char *path = NULL;
      char *tmp = NULL;

      languages = (struct lang *)calloc(count + 1, sizeof(struct lang));
      if (languages == NULL) {
        fprintf(stderr, "Could not allocate memory for language map: %s\n",
                strerror(errno));
        goto clean;
      }

      lang_map = move_ptr(header_data_ptr, sizeof(count));

      for (i = 0; i < count; i++) {
        languages[i].id = lang_map[i].id;
        languages[i].name = read16to8(
            (char16_t *)(move_ptr(header_data_ptr, lang_map[i].offset)));

        if (languages[i].name == NULL) {
          fprintf(stderr, "Could not allocate memory for language: %s\n",
                  strerror(errno));
          goto clean;
        }

        total_len = base_len + 1 + strlen(languages[i].name) + 1;

        tmp = (char *)realloc(path, total_len);
        if (tmp == NULL) {
          fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
          if (path != NULL) {
            free(path);
          }
          goto clean;
        } else {
          path = tmp;
        }

        snprintf(path, total_len, "%s/%s", base, languages[i].name);

        ret = mkdir(path,
                    S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH | S_IROTH);
        if (ret != 0 && errno != EEXIST) {
          char *error = strerror(ret);
          fprintf(stderr, "Failed to create directory %s: %s\n", path, error);
          free(path);
          goto clean;
        }
      }
      languages[count].id = 0;
      languages[count].name = NULL;
      free(path);
    }
    header_data_ptr = move_ptr(header_data_ptr, header.lang_map_size);
  }

  /* Read soundbanks */
  {
    uint32_t count = *((uint32_t *)header_data_ptr);

    if (count > 0) {
      char *path = NULL;
      char *tmp = NULL;
      struct soundbank_entry32 *sb = move_ptr(header_data_ptr, sizeof(count));

      for (i = 0; i < count; i++) {
        char *lang = get_language_str(languages, sb[i].language_id);
        unsigned long total_len = base_len + strlen(lang) + 1 + 1;

        tmp = (char *)realloc(path, total_len);
        if (tmp == NULL) {
          fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
          if (path != NULL) {
            free(path);
          }
          goto clean;
        } else {
          path = tmp;
        }

        snprintf(path, total_len, "%s/%s", base, lang);

        if (read_soundbank(sb[i].id, sb[i].block_size * sb[i].file_size,
                           sb[i].start_block, path) < 0) {
          free(path);
          goto clean;
        }
      }
      free(path);
    }
    header_data_ptr = move_ptr(header_data_ptr, header.sb_lut_size);
  }

  /* Read STM soundbanks */
  {
    uint32_t count = *((uint32_t *)header_data_ptr);

    if (count > 0) {
      char *path = NULL;
      char *tmp = NULL;
      struct soundbank_entry32 *sb = move_ptr(header_data_ptr, sizeof(count));

      for (i = 0; i < count; i++) {
        char *lang = get_language_str(languages, sb[i].language_id);
        unsigned long total_len = base_len + strlen(lang) + 1 + 1;

        tmp = (char *)realloc(path, total_len);
        if (tmp == NULL) {
          fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
          if (path != NULL) {
            free(path);
          }
          goto clean;
        } else {
          path = tmp;
        }

        snprintf(path, total_len, "%s/%s", base, lang);

        if (read_soundbank(sb[i].id, sb[i].block_size * sb[i].file_size,
                           sb[i].start_block, path) < 0) {
          free(path);
          goto clean;
        }
      }
      free(path);
    }
    header_data_ptr = move_ptr(header_data_ptr, header.stm_lut_size);
  }

  /* Read EXTERNALS */
  {
    uint32_t count = *((uint32_t *)header_data_ptr);

    if (count > 0) {
      char *path = NULL;
      char *tmp = NULL;
      struct soundbank_entry64 *sb = move_ptr(header_data_ptr, sizeof(count));

      for (i = 0; i < count; i++) {
        char *lang = get_language_str(languages, sb[i].language_id);
        unsigned long total_len = base_len + strlen(lang) + 1 + 1;

        tmp = (char *)realloc(path, total_len);
        if (tmp == NULL) {
          fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
          if (path != NULL) {
            free(path);
          }
          goto clean;
        } else {
          path = tmp;
        }

        snprintf(path, total_len, "%s/%s", base, lang);

        if (read_soundbank(sb[i].id, sb[i].block_size * sb[i].file_size,
                           sb[i].start_block, path) < 0) {
          free(path);
          goto clean;
        }
      }
      free(path);
    }
  }

clean:
  if (languages) {
    struct lang *lang = languages;
    while (lang->name != NULL) {
      free(lang->name);
      lang++;
    }
    free(languages);
  }

  if (sbbuf) {
    free(sbbuf);
  }

  if (base) {
    free(base);
  }
  if (header_data) {
    free(header_data);
  }
  if (stream_fd) {
    close(stream_fd);
  }
}

static int read_soundbank(uint64_t id, ssize_t size, uint32_t offset,
                          char *path) {
  sbbuf = realloc(sbbuf, (size_t)size);

  if (sbbuf == NULL) {
    fprintf(stderr, "Could not allocate memory for soundbank: %s\n",
            strerror(errno));
    free(sbbuf);
    return -1;
  }

  if (pread(stream_fd, sbbuf, size, offset) == size) {
    uint32_t magic = *((uint32_t *)sbbuf);
    switch (magic) {
    case BKHD:
      read_bkhd(sbbuf, size, path);
      break;
    case RIFF: {
      wem_info(sbbuf);
      save_wem(sbbuf, size, id, path);
    } break;
    default:
      fprintf(stderr, "Unknown soundbank container type: %X\n", magic);
    }
  } else {
    fprintf(stderr, "Failed to read file. Exit.\n");
  }

  return 0;
}

char *get_language_str(struct lang *languages, uint32_t id) {
  static char *none = "";
  char *ret = none;

  struct lang *p = languages;
  while (p->name != NULL) {
    if (p->id == id) {
      ret = p->name;
      break;
    }
    p++;
  }

  return ret;
}

char *basename(const char *filepath) {
  char *filebasename;
  char *p = (char *)filepath;
  char *filename = (char *)filepath;
  unsigned long len = 0;

  while (*p != '\0') {
    if (*p == '/' || *p == '\\') {
      filename = p + 1;
    }
    p++;
  }

  p = filename;
  while (*p != '\0') {
    if (*p == '.') {
      break;
    }
    len++;
    p++;
  }

  filebasename = (char *)malloc(len + 1);
  if (filebasename == NULL) {
    return NULL;
  }

  memcpy(filebasename, filename, len);
  filebasename[len] = '\0';

  return filebasename;
}

char *read16to8(char16_t *str16) {
  char *char8str;
  unsigned long len = 0;
  unsigned long i;

  while (str16[len] != 0) {
    len++;
  }

  char8str = (char *)malloc(len + 1);
  if (char8str == NULL) {
    return NULL;
  }

  for (i = 0; i <= len; i++) {
    char8str[i] = (char)str16[i];
  }
  /* char8str[len] = '\0'; */

  return char8str;
}

/* vim: ts=2 sw=2 expandtab
 */
