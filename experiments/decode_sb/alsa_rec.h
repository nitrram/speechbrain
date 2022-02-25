#pragma once

#include <alsa/asoundlib.h>

#include <unistd.h>
#include <stdint.h>

#define E_ALSA_REC_UNABLE_OPEN_DEVICE      -1
#define E_ALSA_REC_UNABLE_WRITE_HW_PARAMS  -2
#define E_ALSA_REC_SUCCESS                  0


static snd_pcm_t *_handle = NULL;
static uint8_t *_buffer = NULL;
static int _exitting = 0;

#ifdef __cplusplus
extern "C"{
#endif

  int start_recording(int (*callback)(uint8_t *buf, size_t siz));

#ifdef __cplusplus
}
#endif
