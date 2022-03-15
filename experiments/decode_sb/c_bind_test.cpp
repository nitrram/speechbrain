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

//static circular_t buffer;
static drain_t buffer;

//static torch::Tensor _tensor = torch::empty({115200, 1}, torch::kFloat32);

static std::promise<void> signal_exit;
static std::future<void> stop_recording;

//static torch::Tensor tensor;

int _exitting;

size_t __buffer_size = 1024;

int record_callback(buf_t *data, size_t size) {

  std::cout << "callback data of size " << size << "[bytes]\n";

  buffer.put(data, size);

  return size;
}


extern "C" {

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

  auto poll_tensor() -> torch::Tensor {


    //    return torch::empty({static_cast<int64_t>(__buffer_size), 1}, torch::kFloat32).contiguous();
    //    return  at::ones({2, 2}, at::kInt);


    /*
    buf_t res[__buffer_size];
    std::cout << "poll_tensor: " << sizeof(res) << "[bytes] reserved\n";
    size_t read_size = buffer.read_safe(res);
    std::cout << "poll_tensor: " << read_size * sizeof(buf_t) << "[bytes] read\n";
    */

    torch::Tensor tensor = torch::empty({static_cast<int64_t>(__buffer_size), 1}, torch::kInt16);

    std::cout << "poll_tensor: size of tensor in bytes: " << tensor.numel() * torch::elementSize(torch::typeMetaToScalarType(tensor.dtype())) << std::endl;

    tensor = tensor.contiguous();

    std::cout << "poll_tensor: contiguous\n";

    return  tensor;

    /*
    if(read_size == 0) {
      return torch::empty({static_cast<int64_t>(__buffer_size), 1}, torch::kInt16);
    }
    torch::Tensor tensor = torch::empty({static_cast<int64_t>(read_size), 1}, torch::kInt16);
    std::cout << "poll_tensor: " << read_size * sizeof(buf_t) << "[bytes] read\n";
    std::cout << "poll_tensor: size of tensor in bytes: " << tensor.numel() * torch::elementSize(torch::typeMetaToScalarType(tensor.dtype())) << std::endl;
    std::cout << "poll_tensor: filling tensor >>>>\n";
    auto ptr = tensor.data_ptr<buf_t>();
    std::cout << "poll_tensor: filling tensor >>>>\n";

    for (size_t i = 0; i < read_size; ++i) {

      std::cout << i << " ";

      ptr[i] = static_cast<buf_t>(res[i]);
    }
    std::cout << std::endl;

    if(read_size > 0)
      std::cout << "read " << read_size * sizeof(buf_t) << "[bytes]\n";
    */

    /* transposing is only applicable if channel_first is selected, and since we're working wih mono only,*/
    /* there's no need for it*/
    //    return tensor./*transpose(1,0).*/contiguous();
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

}
