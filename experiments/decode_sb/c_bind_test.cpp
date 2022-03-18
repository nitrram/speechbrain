#include <iostream>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <thread>
#include <future>
#include <tuple>

#include <torch/script.h>

#include "c_circular.h"
#include "c_drain.h"
#include "buf_type.h"
#include "alsa_rec.h"

static drain_t buffer;

static std::promise<void> signal_exit;
static std::future<void> stop_recording;

int _exitting;
size_t __buffer_size = 1024;

int record_callback(buf_t *data, size_t size) {
#ifdef DEBUG  
  std::cout << "record_callback: data of size " << size << "[bytes]\n";
#endif

  buffer.put(data, size);

  return size;
}


namespace spr::sb {

  void init() {

    std::cout << "init recording...\n";
    stop_recording = signal_exit.get_future();
    std::thread([]() {
      if(start_recording(record_callback) < 0) {
        std::cerr << "failure in starting alsa" << std::endl;
      }

      signal_exit.set_value();
    }).detach();
  }

  auto poll_tensor() -> std::tuple<torch::Tensor, int64_t> {

    const torch::Tensor &tensor = buffer.read_into_tensor();
    
#ifdef DEBUG
    std::cout << "poll_tensor: size of tensor in bytes: " << tensor.numel() * torch::elementSize(torch::typeMetaToScalarType(tensor.dtype())) << std::endl;
#endif

    /* transposing is only applicable if channel_first is selected, and since we're working wih mono only,*/
    /* there's no need for it*/
    return  std::make_tuple(tensor./*transpose(1,0).*/contiguous(), ALSA_REC_SAMPLE_RATE);
  }

  buf_t *poll_frames() {
#ifdef DEBUG
    buf_t alt[128];
    for(size_t i = 0;i<sizeof(alt); alt[i] = i++);
    buffer.put(alt, sizeof(alt));
#endif
    buf_t res[1024];
    int read_size = buffer.read_safe(res);

    //    std::cout << "polling: " << read_size << "/" << sizeof(res)<<" [bytes]\n";

    return (read_size > 0 ? res : NULL);
  }

  void release() {
    std::cout << "releaseing c_bind\n";
    _exitting = 1;
    stop_recording.wait();
  }


  TORCH_LIBRARY_FRAGMENT(sprbind, m) {
    m.def("sprbind::poll_tensor",
          &spr::sb::poll_tensor);

    m.def("sprbind::init", &spr::sb::init);
    m.def("sprbind::release", &spr::sb::release);
  }
  
} /* spr::sb */

/***
 * python/c++ interop wraps:
 *
 * PyObject * THPVariable_Wrap(at::Tensor t);
 * at::Tensor& THPVariable_Unpack(PyObject* obj);
 *
 ***/

