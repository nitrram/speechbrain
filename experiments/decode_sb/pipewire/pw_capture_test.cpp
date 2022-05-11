#include "pw_capture.h"

#include <iostream>
#include <chrono>
#include <thread>

#include "buf_type.h"


int main(int argc, char *argv[]) {

  using namespace spr::pw;
  
  pw_recorder *rec =  pw_recorder::pipewire_capture_create(
                                                           [](buf_t * buffer, size_t size){
                                                             std::cout << "receiving buffer of size " << size << "[frames] (" << buffer[18] << ")\n"; });
  
  rec->pipewire_start_streaming(333);
  
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));
  
  pw_recorder::pipewire_capture_destroy(rec);
    
  return 0;
}

