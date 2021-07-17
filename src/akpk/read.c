#define _GNU_SOURCE
#include "akpk.h"
#include "bkhd.h"
#include "didx.h"
#include "hirc.h"
#include <assert.h>
#include <errno.h>
#include <memory.h>
#include <sndfile.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <threads.h>
#include <unistd.h>

static lang_list_t languages = NULL;
static char sbdir[128] = {0};
static char cur_path[128] = {0};

static char* get_language_str(uint32_t id);
static void read_bkhd(void* data);
static void read_didx(void* data);
static void read_hirc(void* data);

void akpk_open(const char* filename) {
  void* header_data = NULL;
  void* header_data_ptr = NULL;
  FILE* stream = NULL;
  akpk_header_t header;

  if ((stream = fopen(filename, "rb")) == 0) {
    fprintf(stderr, "Failed to open file %s. Exit\n", filename);
    return;
  }

  if (fread(&header, sizeof(header), 1, stream) != 1) {
    fprintf(stderr, "Failed to read file header. Exit\n");
    fclose(stream);
    return;
  }

  if (header.magic != AKPK) {
    fprintf(stderr, "File is not a valid AKPK archive. Exit\n");
    fclose(stream);
    return;
  }

  if (header.version != 1) {
    fprintf(stderr, "Supported version 1 but file version is %u. Exit\n",
            header.version);
    fclose(stream);
    return;
  }

  // Read remaining header data (languages map, soundbanks look-up tables)
  {
    //    uint32_t rem = header.size - (uint32_t)sizeof(header);
    uint32_t size = header.language_map_size + header.soundbanks_lut_size +
                    header.stm_lut_size + header.externals_lut_size;
    header_data = malloc(size);
    header_data_ptr = header_data;

    if (fread(header_data, size, 1, stream) != 1) {
      free(header_data);
      fclose(stream);
      return;
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

        // printf("* LANGUAGE: %u,%s\n", lang_map[i].id, title);
      }
      languages[count].id = 0;
      languages[count].name = NULL;
    }

    header_data_ptr =
        (void*)((uintptr_t)header_data_ptr + header.language_map_size);
  }

  // All seems OK, unpacking
  strcat(sbdir, basename(filename));
  strcat(sbdir, ".extracted");

  int ret =
      mkdir(sbdir, S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH | S_IROTH);
  if (ret != 0 && errno != EEXIST) {
    char* error = strerror(ret);
    fprintf(stderr, "Failed to create directory %s: %s\n", sbdir, error);
    free(header_data);
    fclose(stream);
    return;
  }

  // Read soundbanks
  {
    uint32_t count = *((uint32_t*)header_data_ptr);

    if (count > 0) {
      soundbank_entry32_t* sb =
          (soundbank_entry32_t*)((uintptr_t)header_data_ptr + sizeof(count));

      for (uint32_t i = 0; i < count; i++) {
        // printf("* SOUNDBANK#%u size=%u language=%u \n", sb[i].id,
        // sb[i].block_size*sb[i].file_size, sb[i].language_id);

        if (snprintf(cur_path, 128, "%s/%s", sbdir,
                     get_language_str(sb->language_id)) < 0) {
          free(header_data);
          fclose(stream);
          return;
        }

        mkdir(cur_path,
              S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH | S_IROTH);

        size_t data_size = sb[i].block_size * sb[i].file_size;
        void* data = malloc(data_size);
        fseek(stream, sb[i].start_block, SEEK_SET);

        if (fread(data, data_size, 1, stream) == 1) {
          uint32_t magic = *((uint32_t*)data);
          switch (magic) {
            case BKHD:
              read_bkhd(data);
              break;
            case RIFF:
              save_riff(data);
              printf("RIFF detected. What??\n");
              break;
            default:
              fprintf(stderr, "Unknown soundbank container type: %X\n", magic);
              return;
          }
        } else {
          fprintf(stderr, "Failed to read file. Exit.\n");
        }
        free(data);
      }
    }
    header_data_ptr =
        (void*)((uintptr_t)header_data_ptr + header.soundbanks_lut_size);
  }

  // Read STM soundbanks
  {
    uint32_t count = *((uint32_t*)header_data_ptr);

    if (count > 0) {
      soundbank_entry32_t* sb =
          (soundbank_entry32_t*)((uintptr_t)header_data_ptr + sizeof(count));

      for (uint32_t i = 0; i < count; i++) {
        //        printf("* STM SOUNDBANK#%u size=%u language=%u \n", sb[i].id,
        //               sb[i].block_size * sb[i].file_size, sb[i].language_id);

        if (snprintf(cur_path, 128, "%s/%s", sbdir,
                     get_language_str(sb->language_id)) < 0) {
          free(header_data);
          fclose(stream);
          return;
        }

        mkdir(cur_path,
              S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH | S_IROTH);

        size_t data_size = sb[i].block_size * sb[i].file_size;
        void* data = malloc(data_size);
        fseek(stream, sb[i].start_block, SEEK_SET);

        if (fread(data, data_size, 1, stream) == 1) {
          uint32_t magic = *((uint32_t*)data);
          switch (magic) {
            case BKHD:
              read_bkhd(data);
              break;
            case RIFF:
              printf("RIFF detected. What??\n");
              break;
            default:
              fprintf(stderr, "Unknown soundbank container type: %X\n", magic);
              return;
          }
        } else {
          fprintf(stderr, "Failed to read file. Exit.\n");
        }
        free(data);
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
        //        printf("* EXTERNALS#%lu size=%u language=%u \n", sb[i].id,
        //               sb[i].block_size * sb[i].file_size, sb[i].language_id);

        if (snprintf(cur_path, 128, "%s/%s", sbdir,
                     get_language_str(sb->language_id)) < 0) {
          free(header_data);
          fclose(stream);
          return;
        }

        mkdir(cur_path,
              S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IXOTH | S_IROTH);

        size_t data_size = sb[i].block_size * sb[i].file_size;
        void* data = malloc(data_size);
        fseek(stream, sb[i].start_block, SEEK_SET);

        if (fread(data, data_size, 1, stream) == 1) {
          uint32_t magic = *((uint32_t*)data);
          switch (magic) {
            case BKHD:
              read_bkhd(data);
              break;
            case RIFF:
              printf("RIFF detected. What??\n");
              break;
            default:
              fprintf(stderr, "Unknown soundbank container type: %X\n", magic);
              return;
          }
        } else {
          fprintf(stderr, "Failed to read file. Exit.\n");
        }
        free(data);
      }
    }
  }

  if (header_data) free(header_data);

  if (languages != NULL) {
    free(languages);
  }

  if (stream) {
    fclose(stream);
  }
}

static void read_didx(void* data) {
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

    //    printf("*   WEM#%X (%u bytes)\n", entry->wem_id, entry->size);
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
  printf("HIRC is not supported yet.\n");
  return;

  hirc_header_t* header = (hirc_header_t*)data;

  char filename[128];
  snprintf(filename, 128, "%uOF%u.hirc", header->size ,header->count);
  FILE* out = fopen(filename, "wb");
  fwrite(data, header->size, 1, out);
  fclose(out);
  return;

  data = (void*)((uintptr_t)data + sizeof(hirc_header_t));

  for (uint32_t i = 0; i < header->count; i++) {
    hirc_object_info_t* object = (hirc_object_info_t*)data;

    printf("HIRC object type=%u\n", object->type);
    data = (void*)((uintptr_t)data + object->size);
  }
}

static void read_bkhd(void* data) {
  bkhd_header_t* header = (bkhd_header_t*)data;

  data = (void*)((uintptr_t)data + sizeof(bkhd_header_t));

  //  bkhd_info_t* info = (bkhd_info_t*)data;
  //  printf("*  DIDX#%u from %u\n", info.bank_id, info.soundbank_id);

  data = (void*)((uintptr_t)data + header->size);
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
