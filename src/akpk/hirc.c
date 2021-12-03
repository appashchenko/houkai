#include "hirc.h"

void read_hirc(void *data, char *path) {
  hirc_header_t *header = (hirc_header_t *)data;

  data = (void *)((uintptr_t)data + sizeof(hirc_header_t));

  for (uint32_t i = 0; i < header->count; i++) {
    hirc_object_t *object = (hirc_object_t *)data;

    switch (object->type) {
    case HIRC_SOUND: {
      hirc_obj_snd *sound = (hirc_obj_snd *)data;

#ifdef VERBOSE
      printf(">>>>HIRC-SOUND#%X PARENT#%X\n", sound->audio_id, sound->group_id);
#endif
    } break;
    case HIRC_MUSIC_TRACK: {
      hirc_obj_mt *track = (hirc_obj_mt *)data;

#ifdef VERBOSE
      printf(">>>>HIRC-MUSICTRCK#%X\n", track->id);
#endif
    } break;
    case HIRC_MOTION_FX: {
      hirc_obj_mfx *mfx = (hirc_obj_mfx *)data;

#ifdef VERBOSE
      printf(">>>>HIRC MFX#%X\n", mfx->id);
#endif
    } break;
    default:
#ifdef VERBOSE
      fprintf(stderr, "HIRC object type %u is not supported yet. Skip\n",
              object->type);
#endif
      break;
    }

    data = (void *)((uintptr_t)data + object->size + sizeof(hirc_object_t));
  }
}
