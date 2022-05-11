#pragma once

#include <pipewire/pipewire.h>

namespace spr::pw {

  class pw_wrapper {
  private:
    uint32_t m_pipewire_refs = 0;
    struct pw_thread_loop *m_pipewire_mainloop = NULL;
    struct pw_context *m_pipewire_context = NULL;
    struct pw_core *m_pipewire_core = NULL;

    const char * const m_tag = "Speechbrain streaming";

  public:

    void init();
    void unref();

    void proxy_destroy(struct pw_proxy *proxy);
  
    struct pw_registry *add_registry_listener(bool call_now, struct spa_hook *hook,
                                              const struct pw_registry_events *callbacks,
                                              void *data);

    struct pw_stream *stream_new(struct pw_properties *props,
                                 struct spa_hook *stream_listener,
                                 const struct pw_stream_events *callbacks,
                                 void *data);

    int stream_connect(struct pw_stream *stream,
                       enum spa_direction direction, uint32_t target_id,
                       enum pw_stream_flags flags,
                       const struct spa_pod **params, uint32_t n_params);
  
    int stream_disconnect(struct pw_stream *stream);

    void stream_destroy(struct pw_stream *stream);

    void pipewire_continue();
  };
}
