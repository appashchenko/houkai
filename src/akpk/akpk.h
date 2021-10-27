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

#define E_AVATAR_FOOTSTEP_LEFT 78C82F4B
#define E_AVATAR_FOOTSTEP_RIGHT 69413D48
#define E_AVATAR_MEI_SKILL_ULTRA_HIT CBD811D
#define E_AVATAR_MEI_SKILL_ULTRA_JUMP 82D77E1
#define E_AVATAR_MEI_SKILL_ULTRA_LANDING 3050420065U
#define E_AVATAR_SKILLULTRA_START 2177178959U
#define E_AVATAR_SKILLULTRA_STOP 4077935573U
#define E_AVATAR_SWD_WSH_HIT 1636107040U
#define E_AVATAR_TIMESKILL_BOOMOFF 2940506964U
#define E_BGM_GAMEENTRY 3546849366U
#define E_BGM_MAINMENU 364086542U
#define E_BGM_SCENEENTER 1816222402U
#define E_UI_CLICK 2249769530U
#define E_UI_SELECT_GAMEENTRY 1669353005U

#define DLG_GROUP1 1217192515U
#define DLG_GROUP1_S1 1584861493U
#define DLG_GROUP1_S2 1584861494U
#define DLG_GROUP1_S3 1584861495U

#define DLG_GROUP2 2026385227U
#define DLG_GROUP2_T1 1568083776U
#define DLG_GROUP2_T2 1568083779U
#define DLG_GROUP2_T3 1568083778U

enum SECTION {

  AKPK = 0x4B504B41,
  BKHD = 0x44484B42,
  HIRC = 0x43524948,
  DIDX = 0x58444944,
  DATA = 0x41544144,
  RIFF = 0x46464952
};

struct akpk_header_t {
  uint32_t magic;
  uint32_t size;
  uint32_t version;
  uint32_t lang_map_size;
  uint32_t sb_lut_size;
  uint32_t stm_lut_size;
  uint32_t ext_lut_size;
};
typedef struct akpk_header_t akpk_header_t;

struct soundbank_entry32_t {
  uint32_t id;
  uint32_t block_size;
  uint32_t file_size;
  uint32_t start_block;
  uint32_t language_id;
};
typedef struct soundbank_entry32_t soundbank_entry32_t;

struct soundbank_entry64_t {
  uint64_t id;
  uint32_t block_size;
  uint32_t file_size;
  uint32_t start_block;
  uint32_t language_id;
};
typedef struct soundbank_entry64_t soundbank_entry64_t;

struct lang_entry_t {
  uint32_t offset;
  uint32_t id;
};
typedef struct lang_entry_t lang_entry_t;

struct lang_t {
  uint32_t id;
  char *name;
} __attribute__((packed));
typedef struct lang_t lang_t;
typedef lang_t *lang_list_t;

void akpk_open(const char *);
#endif

// # vim: ts=2 sw=2 expandtab
