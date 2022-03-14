#pragma once

#include "buf_type.h"

#include <atomic>
#include <mutex>

class circular_t {

private:
  enum { N = 4096 };
  buf_t data[N];
  std::atomic_size_t start = 0;
  std::atomic_size_t end = 0;
  std::atomic_int64_t ahead = 0;
  std::recursive_mutex mtx;

public:
  void put(buf_t *block, size_t size);

  //uint8_t * read(size_t size);
  
  size_t read_safe(buf_t *mem, size_t size);
};
