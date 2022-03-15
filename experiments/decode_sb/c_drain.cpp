#include "c_drain.h"

#include <cstring>
#include <iostream>

void drain_t::put(buf_t *block, size_t size_in_bytes) {

  std::lock_guard<std::recursive_mutex> lck(mtx);

  size_t size_in_frames = size_in_bytes/sizeof(buf_t);
  size_t orig_end = end.fetch_add(size_in_frames);

  if((orig_end + size_in_frames) < N) {
    std::cout << "drain_t::put copying into data on the " << orig_end*sizeof(buf_t) << " offset\n";
    memcpy(data + orig_end, block, size_in_bytes);

  } else {
    size_t carry = (orig_end + size_in_frames) - N;

    std::cout << "carry from " << orig_end << " to " << carry << std::endl;

    memcpy(data + orig_end, block, (N - orig_end)*sizeof(buf_t));
    memcpy(data, block, (size_in_frames - carry) * sizeof(buf_t));
  }

  end.store(end.load() % N);
}

size_t drain_t::read_safe(buf_t *mem) {

  std::lock_guard<std::recursive_mutex> lck(mtx);

  size_t orig_end = end.load();
  int size_to_end;

  if( orig_end < N ) {
    size_to_end = orig_end;
  } else {
    size_to_end = N;
  }

  memcpy(mem, data, size_to_end*sizeof(buf_t));

  return size_to_end;
}
