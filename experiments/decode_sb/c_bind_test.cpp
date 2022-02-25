#include <iostream>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <mutex>

#include "c_circular.h"
#include "alsa_rec.h"

static circular_t buffer;

int record_callback(uint8_t *data, size_t size) {

  buffer.put(data, size);
  
  return size;
}


extern "C" {

  void init() {

    if(start_listening(record_callback) < 0) {
      std::cerr << "failure in starting alsa" << std::endl;
    } 
    
  }


  uint8_t *poll_frames() {

    uint8_t alt[128];

    for(size_t i = 0;i<sizeof(alt); alt[i] = i++);

    buffer.put(alt, sizeof(alt));
    

    uint8_t res[1024];
    int read_size = buffer.read_safe(res, sizeof(res));

    return (read_size > 0 ? res : NULL);
  }

  void release() {
  }

}
