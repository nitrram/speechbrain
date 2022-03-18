#include "c_drain.h"

#include <cstring>
#include <iostream>

#ifdef __GNUC__
#define LSX_UNUSED  __attribute__ ((unused)) /* Parameter or local variable is intentionally unused. */
#else
#define LSX_UNUSED /* Parameter or local variable is intentionally unused. */
#endif

#define LSX_USE_VAR(x) ((void)(x))

using sample_aligned_t = int;

#define SAMPLE_MAX 0x7FFFFFFF

#define SAMPLE_LOCALS sample_aligned_t macro_temp_sample LSX_UNUSED; \
  double macro_temp_double LSX_UNUSED

#define SIGNED_TO_SAMPLE(bits,d)((sample_aligned_t)(d)<<(32-bits))

#define SIGNED_16BIT_TO_SAMPLE(d,clips) SIGNED_TO_SAMPLE(16,d)

#define SAMPLE_TO_FLOAT_32BIT(d,clips) (LSX_USE_VAR(macro_temp_double),macro_temp_sample=(d),macro_temp_sample>SAMPLE_MAX-64?++(clips),1:(((macro_temp_sample+64)&~127)*(1./(SAMPLE_MAX+1.))))


void drain_t::put(buf_t *block, size_t size_in_bytes) {

  std::lock_guard<std::recursive_mutex> lck(mtx);

  size_t size_in_frames = size_in_bytes/sizeof(buf_t);
  size_t orig_end = end.fetch_add(size_in_frames);

  if((orig_end + size_in_frames) < N) {
#ifdef DEBUG
    std::cout << "drain_t::put copying into data on the " << orig_end*sizeof(buf_t) << " offset\n";
#endif
    memcpy(data + orig_end, block, size_in_bytes);

  } else {
    size_t carry = (orig_end + size_in_frames) - N;

#ifdef DEBUG
    std::cout << "carry from " << orig_end << " to " << carry << std::endl;
#endif

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

#ifdef DEBUG
  std::cout << "drain_t::read_safe ~ gonna read " << size_to_end*sizeof(buf_t) << "[bytes]\n";
#endif

  memcpy(mem, data, size_to_end*sizeof(buf_t));

  return size_to_end;
}


torch::Tensor drain_t::read_into_tensor() {

  std::lock_guard<std::recursive_mutex> lck(mtx);
  
  torch::Tensor tensor = torch::empty({static_cast<int64_t>(bytes_ready()), 1}, torch::kFloat32);
  int dummy = 0;

  SAMPLE_LOCALS;

  auto ptr = tensor.data_ptr<float_t>();
  for (int32_t i = 0; i < tensor.numel(); ++i) {

    sample_aligned_t sample = SIGNED_16BIT_TO_SAMPLE(data[i], dummy);
    ///fixme: clip to bounds, casting is not sufficient
    ptr[i] = SAMPLE_TO_FLOAT_32BIT(sample, dummy);
  }

  return tensor;
}
