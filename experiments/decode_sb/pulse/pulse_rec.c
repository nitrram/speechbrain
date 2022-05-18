#include "pulse_rec.h"

#include <pulse/error.h>
#include <cstdio>
#include <cstdlib>

extern int _exitting;


int start_recording(int (*callback)(buf_t *buf, size_t siz)) {

	    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = PULSE_REC_SAMPLE_RATE,
        .channels = 1
    };
    pa_simple *s = NULL;
    int ret = 1;
    int error;

		int size = 512 * sizeof(buf_t);
		int psize;

		_buffer = (buf_t*)malloc(size);
		
 
    /* Create the recording stream */
    if (!(s = pa_simple_new(NULL, "speechbrain_pulse", PA_STREAM_RECORD, NULL, "speechbrain record", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        goto finish;
    }

		int rc;
    while (!_exitting) {
 
        /* Record some data ... */
			if ( (rc=pa_simple_read(s, _buffer, size, &error)) < 0) {
            fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
            goto finish;
        }

				psize = size;
				while( (psize -= callback(_buffer, size) ) > 0 ) {
					fprintf(stderr, "short write: wrote %d bytes\n", rc);
				}

		/*	if(callback(buf, sizeof(buf)) {*/
        /* And write it to STDOUT */
						//        if (loop_write(STDOUT_FILENO, buf, sizeof(buf)) != sizeof(buf)) {
		/*            fprintf(stderr, __FILE__": write() failed: %s\n", strerror(errno));
            goto finish;
						}*/
    }
 
    ret = 0;
 
finish:
 
    if (s)
        pa_simple_free(s);

		if(_buffer)
			free(_buffer);
 
    return ret;
}
