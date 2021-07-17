#include "akpk.h"

struct hirc_header_t {
  uint32_t magic;
  uint32_t size;
  uint32_t count;
};
typedef struct hirc_header_t hirc_header_t;

struct hirc_object_info_t {
  uint8_t type;
  uint32_t size;
  uint32_t id;
};
typedef struct hirc_object_info_t hirc_object_info_t;

enum HIRC_OBJECT {
  HIRC_SETTINGS = 1,
  HIRC_SOUND = 2,
  HIRC_EVENT_ACTION = 3,
  HIRC_ACTION = 4,
  HIRC_CONTAINER = 5,
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
