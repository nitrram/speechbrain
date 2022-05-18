#include <iostream>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <thread>
#include <future>
#include <tuple>

#include <torch/script.h>


//#include "pipewire/pw_capture.h"

#include "c_circular.h"
#include "c_drain.h"
#include "buf_type.h"
//#include "alsa/alsa_rec.h"
#include "pulse/pulse_rec.h"
#include "log.h"

static drain_t buffer;

//static spr::pw::pw_recorder *_rec;

static std::promise<void> signal_exit;
static std::future<void> stop_recording;

int _exitting;
size_t __buffer_size = 512;

int record_callback(buf_t *data, size_t size) {

  SPR_DLOG("record_callback: data of size %lu [bytes]\n", size);
  
  buffer.put(data, size);

  return size;
}


namespace spr::sb {

  void init() {

    std::cout << "init recording...\n";
		stop_recording = signal_exit.get_future();

		/*
    _rec = spr::pw::pw_recorder::pipewire_capture_create(
                                                [&](buf_t * data, size_t size) {
                                                  
                                                  SPR_DLOG("record_callback: data of size %lu [bytes]\n", size);
                                                  
                                                  buffer.put(data, size);
                                                  
                                                });
    _rec->pipewire_start_streaming(333);
		*/

    /*
    std::thread([&]() {

      if(start_recording(record_callback) < 0) {
        std::cerr << "failure in starting alsa" << std::endl;
      }

      signal_exit.set_value();
    }).detach();
		*/
		
		std::thread([&]() {
			
      if(start_recording(record_callback) < 0) {
        std::cerr << "failure in starting alsa" << std::endl;
      }
			
      signal_exit.set_value();
    }).detach();
    
  }

  auto poll_tensor() -> std::tuple<torch::Tensor, int64_t> {

    const torch::Tensor &tensor = buffer.read_into_tensor();

    SPR_DLOG("poll_tensor: size of tensor in bytes: %lu\n", tensor.numel() * torch::elementSize(torch::typeMetaToScalarType(tensor.dtype())));

    /* transposing is only applicable if channel_first is selected, and since we're working wih mono only,*/
    /* there's no need for it*/
    return  std::make_tuple(tensor./*transpose(1,0).*/contiguous(), PULSE_REC_SAMPLE_RATE);
  }

  buf_t *poll_frames() {

    buf_t res[1024];
    int read_size = buffer.read_safe(res);

    return (read_size > 0 ? res : NULL);
  }

  void release() {
    std::cout << "releaseing c_bind\n";
		//    spr::pw::pw_recorder::pipewire_capture_destroy(_rec);
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

