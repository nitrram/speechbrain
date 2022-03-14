#include "c_circular.h"

#include <cstring>
#include <iostream>

void circular_t::put(buf_t *block, size_t size) {

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

  //  std::cout << "cbuffer ahead: " << ahead << "[bytes]\n";
}

/*
uint8_t * c_circular_t::read(size_t size) {
  size_t orig_start = start;
  start += size;
  start %= N;
  
  return data + orig_start;
}
*/

size_t circular_t::read_safe(buf_t *mem, size_t size) {

  std::lock_guard<std::recursive_mutex> lck(mtx);

  int64_t orig_ahead = ahead.fetch_sub(size);

  if(orig_ahead <= static_cast<int64_t>(size)) {
    ahead.fetch_add(size);
    return 0L;
  }

  std::cout << "cbuffer read: " << size << "[bytes]; " << orig_ahead << " - [bytes] ahead\n" ;

  size_t orig_start = start.fetch_add(size);
  
  if((orig_start + size) < N) {
    memcpy(mem, data + start, size);
  } else {
    size_t carry = (orig_start + size) - N;
    memcpy(mem, data + orig_start, carry*sizeof(buf_t));
    memcpy(mem + carry, data, (size - carry)*sizeof(buf_t));
  }
  
  start.store(start.load() % N);

  return size;
}

