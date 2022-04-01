#pragma once

#include "buf_type.h"

#include <atomic>
#include <mutex>

#include <torch/script.h>

class drain_t {
 private:
  enum { N = 10000 };
  buf_t m_data[N];
  std::atomic_size_t m_end = 0;
  size_t m_carry_end = 0;
  std::recursive_mutex m_mtx;

 public:
  void put(buf_t* block, size_t size);

  size_t read_safe(buf_t *mem);

  /*  torch::Tensor read_into_tensor(c10::ScalarType type); */
  torch::Tensor read_into_tensor();

  inline size_t capacity() const {
    return N;
  }

  inline size_t bytes_ready() const {
    return m_end.load() * sizeof(buf_t);
  }

  inline size_t frames_ready() const {
    return m_end.load();
  }
};
