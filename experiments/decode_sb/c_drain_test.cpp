#include "c_drain.h"

#include <iostream>
#include <cstdint>
#include <cstring>
#include <thread>




int main(int argc, char *argv[]) {

  buf_t mem[4096];

  size_t mem_size = sizeof(mem) / sizeof(buf_t);

  std::cout << "memory size in frames: " << mem_size << std::endl;
  
  for(size_t i=0; i< mem_size; ++i) {
    mem[i] = i + 1;    
  }

  drain_t buffer;
  buf_t data[buffer.capacity()];

	std::thread t([](){
		for(int i=0; i < 1000; ++i, buffer.put(mem, sizeof(mem))); });
  //  buffer.put(mem, sizeof(mem));
  //  buffer.put(mem, sizeof(mem));

	std::thread r([](){
		size_t read_size = buffer.read_safe(data);
		std::cout << "read: " << read_size * sizeof(buf_t) << "[bytes]\n";
	});

  /*  
  std::cout << "[\n";
  for(size_t i=0; i< read_size; ++i) {
    std::cout << data[i] << " ";
  }
  std::cout << "\n]\n";
  */
  
  
  return 0;
}
