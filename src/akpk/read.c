#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
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
#include <sndfile.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <threads.h>
#include <unistd.h>


// static sem_t limiter;
static lang_list_t languages = NULL;
static char cur_path[128] = {0};
static int stream_fd = 0;

static char* get_language_str(uint32_t id);
static void read_bkhd(void* data);
static void read_didx(void* data);
static void read_hirc(void* data);
static int read_soundbank(uint64_t id, size_t sz, uint32_t len, uint32_t off);

void akpk_open(const char* filename) {
  struct stat sb;
  void* header_data = NULL;
  void* header_data_ptr = NULL;
  akpk_header_t header;
  // const int cpu_count = get_nprocs();
  uint8_t* filedata = NULL;

  if ((stream_fd = open(filename, O_RDONLY, O_NOFOLLOW)) == 0) {
    fprintf(stderr, "Failed to open file %s. Exit\n", filename);
    goto clean;
  }

  if (fstat(stream_fd, &sb) == -1) {
    perror("Failed to get file info");
    goto clean;
  }

  printf("File size %lu bytes.\n", sb.st_size);

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
    uint32_t count = *((uint32_t*)header_data_ptr);

    if (count > 0) {
      languages = (lang_list_t)calloc(count + 1, sizeof(lang_t));
      lang_entry_t* lang_map =
          (lang_entry_t*)((uintptr_t)header_data_ptr + sizeof(count));

      for (uint32_t i = 0; i < count; i++) {
        languages[i].id = lang_map[i].id;
        languages[i].name = (char*)header_data_ptr + lang_map[i].offset;
        /* printf("* LANGUAGE: %u,%s\n", languages[i].id, languages[i].name);*/
      }
      languages[count].id = 0;
      languages[count].name = NULL;
    }

    header_data_ptr =
        (void*)((uintptr_t)header_data_ptr + header.lang_map_size);
  }

  // All seems OK, unpacking
  //  strcat(sbdir, basename(filename));
  //  strcat(sbdir, ".extracted");

  //  int ret =
  //      mkdir(sbdir, S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH |
  //      S_IROTH);
  //  if (ret != 0 && errno != EEXIST) {
  //    char* error = strerror(ret);
  //    fprintf(stderr, "Failed to create directory %s\n%s\n", sbdir, error);
  //    goto clean;
  //  }

  // Read soundbanks
  {
    uint32_t count = *((uint32_t*)header_data_ptr);

    if (count > 0) {
      soundbank_entry32_t* sb =
          (soundbank_entry32_t*)((uintptr_t)header_data_ptr + sizeof(count));

      /*if (count >= (uint32_t)cpu_count) {
        sem_init(&limiter, 0, (unsigned int)cpu_count);
        for (uint32_t i = 0; i < count; i++) {
          thrd_t thread;
          thrd_create(&thread, start_soundbank32, (void*)&sb[i]);
          sem_wait(&limiter);
        }
        sem_close(&limiter);
      }*/

      for (uint32_t i = 0; i < count; i++) {
        /* printf("* SB32 SOUNDBANK#%u size=%lu language=%u \n", sb[i].id,
               (uint64_t)sb[i].block_size * (uint64_t)sb[i].file_size,
               sb[i].language_id);*/
        if (read_soundbank(sb[i].id, sb[i].block_size * sb[i].file_size,
                           sb[i].start_block, sb[i].language_id) < 0) {
          goto clean;
        }
      }
    }

    header_data_ptr = (void*)((uintptr_t)header_data_ptr + header.sb_lut_size);
  }

  // Read STM soundbanks
  {
    uint32_t count = *((uint32_t*)header_data_ptr);

    if (count > 0) {
      soundbank_entry32_t* sb =
          (soundbank_entry32_t*)((uintptr_t)header_data_ptr + sizeof(count));

      for (uint32_t i = 0; i < count; i++) {
        /* printf("* STM SOUNDBANK#%u size=%lu language=%u \n", sb[i].id,
               (uint64_t)sb[i].block_size * (uint64_t)sb[i].file_size,
               sb[i].language_id);*/

        if (read_soundbank(sb[i].id, sb[i].block_size * sb[i].file_size,
                           sb[i].start_block, sb[i].language_id) < 0) {
          goto clean;
        }
      }
    }
    header_data_ptr = (void*)((uintptr_t)header_data_ptr + header.stm_lut_size);
  }

  // Read EXTERNALS
  {
    uint32_t count = *((uint32_t*)header_data_ptr);

    if (count > 0) {
      soundbank_entry64_t* sb =
          (soundbank_entry64_t*)((uintptr_t)header_data_ptr + sizeof(count));

      for (uint32_t i = 0; i < count; i++) {
        /* printf("* EXTERNALS#%lu size=%u language=%u \n", sb[i].id,
               sb[i].block_size * sb[i].file_size, sb[i].language_id);*/

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
  void* data = malloc(size);

  if (pread(stream_fd, data, size, offset) == (ssize_t)size) {
    uint32_t magic = *((uint32_t*)data);
    switch (magic) {
      case BKHD:
        // printf("%s\n", "*** BKHD");
        read_bkhd(data);
        break;
      case RIFF: {
        // printf("%s\n", "***  RIFF");
        break;

        /*riff_header_t* rh = (riff_header_t*)data;

        if (snprintf(out_name, 127, "%s/%lu.wem", cur_path, id) > 0) {
          out = fopen(out_name, "wb");

          if (out) {
            printf("Save raw RIFF %u bytes to %s\n", rh->size, out_name);
            size_t ret = fwrite(data, rh->size, 1, out);
            if (ret != 1) {
              perror("Failed to write RIFF to file");
            }
            fclose(out);
          } else {
            perror("Failed to open file to save raw RIFF");
          }
        }*/
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

static void read_didx(void* data) {
  return;

  FILE* out = NULL;
  char filename[256] = {0};
  struct stat info;

  didx_header_t* header = (didx_header_t*)data;

  didx_entry_t* entry =
      (didx_entry_t*)((uintptr_t)data + sizeof(didx_header_t));

  didx_entry_t* data_header =
      (didx_entry_t*)((uintptr_t)entry + header->enties_size);
  void* data_start = (void*)((uintptr_t)data_header + sizeof(didx_data_t));

  for (; entry < data_header; entry++) {
    int ret = stat(filename, &info);

    if (ret == 0) continue;

    // printf("*   WEM#%X (%u bytes)\n", entry->wem_id, entry->size);
    if (sprintf(filename, "%s/%X.wem", cur_path, entry->wem_id) < 0) {
      return;
    }

    out = fopen(filename, "wb");

    if (out) {
      void* wem = (void*)((uintptr_t)data_start + entry->data_offset);
      fwrite(wem, entry->size, 1, out);
      fclose(out);
    } else {
      perror("Failed to write WEM to file");
      continue;
    }
  }
}

static void read_hirc(void* data) {
  hirc_header_t* header = (hirc_header_t*)data;

  data = (void*)((uintptr_t)data + sizeof(hirc_header_t));

  for (uint32_t i = 0; i < header->count; i++) {
    hirc_object_t* object = (hirc_object_t*)data;

    printf("HIRC size %u bytes.\n", object->size);

    switch (object->type) {
      case HIRC_SOUND: {
        hirc_obj_snd* sound = (hirc_obj_snd*)data;

        int out = open("sound.hirc", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
        if (out) {
          write(out, sound, sound->size);
          close(out);
          return;
        } else {
          perror("Failed to write HIRC");
          return;
        }

        printf(">>>HIRC#%X %u bytes\n", sound->sfx_id, sound->size);
      } break;
      case HIRC_MUSIC_TRACK: {
        hirc_obj_mt* track = (hirc_obj_mt*)data;

        int out = open("track.hirc", O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR);
        if (out) {
          write(out, track, track->size);
          close(out);
          return;
        } else {
          perror("Failed to write HIRC");
          return;
        }
        printf("HIRC MUSIC TRACK #%u\n", track->id);
      } break;
      default:
        fprintf(stderr, "HIRC object type %u is not supported yet. Skip\n",
                object->type);
        break;
    }

    printf("HIRC object type=%u\n", object->type);
    data = (void*)((uintptr_t)data + object->size + sizeof(hirc_object_t));
  }
}

static void read_bkhd(void* data) {
  bank_header_t* header = (bank_header_t*)data;

  printf("*  DIDX#%u from %u\n", header->bank_id, header->soundbank_id);
  data = (void*)((uintptr_t)data + sizeof(header->magic) +
                 sizeof(header->size) + header->size);

  uint32_t section_magic = *((uint32_t*)data);

  switch (section_magic) {
    case DIDX:
      read_didx(data);
      break;
    case HIRC:
      read_hirc(data);
      break;
    default:
      fprintf(stderr, "!!!! Unknown BKHD section %u\n", section_magic);
  }
}

/*char* get_language_str(uint32_t id) {
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
}*/

// # vim: ts=2 sw=2 expandtab
