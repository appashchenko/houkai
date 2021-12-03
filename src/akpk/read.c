#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#define _GNU_SOURCE
#include "akpk.h"
#include "bkhd.h"
#include "didx.h"
#include "riff.h"
#include "sys/sysinfo.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <memory.h>
#include <semaphore.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <threads.h>
#include <uchar.h>
#include <unistd.h>

static int stream_fd = 0;
static char *get_language_str(lang_list_t languages, uint32_t id);
static int read_soundbank(uint64_t id, ssize_t sz, uint32_t len, char *path);
static char *basename(const char *filepath);
static char *read16to8(char16_t *string);
static void *sbbuf = NULL;

void akpk_open(const char *filepath) {
  struct stat sib;
  void *header_data = NULL;
  void *header_data_ptr = NULL;
  akpk_header_t header;
  char *base = NULL;
  lang_list_t languages = NULL;
  char *path = NULL;
  unsigned long base_len;

  if ((stream_fd = open(filepath, O_RDONLY, O_NOFOLLOW)) == 0) {
    fprintf(stderr, "Failed to open file %s. Exit\n", filepath);
    goto clean;
  }

  if (fstat(stream_fd, &sib) == -1) {
    perror("Failed to get file info");
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

  int ret =
      mkdir(base, S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH | S_IROTH);
  if (ret != 0 && errno != EEXIST) {
    char *error = strerror(ret);
    fprintf(stderr, "Failed to create directory %s\n%s\n", base, error);
    goto clean;
  }

  base_len = strlen(base);

  // Read remaining header data (languages map, soundbanks look-up tables)
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

  // Read the languages map
  {
    uint32_t count = *((uint32_t *)header_data_ptr);

    if (count > 0) {
      languages = (lang_list_t)calloc(count + 1, sizeof(lang_t));
      if (languages == NULL) {
        fprintf(stderr, "Could not allocate memory for language map: %s\n",
                strerror(errno));
        goto clean;
      }

      lang_entry_t *lang_map =
          (lang_entry_t *)((uintptr_t)header_data_ptr + sizeof(count));

      for (uint32_t i = 0; i < count; i++) {
        languages[i].id = lang_map[i].id;
        languages[i].name = read16to8(
            (char16_t *)((uintptr_t)header_data_ptr + lang_map[i].offset));

        if (languages[i].name == NULL) {
          fprintf(stderr, "Could not allocate memory for language: %s\n",
                  strerror(errno));
          goto clean;
        }

        unsigned long total_len = base_len + strlen(languages[i].name) + 1 + 1;

        path = (char *)malloc(total_len);
        if (path == NULL) {
          fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
          goto clean;
        }

        snprintf(path, total_len, "%s/%s", base, languages[i].name);

        int ret = mkdir(path, S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH |
                                  S_IROTH);
        if (ret != 0 && errno != EEXIST) {
          char *error = strerror(ret);
          fprintf(stderr, "Failed to create directory %s\n%s\n", path, error);
          goto clean;
        }
        free(path);
      }
      languages[count].id = 0;
      languages[count].name = NULL;
    }
    header_data_ptr =
        (void *)((uintptr_t)header_data_ptr + header.lang_map_size);
  }

  // Read soundbanks
  {
    uint32_t count = *((uint32_t *)header_data_ptr);

    if (count > 0) {
      soundbank_entry32_t *sb =
          (soundbank_entry32_t *)((uintptr_t)header_data_ptr + sizeof(count));

      for (uint32_t i = 0; i < count; i++) {
        char *lang = get_language_str(languages, sb[i].language_id);
        unsigned long total_len = base_len + strlen(lang) + 1 + 1;

        path = (char *)malloc(total_len);
        if (path == NULL) {
          fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
          goto clean;
        }

        snprintf(path, total_len, "%s/%s", base, lang);

        if (read_soundbank(sb[i].id, sb[i].block_size * sb[i].file_size,
                           sb[i].start_block, path) < 0) {
          free(path);
          goto clean;
        }
        free(path);
      }
    }
    header_data_ptr = (void *)((uintptr_t)header_data_ptr + header.sb_lut_size);
  }

  // Read STM soundbanks
  {
    uint32_t count = *((uint32_t *)header_data_ptr);

    if (count > 0) {
      soundbank_entry32_t *sb =
          (soundbank_entry32_t *)((uintptr_t)header_data_ptr + sizeof(count));

      for (uint32_t i = 0; i < count; i++) {
        char *lang = get_language_str(languages, sb[i].language_id);
        unsigned long total_len = base_len + strlen(lang) + 1 + 1;

        path = (char *)malloc(total_len);
        if (path == NULL) {
          fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
          goto clean;
        }

        snprintf(path, total_len, "%s/%s", base, lang);

        if (read_soundbank(sb[i].id, sb[i].block_size * sb[i].file_size,
                           sb[i].start_block, path) < 0) {
          free(path);
          goto clean;
        }
        free(path);
      }
    }
    header_data_ptr =
        (void *)((uintptr_t)header_data_ptr + header.stm_lut_size);
  }

  // Read EXTERNALS
  {
    uint32_t count = *((uint32_t *)header_data_ptr);

    if (count > 0) {
      soundbank_entry64_t *sb =
          (soundbank_entry64_t *)((uintptr_t)header_data_ptr + sizeof(count));

      for (uint32_t i = 0; i < count; i++) {
        char *lang = get_language_str(languages, sb[i].language_id);
        unsigned long total_len = base_len + strlen(lang) + 1 + 1;

        path = (char *)malloc(total_len);
        if (path == NULL) {
          fprintf(stderr, "Could not allocate memory: %s\n", strerror(errno));
          goto clean;
        }

        snprintf(path, total_len, "%s/%s", base, lang);

        if (read_soundbank(sb[i].id, sb[i].block_size * sb[i].file_size,
                           sb[i].start_block, path) < 0) {
          free(path);
          goto clean;
        }
        free(path);
      }
    }
  }

clean:
  if (languages) {
    lang_t *lang = languages;
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

char *get_language_str(lang_list_t languages, uint32_t id) {
  static char *none = "";
  char *ret = none;

  lang_list_t p = languages;
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
  filebasename[len + 1] = '\0';

  return filebasename;
}

char *read16to8(char16_t *str16) {
  char *char8str;
  unsigned long len = 0;
  uint16_t letter;

  while (str16[len] != 0) {
    len++;
  }

  char8str = (char *)malloc(len + 1);
  if (char8str == NULL) {
    return NULL;
  }

  for (unsigned long i = 0; i <= len; i++) {
    letter = (uint16_t)str16[i];
    char8str[i] = (char)letter;
  }
  // char8str[len + 1] = '\0';

  return char8str;
}

// # vim: ts=2 sw=2 expandtab
