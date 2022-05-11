#pragma once

#include "buf_type.h"

#include <spa/param/audio/format-utils.h>
#include <spa/param/audio/type-info.h>

#include <functional>

#define E_PW_CAPTURE_OK 0
#define E_PW_CAPTURE_STREAM_NOT -2
#define E_PW_CAPTURE_STREAM_CONNECT -3


class pw_registry;
class pw_stream;


namespace spr::pw {

  using fun_callback_t=std::function<void(buf_t*,size_t)>;

  class pw_wrapper;

  class pw_recorder {
  private:

    uint32_t pw_target_id;
    const char *pw_target_name;

    struct spa_hook pw_registry_listener;
    struct pw_registry *pw_registry_proxy;
    struct spa_hook pw_stream_listener;
    struct pw_stream *pw_stream;

    uint32_t frame_size;
    uint32_t sample_rate;

    pw_wrapper *m_pipewire_wrapper;

    fun_callback_t m_callback;

  public:

    pw_recorder(fun_callback_t callback);
  
  protected:

    void on_stream_process();
  
    void on_stream_param_changed(uint32_t id,
                                 const struct spa_pod *param);

  public:
    int pipewire_start_streaming(uint32_t node_id);

    /*callbacks*/
  
    static void pipewire_global_added(void *data, uint32_t id, uint32_t permissions,
                                      const char *type, uint32_t version,
                                      const struct spa_dict *props);

    static void pipewire_global_removed(void *data, uint32_t id);

    static void on_stream_process(void *data);

    static void on_stream_param_changed(void *data, uint32_t id, const struct spa_pod *param);


    /*API*/
  
    static pw_recorder *pipewire_capture_create(fun_callback_t callback);

    static void pipewire_capture_destroy(void *data);
  };
}
