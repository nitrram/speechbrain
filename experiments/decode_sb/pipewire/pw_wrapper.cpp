#include "pw_wrapper.h"

namespace spr::pw {

  void pw_wrapper::init() {
    if (m_pipewire_refs == 0) {
      pw_init(NULL, NULL);

      m_pipewire_mainloop = pw_thread_loop_new(m_tag, NULL);

      pw_thread_loop_lock(m_pipewire_mainloop);

      if (pw_thread_loop_start(m_pipewire_mainloop) != 0) {
        pw_thread_loop_destroy(m_pipewire_mainloop);
        pw_thread_loop_unlock(m_pipewire_mainloop);
        return;
      };

      m_pipewire_context = pw_context_new(pw_thread_loop_get_loop(m_pipewire_mainloop), NULL, 0);

      m_pipewire_core = pw_context_connect(m_pipewire_context, NULL, 0);

      pw_thread_loop_unlock(m_pipewire_mainloop);
    }
    m_pipewire_refs++;
  };

  void pw_wrapper::unref() {
    if (--m_pipewire_refs == 0) {
      if (m_pipewire_mainloop) {
        pw_thread_loop_stop(m_pipewire_mainloop);

        if (m_pipewire_core) {
          pw_core_disconnect(m_pipewire_core);
        }

        if (m_pipewire_context) {
          pw_context_destroy(m_pipewire_context);
        }

        pw_thread_loop_destroy(m_pipewire_mainloop);
      }

      pw_deinit();
    }
  }

  struct pw_registry *pw_wrapper::add_registry_listener(bool call_now, struct spa_hook *hook,
                                                        const struct pw_registry_events *callbacks, void *data) {
    pw_thread_loop_lock(m_pipewire_mainloop);

    struct pw_registry *pipewire_registry =
      pw_core_get_registry(m_pipewire_core, PW_VERSION_REGISTRY, 0);
    /*inspect the last NULL -> should be callback (void*)data*/
    pw_registry_add_listener(pipewire_registry, hook, callbacks, data);

    if (call_now)
      pw_thread_loop_wait(m_pipewire_mainloop);

    pw_thread_loop_unlock(m_pipewire_mainloop);

    return pipewire_registry;
  }

  void pw_wrapper::proxy_destroy(struct pw_proxy *proxy) {
    pw_thread_loop_lock(m_pipewire_mainloop);
    pw_proxy_destroy(proxy);
    pw_thread_loop_unlock(m_pipewire_mainloop);
  }

  struct pw_stream *pw_wrapper::stream_new(struct pw_properties *props,
                                           struct spa_hook *stream_listener,
                                           const struct pw_stream_events *callbacks,
                                           void *data) {

    pw_thread_loop_lock(m_pipewire_mainloop);

    struct pw_stream *stream =
      pw_stream_new(m_pipewire_core, m_tag, props);
    /*inspect the last NULL -> should be callback (void*)data*/
    pw_stream_add_listener(stream, stream_listener, callbacks, data);

    pw_thread_loop_unlock(m_pipewire_mainloop);

    return stream;
  }


  int pw_wrapper::stream_connect(struct pw_stream *stream,
                                 enum spa_direction direction, uint32_t target_id,
                                 enum pw_stream_flags flags,
                                 const struct spa_pod **params, uint32_t n_params) {
    pw_thread_loop_lock(m_pipewire_mainloop);


    int res = -1;
    res = pw_stream_connect(stream, direction, target_id, flags, params,
                            n_params);

    pw_thread_loop_unlock(m_pipewire_mainloop);;

    return res;
  }

  int pw_wrapper::stream_disconnect(struct pw_stream *stream) {
    pw_thread_loop_lock(m_pipewire_mainloop);

    int res = -1;
    res = pw_stream_disconnect(stream);

    pw_thread_loop_unlock(m_pipewire_mainloop);;

    return res;
  }

  void pw_wrapper::stream_destroy(struct pw_stream *stream) {
    pw_thread_loop_lock(m_pipewire_mainloop);

    pw_stream_disconnect(stream);
    pw_stream_destroy(stream);

    pw_thread_loop_unlock(m_pipewire_mainloop);
  }

  void pw_wrapper::pipewire_continue() {
    pw_thread_loop_signal(m_pipewire_mainloop, false);
  }

}
