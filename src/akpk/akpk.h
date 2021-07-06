/*
Wwise AKPK File Structure

magic 'AKPK'  4 byte
header_size   4 byte - total size of header
version       4 byte - should be 1
languageMapSize 4 byte
soundBanksSize  4 byte
stmFilesSize    4 byte
externalsSize   4 byte


-- language map structure
count of languages              4 byte
string_entry[0]...string_entry[count-1]
  struct {
    .offset                     4 byte
    .id                         4 byte
  }
string of languages + (optional)padding

* read characters by 0 from languageMap start address + entry[i].offset
* don't forget about padding % 4 while construct new languageMap


-- soundBanks structure
count of files      4 byte
file_entry[0] ... file_entry[count - 1]
  struct {
    .file_id        4 byte
    .block_size     4 byte - usualy is 1
    .file_size      4 byte
    .start_block    4 byte
    .language_id    4 byte
  }


-- statements structure
!! same as soundbanks


-- externals sctructure
!! same as soundbanks,
!! but .file_id has 8 bytes

-- DATA
use .start_block from sb, stm, externals
//Wwise Encoded Media
*/

#ifndef AKPK_H
#define AKPK_H

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define AKPK_BKHD "BKHD"
#define AKPK_DATA "DATA"
#define AKPK_RIFF "RIFF"

#define AKPK_MAGIC                                                             \
  ((uint32_t)'A' | ((uint32_t)'K' << 8) | ((uint32_t)'P' << 16) |              \
   ((uint32_t)'K' << 24))

struct akpk_file_header_t {
  uint32_t magic;
  uint32_t header_size;
  uint32_t version;
  uint32_t language_map_size;
  uint32_t sound_banks_lut_size;
  uint32_t stm_files_lut_size;
  uint32_t externals_lut_size;
};

struct file32_entry_t {
  uint32_t id;
  uint32_t block_size;
  uint32_t file_size;
  uint32_t start_block;
  uint32_t language_id;
};

struct file64_entry_t {
  uint64_t id;
  uint32_t block_size;
  uint32_t file_size;
  uint32_t start_block;
  uint32_t language_id;
} ;

struct string_entry_t {
  uint32_t offset;
  uint32_t id;
} ;

struct lang_info_t {
  uint32_t id;
  char* name;
} ;

struct lang_map_t {
  uint32_t count;
  struct lang_info_t* languages;
} ;

struct soundbank_t {
  uint32_t count;
  struct file32_entry_t* files;
} ;

struct soundbank64_t {
  uint32_t count;
  struct file64_entry_t** files;
} ;

struct akpk_data_index_t {
  char DIDX[4];
  uint32_t size;
  uint32_t hash;
  uint32_t data_offset;
  uint32_t data_size;
  uint32_t _unknown1;
  uint64_t _unknown2;
};

struct akpk_didx_data_info_t {
  char DATA[4];
  uint32_t size;
};

struct AKPK {
  struct akpk_file_header_t header;
  struct lang_map_t lang_map;
  struct soundbank_t soundbanks;
  struct soundbank_t smt;
  struct soundbank64_t externals;
  char * _filename;
};

struct AKPK* akpk_open(const char*);
void akpk_close(struct AKPK*);
void akpk_extract(struct AKPK*);
uint8_t *akpk_get_file(struct AKPK* akpk, struct file32_entry_t* entry);

#endif // AKPK_H
