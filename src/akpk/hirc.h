#include "akpk.h"
#include <stdint.h>

#ifndef HIRC_H
#define HIRC_H

struct __attribute__((packed)) hirc_header_t {
  uint32_t magic;
  uint32_t size;
  uint32_t count;
};
typedef struct hirc_header_t hirc_header_t;

struct __attribute__((packed)) hirc_object_t {
  uint8_t type;
  uint32_t size;
};

enum include_type { INCLUDED = 0, STREAMED = 1, PREFETCH = 2 };

struct __attribute__((packed)) hirc_obj_act {
  uint8_t type;
  uint32_t size;
  uint32_t offset;
  uint32_t id;
  uint8_t scope;
  uint8_t action_type;
  uint8_t unk1;
};

struct __attribute__((packed)) hirc_obj_snd {
  uint8_t type;
  uint32_t size;
  uint32_t sfx_id;
  uint32_t group_id;
  uint32_t include;
  uint32_t audio_id;
  uint32_t source_id;
  // uint32_t offset;
  // uint32_t size;
};

struct __attribute__((packed)) hirc_obj_mt {
  uint8_t type;
  uint32_t size;
  uint32_t id;
  uint32_t unk1;
  uint32_t unk2;
  uint8_t streamed;
  uint32_t audio_id;
  uint32_t raudio_id;
};

struct __attribute__((packed)) hirc_obj_mfx {
  uint8_t type;
  uint32_t size;
  uint32_t id;
  uint32_t group1;
  uint32_t group2;
  uint32_t group3;
};

typedef struct hirc_object_t hirc_object_t;
typedef struct hirc_obj_mt hirc_obj_mt;
typedef struct hirc_obj_snd hirc_obj_snd;
typedef struct hirc_obj_mfx hirc_obj_mfx;

enum HIRC_OBJECT {
  HIRC_SETTINGS = 1,
  HIRC_SOUND = 2,
  HIRC_EVENT_ACTION = 3,
  HIRC_ACTION = 4,
  HIRC_SEQUENCE_CONTAINER = 5,
  HIRC_SWITCH_CONTAINER = 6,
  HIRC_ACTOR_MIXER = 7,
  HIRC_AUDIO_BUS = 8,
  HIRC_BLEND_CONTAINER = 9,
  HIRC_MUSIC_SEGMENT = 10,
  HIRC_MUSIC_TRACK = 11,
  HIRC_MUSIC_SWITCH_CONTAINER = 12,
  HIRC_MUSIC_PLAYLIST_CONTAINER = 13,
  HIRC_ATTENUATION = 14,
  HIRC_DIALOGUE_EVENT = 15,
  HIRC_MOTION_BUS = 16,
  HIRC_MOTION_FX = 17,
  HIRC_EFFECT = 18,
  HIRC_AUXILIARY_BUS = 20
};

void read_hirc(void *data, char *path);
#endif

/* vim: ts=2 sw=2 expandtab
*/
