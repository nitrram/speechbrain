#pragma once

#include "buf_type.h"

#include <pulse/simple.h>

#define E_PULSE_REC_UNABLE_OPEN_DEVICE      -1
#define E_PULSE_REC_UNABLE_WRITE_HW_PARAMS  -2
#define E_PULSE_REC_SUCCESS                  0

static buf_t *_buffer = NULL;

#define PULSE_REC_SAMPLE_RATE 16000

#ifdef __cplusplus
extern "C"{
#endif

  int start_recording(int (*callback)(buf_t *buf, size_t siz));

#ifdef __cplusplus
}
#endif
