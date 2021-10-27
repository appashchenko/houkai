#include <stddef.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE
#include "akpk.h"
#include "bkhd.h"
#include "didx.h"
#include "hirc.h"
#include "riff.h"
#include "sys/sysinfo.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <semaphore.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <threads.h>
#include <unistd.h>
#include <limits.h>

#define VERBOSE 1

char extr_path[256] = {0};

// static sem_t limiter;
static lang_list_t languages = NULL;
static int stream_fd = 0;

static char *get_language_str(uint32_t id);
static int read_soundbank(uint64_t id, size_t sz, uint32_t len, uint32_t off);
static void save_wem(void *data, size_t size, uint64_t id);
static char* basename(const char* filepath, char* path);

void akpk_open(const char *filepath) {
  struct stat sb;
  void *header_data = NULL;
  void *header_data_ptr = NULL;
  akpk_header_t header;
  uint8_t *filedata = NULL;
  char base[PATH_MAX];

  if ((stream_fd = open(filepath, O_RDONLY, O_NOFOLLOW)) == 0) {
    fprintf(stderr, "Failed to open file %s. Exit\n", filepath);
    goto clean;
  }

  if (fstat(stream_fd, &sb) == -1) {
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

  basename(filepath, base);
  int ret = mkdir(base,
                  S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH | S_IROTH);
  if (ret != 0 && errno != EEXIST) {
    char *error = strerror(ret);
    fprintf(stderr, "Failed to create directory %s\n%s\n", extr_path, error);
    goto clean;
  }

  // Read remaining header data (languages map, soundbanks look-up tables)
  {
    uint32_t size = header.lang_map_size + header.sb_lut_size +
                    header.stm_lut_size + header.ext_lut_size;
    header_data = malloc(size);
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
      lang_entry_t *lang_map =
          (lang_entry_t *)((uintptr_t)header_data_ptr + sizeof(count));

      for (uint32_t i = 0; i < count; i++) {
        languages[i].id = lang_map[i].id;
        languages[i].name = (char *)header_data_ptr + lang_map[i].offset;
#ifdef VERBOSE
        printf("* LANGUAGE: %u,%s\n", languages[i].id, languages[i].name);
#endif
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
        if (read_soundbank(sb[i].id, sb[i].block_size * sb[i].file_size,
                           sb[i].start_block, sb[i].language_id) < 0) {
          goto clean;
        }
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
        if (read_soundbank(sb[i].id, sb[i].block_size * sb[i].file_size,
                           sb[i].start_block, sb[i].language_id) < 0) {
          goto clean;
        }
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
        if (read_soundbank(sb[i].id, sb[i].block_size * sb[i].file_size,
                           sb[i].start_block, sb[i].language_id) < 0) {
          goto clean;
        }
      }
    }
  }

clean:
  if (filedata) {
    if (munmap(filedata, (size_t)sb.st_size) == -1) {
      perror("Failed to unmap file");
    }
    free(filedata);
  }

  if (header_data) {
    free(header_data);
  }
  if (languages != NULL) {
    free(languages);
  }
  if (stream_fd) {
    close(stream_fd);
  }
}

static int read_soundbank(uint64_t id, size_t size, uint32_t offset,
                          uint32_t lang_id) {
  void *data = malloc(size);

  char path[PATH_MAX];

  snprintf(path, PATH_MAX, "%s/%s", base, get_language_str(lang_id));

  if (pread(stream_fd, data, size, offset) == (ssize_t)size) {
    uint32_t magic = *((uint32_t *)data);
    switch (magic) {
    case BKHD:
      read_bkhd(data path);
      break;
    case RIFF: {
      save_wem(data, size, id, path);
    } break;
    default:
      fprintf(stderr, "Unknown soundbank container type: %X\n", magic);
    }
  } else {
    fprintf(stderr, "Failed to read file. Exit.\n");
  }
  free(data);

  return 0;
}


void save_wem(void *data, size_t size, uint64_t id) {
  char filename[21] = {0};
  int out;

  snprintf(filename, 20, "%lX.wem", id);

  out = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

  if (out) {
    uint64_t ret = write(out, data, size);

    if (ret != size) {
      char *err_str = strerror(errno);
      printf("Failed to save %s: %s\n", filename, err_str);
    }
    close(out);
  }
}

char* get_language_str(uint32_t id) {
  static char* none = "";
  char* ret = none;

  lang_list_t list = languages;
  while (list->name != NULL) {
    if (list->id == id) {
      ret = list->name;
      break;
    }
    list++;
  }

  return ret;
}


char * basename(const char* filepath, char* path) {
  char * filebasename;
  char * p = (char*)filepath;
  char * filename = (char*)filepath;
  size_t len = 0;

  if (path == NULL) {
    filebasename = (char*)malloc(len + 1);
  } else {
    filebasename = path;
  }

  while(*p != '\0') {
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
  }

  memcpy(filename, &filebasename, len);
  filebasename[len + 1] = '\0';

  return filebasename;
}

// # vim: ts=2 sw=2 expandtab
