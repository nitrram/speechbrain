#include "alsa_rec.h"

int start_recording(int (*callback)(uint8_t *buf, size_t siz)) {

  int rc;
  int size;
  int psize;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;

  int safe_limit = INT32_MIN + 8192;

  /* Open PCM device for recording (capture). */
  rc = snd_pcm_open(&_handle, "default", SND_PCM_STREAM_CAPTURE, 0);
  if (rc < 0) {
    fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
    return E_ALSA_REC_UNABLE_OPEN_DEVICE;
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(_handle, params);

  /* Set the desired hardware parameters. */

  /* Interleaved mode */
  snd_pcm_hw_params_set_access(_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(_handle, params, SND_PCM_FORMAT_S16_LE);

  /* One channel (mono) */
  snd_pcm_hw_params_set_channels(_handle, params, 1);

  /* 16000 bits/second sampling rate */
  val = 16000;
  snd_pcm_hw_params_set_rate_near(_handle, params, &val, &dir);

  /* Set period size to 2048 frames. */
  frames = 2048;
  snd_pcm_hw_params_set_period_size_near(_handle, params, &frames, &dir);

  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(_handle, params);
  if (rc < 0) {
    fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
    return E_ALSA_REC_UNABLE_WRITE_HW_PARAMS;
  }

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params, &frames, &dir);
  size = frames * 2; /* 2 bytes/sample, 1 channels */
  _buffer = (uint8_t *) malloc(size);

  int skip_n_frames = 8192;
  while (!_exitting) {

    skip_n_frames -= frames;
    rc = snd_pcm_readi(_handle, _buffer, frames);
    if (rc == -EPIPE) {
      /* EPIPE means overrun */
      fprintf(stderr, "overrun occurred\n");
      snd_pcm_prepare(_handle);
    } 
    else if (rc < 0) {
      fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
    } 
    else if (rc != (int)frames) {
      fprintf(stderr, "short read, read %d frames\n", rc);
    }

    if(!_exitting && skip_n_frames < 0) {
      /*      rc = write(1, _buffer, size); */
      psize = size;
      while( (psize -= callback(_buffer, size)) > 0 ) { 
        fprintf(stderr, "short write: wrote %d bytes\n", rc);
      }

    } else if(skip_n_frames <= safe_limit) {
      skip_n_frames = 0; /* don't let it underflow */
    }
  }


  free(_buffer);
  printf("\33[0;32mDone\n\033[0;0m");


  return E_ALSA_REC_SUCCESS;
}
