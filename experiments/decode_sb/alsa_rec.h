#pragma once

#include "buf_type.h"

#include <alsa/asoundlib.h>

#define E_ALSA_REC_UNABLE_OPEN_DEVICE      -1
#define E_ALSA_REC_UNABLE_WRITE_HW_PARAMS  -2
#define E_ALSA_REC_SUCCESS                  0

static snd_pcm_t *_handle = NULL;
static buf_t *_buffer = NULL;

#ifdef __cplusplus
extern "C"{
#endif

  int start_recording(int (*callback)(buf_t *buf, size_t siz));

#ifdef __cplusplus
}
#endif
