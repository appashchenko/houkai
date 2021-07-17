#include "akpk.h"
#include <assert.h>
#include <memory.h>
#include <sndfile.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

akpk_t* akpk_create(void) {
  akpk_t* archive = (akpk_t*)malloc(sizeof(akpk_t));
  archive->languages = NULL;

  return archive;
}

uint32_t akpk_add_language(akpk_t* archive, char* language, uint32_t id) {
  uint32_t lang_count = 0;

  if (archive->languages == NULL) {
    archive->languages = (lang_list_t)calloc(2, sizeof(lang_t));
    archive->languages[0].name = NULL;
  }

  while (archive->languages->name != NULL) {
    lang_count++;
  }

  archive->languages =
      realloc(archive->languages, sizeof(lang_t) * (lang_count + 1));
  archive->languages[lang_count].id = id;
  archive->languages[lang_count].name = strdup(language);
  archive->languages[lang_count + 1].id = 0;
  archive->languages[lang_count + 1].name = NULL;

  return lang_count;
}

int akpk_save_to_file(akpk_t* archive, const char* filename) {
  FILE* out = NULL;

  if ((out = fopen(filename, "wb")) == NULL) {
    return -1;
  }

  uint32_t lang_count = 0;
  while (archive->languages[lang_count].id > 0) {
    lang_count++;
  }

  akpk_header_t header = {.magic = AKPK,
                          .size = 0,
                          .version = 1,
                          .language_map_size = 0,
                          .soundbanks_lut_size = sizeof(uint32_t),
                          .stm_lut_size = 4,
                          .externals_lut_size = 4};

  fwrite(&header, sizeof(header), 1, out);
  fclose(out);

  return 0;
}
