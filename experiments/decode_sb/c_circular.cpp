#include "c_circular.h"

#include <cstring>

void circular_t::put(uint8_t *block, size_t size) {

  std::lock_guard<std::recursive_mutex> lck(mtx);
  
  size_t orig_end = end.fetch_add(size);
  ahead.fetch_add(size);
  if((orig_end + size) < N) {
    memcpy(data + orig_end, block, size);
  } else {
    size_t carry = (orig_end + size) - N;
    memcpy(data + orig_end, block, carry);
    memcpy(data, block, size - carry);
  }
  
  end.store(end.load() % N);
}

/*
uint8_t * c_circular_t::read(size_t size) {
  size_t orig_start = start;
  start += size;
  start %= N;
  
  return data + orig_start;
}
*/

size_t circular_t::read_safe(uint8_t *mem, size_t size) {

  std::lock_guard<std::recursive_mutex> lck(mtx);

  int64_t orig_ahead = ahead.fetch_sub(size);

  if(orig_ahead <= static_cast<int64_t>(size)) {
    ahead.fetch_add(size);
    return 0L;
  }

  size_t orig_start = start.fetch_add(size);
  
  if((orig_start + size) < N) {
    memcpy(mem, data + start, size);
  } else {
    size_t carry = (orig_start + size) - N;
    memcpy(mem, data + orig_start, carry);
    memcpy(mem + carry, data, size - carry);
  }
  
  start.store(start.load() % N);

  return size;
}

