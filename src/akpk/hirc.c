#include "hirc.h"

void read_hirc(void *data, char *path) {
  struct hirc_header *header = (struct hirc_header *)data;
  uint32_t i;

  data = (void *)((uintptr_t)data + sizeof(struct hirc_header));

  for (i = 0; i < header->count; i++) {
    struct hirc_object *object = (struct hirc_object *)data;

    switch (object->type) {
    case HIRC_SOUND: {
      struct hirc_obj_snd *sound = (struct hirc_obj_snd *)data;
    } break;
    case HIRC_MUSIC_TRACK: {
      struct hirc_obj_mt *track = (struct hirc_obj_mt *)data;
    } break;
    case HIRC_MOTION_FX: {
      struct hirc_obj_mfx *mfx = (struct hirc_obj_mfx *)data;
    } break;
    default:
      fprintf(stderr, "HIRC object type %u is not supported yet. Skip\n",
              object->type);
      break;
    }

    data =
        (void *)((uintptr_t)data + object->size + sizeof(struct hirc_object));
  }
}

/* vim: ts=2 sw=2 expandtab
 */
