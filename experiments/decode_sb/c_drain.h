#pragma once

#include "buf_type.h"

#include <atomic>
#include <mutex>

#include <torch/script.h>

class drain_t {
 private:
  enum { N = 64000 };
  buf_t data[N];
  std::atomic_size_t end = 0;
  std::recursive_mutex mtx;

 public:
  void put(buf_t* block, size_t size);

  size_t read_safe(buf_t *mem);

  /*  torch::Tensor read_into_tensor(c10::ScalarType type); */
  torch::Tensor read_into_tensor();

  inline size_t capacity() const {
    return N;
  }

  inline size_t bytes_ready() const {
    return end.load() * sizeof(buf_t);
  }
};
