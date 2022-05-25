#include "c_drain.h"
#include "log.h"

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

  std::lock_guard<std::recursive_mutex> lck(m_mtx);
  size_t size_in_frames = size_in_bytes/sizeof(buf_t);
  size_t orig_end = m_end.fetch_add(size_in_frames);

  SPR_DLOG("drain_t::put size_in_frames: %lu| orig_end: %lu\n", size_in_frames, orig_end);

  if((orig_end + size_in_frames) < N) {
    SPR_DLOG("drain_t::put copying into data on the %lu offset\n", orig_end*sizeof(buf_t));
    memcpy(m_data + orig_end, block, size_in_bytes);
  } else {
    size_t carry = (orig_end + size_in_frames) - N;
    m_carry_end = N - orig_end;
    SPR_DLOG("drain_t::put carry from %lu to %ld\n", orig_end, carry);
    memcpy(m_data + orig_end, block, m_carry_end*sizeof(buf_t));
    memcpy(m_data, block, (size_in_frames - carry) * sizeof(buf_t));
  }

  m_end.store(m_end.load() % N);
  SPR_DLOG("drain_t::put current_end %lu\n", m_end.load());
}

size_t drain_t::read_safe(buf_t *mem) {

  std::lock_guard<std::recursive_mutex> lck(m_mtx);
  size_t orig_end = m_end.load();
  int size_to_end;

  if( orig_end < N ) {
    size_to_end = orig_end;
  } else {
    size_to_end = N;
  }

  SPR_DLOG("drain_t::read_safe ~ gonna read %lu [bytes]\n", size_to_end*sizeof(buf_t));
  memcpy(mem, m_data, size_to_end*sizeof(buf_t));
  return size_to_end;
}

torch::Tensor drain_t::read_into_tensor() {

  std::lock_guard<std::recursive_mutex> lck(m_mtx);

  size_t frames_end_cnt = frames_ready();
  size_t frames_processed_cnt = frames_end_cnt + m_carry_end;
  
  SPR_DLOG("drain_t::read_into_tensor ~ gonna read %lu\n", frames_processed_cnt);
	/*	printf("drain_t::read_into_tensor ~ gonna read [rd: %lu, cr: %lu]%lu\n", frames_end_cnt, m_carry_end, frames_processed_cnt);*/
  torch::Tensor tensor = torch::empty({static_cast<int64_t>(frames_processed_cnt), 1}, torch::kFloat32);

  if(tensor.numel() <= 0)
    return tensor;

  int dummy = 0;
  SAMPLE_LOCALS;

  auto ptr = tensor.data_ptr<float_t>();

  // carry
	// if (frames_end_count) > (N-m_carry_end):
	//   cue =  ( (frames_end_count > (N-m_carry_end))? (N-frames_end_count) : (N-m_carry_end));
  size_t i;
	/*
	size_t cue = ((frames_end_cnt > (N-m_carry_end))? (N-frames_end_cnt) : (N-m_carry_end));
  for (i = cue; i < N; ++i) {
    sample_aligned_t sample =  SIGNED_16BIT_TO_SAMPLE(m_data[i], dummy);
    ptr[i - cue] = SAMPLE_TO_FLOAT_32BIT(sample, dummy);
  }
	*/
  
  for (i = 0; i < frames_end_cnt; ++i) {
    // int16_t -> int32_t (sample_aligned_t)
    sample_aligned_t sample = SIGNED_16BIT_TO_SAMPLE(m_data[i], dummy);
    // int32_t -> float32_t
    ptr[i] = SAMPLE_TO_FLOAT_32BIT(sample, dummy);
  }

  if(m_end.load() >= static_cast<size_t>(tensor.numel())) {
      auto orig_end = m_end.fetch_sub(tensor.numel());
      m_carry_end = 0;
      SPR_DLOG("drain_t::read_from_tensor orig_end: %lu| tensor.numel(): %lu| size of buffer: %lu| current_end: %lu\n", orig_end, tensor.numel(), sizeof(buf_t), m_end.load());
  }

  return tensor;
}
