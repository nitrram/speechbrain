#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#include <spa/param/audio/type-info.h>

#include "pw_capture.h"
#include "pw_wrapper.h"

#ifdef DEBUG
#define PW_DLOG(...) printf(__VA_ARGS__)
#else
#define PW_DLOG(...) (void)(0)
#endif


namespace spr::pw {

  static const uint32_t pw_global_node_id = 13;

  const struct pw_registry_events registry_events_enum = {
    PW_VERSION_REGISTRY_EVENTS,
    .global = pw_recorder::pipewire_global_added,
    .global_remove = pw_recorder::pipewire_global_removed,
  };

  const struct pw_stream_events stream_callbacks = {
    PW_VERSION_STREAM_EVENTS,
    .param_changed = pw_recorder::on_stream_param_changed,
    .process = pw_recorder::on_stream_process,
  };


  pw_recorder::pw_recorder(fun_callback_t callback) :
    m_pipewire_wrapper(new pw_wrapper()),
    m_callback(callback) {

    m_pipewire_wrapper->init();
  }


  void pw_recorder::on_stream_param_changed(uint32_t id,
                                            const struct spa_pod *param) {
    if (!param || id != SPA_PARAM_Format)
      return;

    struct spa_audio_info_raw audio_info;
    spa_format_audio_raw_parse(param, &audio_info);

    sample_rate = audio_info.rate;
    // format = spa_to_obs_audio_format(audio_info.format);
    //frame_size = calculate_frame_size(format, audio_info.channels);
  }

  int pw_recorder::pipewire_start_streaming(uint32_t node_id) {

    if (!pw_stream) {
      fprintf(stderr, "[PipeWire Audio Stream not opened!]\n");
      return E_PW_CAPTURE_STREAM_NOT;
    }

    uint8_t buffer[1024];
    struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

    const struct spa_pod *params[1];
    params[0] = static_cast<const spa_pod*>(spa_pod_builder_add_object(&b,
                                                                       SPA_TYPE_OBJECT_Format, SPA_PARAM_EnumFormat,
                                                                       SPA_FORMAT_mediaType, SPA_POD_Id(SPA_MEDIA_TYPE_audio),
                                                                       SPA_FORMAT_mediaSubtype, SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw),
                                                                       SPA_FORMAT_AUDIO_channels, SPA_POD_CHOICE_RANGE_Int(0, 1, 8),
                                                                       SPA_FORMAT_AUDIO_format,
                                                                       SPA_POD_CHOICE_ENUM_Id(5, SPA_AUDIO_FORMAT_UNKNOWN, SPA_AUDIO_FORMAT_U8,
                                                                                              SPA_AUDIO_FORMAT_S16_LE, SPA_AUDIO_FORMAT_S32_LE,
                                                                                              SPA_AUDIO_FORMAT_F32_LE)));

    m_pipewire_wrapper->stream_disconnect(pw_stream);
    if (m_pipewire_wrapper->stream_connect(pw_stream, PW_DIRECTION_INPUT, node_id,
                                           static_cast<enum pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS),
                                           params, 1) == 0) {
      pw_target_id = node_id;
    } else {
      fprintf(stderr, "[Pipewire Audio Stream could not connect to the nodeID!]\n");
      return E_PW_CAPTURE_STREAM_CONNECT;
    }

    return E_PW_CAPTURE_OK;
  }

  void pw_recorder::pipewire_global_added(void *data, uint32_t id, uint32_t permissions,
                                          const char *type, uint32_t version,
                                          const struct spa_dict *props) {

    pw_recorder *rec = static_cast<pw_recorder*>(data);
  
    (void)permissions;
    (void)version;
    if (!props || !id)
      goto done;

    if (strcmp(type, PW_TYPE_INTERFACE_Node) == 0) {
      const char *media_class =
        spa_dict_lookup(props, PW_KEY_MEDIA_CLASS);
      if (!media_class)
        goto done;

      const char *node_name = spa_dict_lookup(props, PW_KEY_NODE_NAME);
      if (!node_name)
        goto done;

      const char *node_friendly_name = spa_dict_lookup(props, PW_KEY_NODE_DESCRIPTION);
      if (!node_friendly_name)
        node_friendly_name = node_name;

      PW_DLOG("pipewire global added %s (%s) [is_stream %d] ~ %d\n", node_name, rec->pw_target_name, (rec->pw_stream != NULL ? pw_stream_get_state(rec->pw_stream, NULL) : -3), PW_STREAM_STATE_UNCONNECTED);

      //If we are disconnected, a node for the same target
      //(with the same name) might connect later, connect to that
      if (rec->pw_stream &&
          pw_stream_get_state(rec->pw_stream, NULL) == PW_STREAM_STATE_UNCONNECTED) {

        if (rec->pw_target_name && strcmp(rec->pw_target_name, node_name) == 0) {

          PW_DLOG("pipewire global added: (starting streaming)\n");
          rec->pipewire_start_streaming(id);
        }
      }
    }

  done:
    rec->m_pipewire_wrapper->pipewire_continue();
  }


  void pw_recorder::pipewire_global_removed(void *data, uint32_t id) {

    pw_recorder *rec = static_cast<pw_recorder*>(data);
  
    /*If the object we're connected to is lost, try to find one with the same name
      This is useful for application capture, applications disconnect and connect frequently
      and some times have multiple streams running with the same name
      (For example, Firefox runs a new stream for every tab)*/
    if (id == rec->pw_target_id) {
      rec->m_pipewire_wrapper->stream_disconnect(rec->pw_stream);
      rec->pw_target_id = 0;
      rec->pipewire_start_streaming(rec->pw_target_id);
    }

    //  done:
    rec->m_pipewire_wrapper->pipewire_continue();
  }

  void pw_recorder::on_stream_process(void *data) {

    pw_recorder *rec = static_cast<pw_recorder*>(data);

    struct pw_buffer *b;
    struct spa_buffer *buf;

    b = pw_stream_dequeue_buffer(rec->pw_stream);

    if (!b) {
      fprintf(stderr, "no buffer\n");
      return;
    }

    buf = b->buffer;

    void *d = buf->datas[0].data;
    if (d == NULL || buf->datas[0].type != SPA_DATA_MemPtr)
      goto queue;


    rec->m_callback(static_cast<buf_t*>(buf->datas[0].data), buf->datas[0].chunk->size / 2);

  queue:
    pw_stream_queue_buffer(rec->pw_stream, b);
  }


  void pw_recorder::on_stream_param_changed(void *data, uint32_t id, const struct spa_pod *param) {  
    if (!param || id != SPA_PARAM_Format)
      return;

    pw_recorder *rec = static_cast<pw_recorder*>(data);

    struct spa_audio_info_raw audio_info;
    spa_format_audio_raw_parse(param, &audio_info);

    rec->sample_rate = audio_info.rate;
    rec->frame_size = 2; /*calculate it out of the input format*/
  }

  pw_recorder *pw_recorder::pipewire_capture_create(fun_callback_t callback) {

    const char * strid = "PipeWire Speechbrain Bridge";
    
    pw_recorder *rec = new pw_recorder(callback);

    rec->pw_target_name = strdup(strid);

    rec->m_pipewire_wrapper->init();
 
    const char *capture_sink = "true";

    struct pw_properties *props = pw_properties_new(PW_KEY_APP_NAME, strid,
                                                    PW_KEY_APP_ICON_NAME, "none",
                                                    PW_KEY_NODE_NAME, strid,
                                                    PW_KEY_MEDIA_ROLE, "Production",
                                                    PW_KEY_MEDIA_TYPE, "Audio",
                                                    PW_KEY_MEDIA_CATEGORY, "Capture",
                                                    PW_KEY_STREAM_CAPTURE_SINK, capture_sink, NULL);

    PW_DLOG("pipewire_capture_create\n");
    rec->pw_stream = rec->m_pipewire_wrapper->stream_new(props, &rec->pw_stream_listener,
                                                         &stream_callbacks, rec);

    rec->pw_registry_proxy = rec->m_pipewire_wrapper->add_registry_listener(true, &rec->pw_registry_listener, &registry_events_enum, rec);

    rec->pipewire_start_streaming(pw_global_node_id);

    return rec;
  }


  void pw_recorder::pipewire_capture_destroy(void *data) {

    pw_recorder *rec = static_cast<pw_recorder*>(data);

    if (rec->pw_target_name)
      free((void *)rec->pw_target_name);

  
    if (rec->pw_stream) {
      spa_hook_remove(&rec->pw_stream_listener);
      rec->m_pipewire_wrapper->stream_destroy(rec->pw_stream);
    }

    spa_hook_remove(&rec->pw_registry_listener);
    rec->m_pipewire_wrapper->proxy_destroy((struct pw_proxy*)rec->pw_registry_proxy);

    pw_wrapper *pww = rec->m_pipewire_wrapper;

    free(rec);

    pww->unref();

    free(pww);

    PW_DLOG("pipewire_capture_destroy\n");
  }
}

