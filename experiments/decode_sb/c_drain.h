#pragma once

#include "buf_type.h"

#include <atomic>
#include <mutex>


class drain_t {
 private:
  enum { N = 115200 };
  buf_t data[N];
  std::atomic_size_t end = 0;
  std::recursive_mutex mtx;

 public:
  void put(buf_t* block, size_t size);

  size_t read_safe(buf_t *mem);

  inline size_t capacity() const {
    return N;
  }
};
