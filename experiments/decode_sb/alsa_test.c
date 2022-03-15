#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include <unistd.h>
#include <signal.h>
#include <stdint.h>


static snd_pcm_t *_handle;
static uint8_t *_buffer;

static int _exitting = 0;


void signal_callback_handler(int signum) {

  printf("exitting %d\n", signal);

  _exitting = 1;
}

int main(int argc, char *argv[]) {
  /*int key=0;
  long loops;
  */
  int rc;
  int size;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;

  /*  int safe_limit = INT32_MIN + 8192;*/

  signal(SIGINT, signal_callback_handler);

  /* Open PCM device for recording (capture). */
  rc = snd_pcm_open(&_handle, "default", SND_PCM_STREAM_CAPTURE, 0);
  if (rc < 0) {
    fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
    exit(1);
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
  /*frames = 2048;*/
  frames = 256;
  snd_pcm_hw_params_set_period_size_near(_handle, params, &frames, &dir);

  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(_handle, params);
  if (rc < 0) {
    fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
    exit(1);
  }

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params, &frames, &dir);
  size = frames * 2; /* 2 bytes/sample, 1 channels */
  _buffer = (uint8_t *) malloc(size);


  fprintf(stderr, "\033[0;32mready...\033[0;0m\n");

  /*  int skip_n_frames = 8192; */
  while (!_exitting) {

    /*    skip_n_frames -= frames;*/
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

    /* if(!_exitting && skip_n_frames < 0) {*/
      rc = write(1, _buffer, size);
      /*      printf("buffer written\n");*/
      if (rc != size)
        fprintf(stderr, "short write: wrote %d bytes\n", rc);
      /*} else if(skip_n_frames <= safe_limit) {
      skip_n_frames = 0;
      } */

      /*printf("round %d\n", !_exitting);*/
  }


  /*  snd_pcm_drain(_handle);*/
  snd_pcm_drop(_handle);

  snd_pcm_close(_handle);

  free(_buffer);


  return EXIT_SUCCESS;
}
