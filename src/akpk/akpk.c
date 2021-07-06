#include "akpk.h"

#include <assert.h>
#include <memory.h>
#include <sndfile.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct AKPK* akpk_open(const char* filename) {
  struct AKPK* akpk = (struct AKPK*)malloc(sizeof(struct AKPK));
  void* header_data = NULL;
  void* section_start_p = NULL;
  FILE* stream = fopen(filename, "rb");

  memset(akpk, 0, sizeof(struct AKPK));

  if (stream == 0) {
    free(akpk);
    return NULL;
  }

  akpk->_filename = filename;

  if (fread(&akpk->header, sizeof(akpk->header), 1, stream) != 1) {
    fprintf(stderr, "Failed to read file header. Exit\n");
    fclose(stream);
    free(akpk);
    return NULL;
  }

  if (akpk->header.magic != AKPK_MAGIC || akpk->header.version != 1) {
    fprintf(stderr, "File is not a valid AKPK archive. Exit\n");
    fclose(stream);
    free(akpk);
    return NULL;
  }

  uint32_t rem = (uint32_t)((int32_t)akpk->header.header_size -
                            (int32_t)sizeof(akpk->header));
  header_data = malloc(rem);
  section_start_p = header_data;

  if (fread(header_data, rem, 1, stream) != 1) {
    free(header_data);
    fclose(stream);
    free(akpk);
    return NULL;
  }

  if (akpk->header.language_map_size > 4) {
    uint32_t count = *((uint32_t*)section_start_p);
    struct string_entry_t* languages =
        (struct string_entry_t*)((uint32_t*)section_start_p + 1);

    akpk->lang_map.count = count;
    akpk->lang_map.languages =
        (struct lang_info_t*)calloc(count, sizeof(struct lang_info_t));

    for (uint32_t i = 0; i < count; i++) {
      uint8_t* c = (uint8_t*)section_start_p + languages[i].offset;

      uint64_t len = (uint32_t)strlen((char*)c);

      akpk->lang_map.languages[i].id = languages[i].id;
      akpk->lang_map.languages[i].name = malloc(len + 1);
      memcpy(akpk->lang_map.languages[i].name, (char*)c, len);
    }
  }

  section_start_p =
      (void*)((uint8_t*)section_start_p + akpk->header.language_map_size);

  if (akpk->header.sound_banks_lut_size > 4) {
    uint32_t count = *((uint32_t*)section_start_p);
    struct file32_entry_t* sb_files =
        (struct file32_entry_t*)((uint32_t*)section_start_p + 1);

    akpk->soundbanks.count = count;
    akpk->soundbanks.files =
        (struct file32_entry_t*)calloc(count, sizeof(struct file32_entry_t));

    for (uint32_t i = 0; i < count; i++) {
      memcpy(&akpk->soundbanks.files[i], &sb_files[i],
             sizeof(struct file32_entry_t));
    }
  }

  section_start_p =
      (void*)((uint8_t*)section_start_p + akpk->header.sound_banks_lut_size);

  if (akpk->header.stm_files_lut_size > 4) {
    uint32_t count = *((uint32_t*)section_start_p);
    struct file32_entry_t* stm_files =
        (struct file32_entry_t*)((uint32_t*)section_start_p + 1);

    akpk->smt.count = count;
    akpk->smt.files = calloc(count, sizeof(struct file32_entry_t));

    for (uint32_t i = 0; i < count; i++) {
      memcpy(&akpk->smt.files[i], &stm_files[i], sizeof(struct file32_entry_t));
    }
  }

  section_start_p =
      (void*)((uint8_t*)section_start_p + akpk->header.stm_files_lut_size);

  if (akpk->header.externals_lut_size > 4) {
    uint32_t count = *((uint32_t*)section_start_p);
    struct file64_entry_t* ext_files =
        (struct file64_entry_t*)((uint32_t*)section_start_p + 1);
    akpk->externals.count = count;
    akpk->externals.files = calloc(count, sizeof(struct file64_entry_t));

    for (uint32_t i = 0; i < count; i++) {
      memcpy(&akpk->externals.files[i], &ext_files[i],
             sizeof(struct file64_entry_t));
    }
  }

  fclose(stream);
  free(header_data);

  return akpk;
}

void akpk_close(struct AKPK* bank) {
  if (bank) {
    if (bank->lang_map.count > 0) {
      for (uint32_t i = 0; i < bank->lang_map.count; i++) {
        free(bank->lang_map.languages[i].name);
      }
      free(bank->lang_map.languages);
    }

    if (bank->soundbanks.count > 0) {
      free(bank->soundbanks.files);
    }

    if (bank->smt.count > 0) {
      free(bank->smt.files);
    }

    if (bank->externals.count > 0) {
      free(bank->externals.files);
    }

    free(bank);
  }
}

void akpk_extract(struct AKPK* akpk) {
  FILE* stream = fopen(akpk->_filename, "rb");

  if (akpk->soundbanks.count > 0) {
    char full_path[256];
    char filename[32];

    struct file32_entry_t* files = akpk->soundbanks.files;

    sprintf(full_path, "%s_", akpk->_filename);
    mkdir(full_path, 0755);

    for (uint32_t n = 0; n < akpk->soundbanks.count; n++) {
      if (files[n].language_id < akpk->lang_map.count) {
        sprintf(full_path, "%s_/%s/", akpk->_filename,
                akpk->lang_map.languages[files[n].language_id].name);
      } else {
        sprintf(full_path, "%s_/", akpk->_filename);
      }

      mkdir(full_path, 0755);

      sprintf(filename, "%u.wem", files[n].id);
      strcat(full_path, filename);

      if (fseek(stream, files[n].start_block, SEEK_SET) != 0) {
        perror("Fatal error: ");
        break;
      }

      size_t length = files[n].file_size * files[n].block_size;
      void* file = malloc(length);

      if (fread(file, length, 1, stream) != 1) {
        perror("Failed to read content: ");
        free(file);
        break;
      }

      FILE* out = fopen(full_path, "wb");
      if (out == NULL) {
        perror("Failed to open file to write: ");
        free(file);
        break;
      }

      printf("Extractin %s ...\n", full_path);

      if (fwrite(file, length, 1, out) != 1) {
        perror("Failed to write file: ");
        free(file);
        fclose(out);
        break;
      }

      fclose(out);
      free(file);
    }
  }

  fclose(stream);
}

