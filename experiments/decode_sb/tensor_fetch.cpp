#include "alsa_rec.h"


#include <signal.h>

#include <iostream>
#include <sstream>

#ifdef DEBUG
#include <ctime>
#include <ratio>
#include <chrono>
#endif //DEBUG

#include <torch/script.h>


static uint8_t buffer[]


void signal_callback_handler(int signum) {

  printf("\033[0;31mexitting %d\n\033[0;0m", signal);

  //  _exitting = 1;
  exit(2);
}


int record_callback(uint8_t *buffer, size_t size) {
  std::ostringstream oss;
  
  for(size_t i=0; i<size; ++i) {
    oss << buffer[i] << ", ";
  }
  
  //  std::cout << oss.str() << std::endl;
  
  return size;
}


int main(int argc, char *argv[]) {

#ifdef DEBUG
  using namespace std::chrono;
#endif

  signal(SIGINT, signal_callback_handler);

  //int rc = start_recording(record_callback);

  uint16_t buffer[2048];

  std::cout << "sizeof buffer: " << sizeof(buffer) << std::endl;

  size_t nsamps = sizeof(buffer)/sizeof(uint16_t);
  
  for(size_t i=0; i<nsamps; buffer[++i] = (i % 255));

  torch::Tensor t;

  std::cout << "creating empty tensor" << std::endl;
  t = torch::empty({static_cast<int64_t>(nsamps), 1}, torch::kFloat32);
  
#ifdef DEBUG
  auto t0 = high_resolution_clock::now();
#endif //DEBUG
  
  std::cout << "filling tensor" << std::endl;
  auto ptr = t.data_ptr<float_t>();
  for (size_t i = 0; i < nsamps; ++i) {
    ptr[i] = static_cast<float_t>(buffer[i]);
  }

  t = t.transpose(1, 0);
  t = t.contiguous();
  //t = *t.expect_contiguous();

#ifdef DEBUG
  duration<double, std::milli> ts  = high_resolution_clock::now() - t0;
  std::cout << "time spent: " << ts.count() << "[ms]\n";
#endif //DEBUG


  


  
  std::cout << sizeof(buffer) << " samp: " << static_cast<int>(buffer[300]) <<std::endl;


  std::cout << "\033[0;32mDone\n\033[0;0m";



  return EXIT_SUCCESS;
}
