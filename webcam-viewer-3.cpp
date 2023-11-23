#include <gst/gst.h>
#include <glib-unix.h>

#define HOST "127.0.0.1"

gboolean signal_handler(gpointer user_data)
{
  GMainLoop *loop = (GMainLoop *)user_data;

  g_print("Closing the main loop\n");
  g_main_loop_quit(loop);

  return G_SOURCE_CONTINUE;
}

int main(gint argc, gchar *argv[])
{
  GstElement *pipeline, *src, *videoconvert, *videoscale, *encoder, *capsfilter, *muxer, *sink;
  GstCaps *caps;
  GMainLoop *loop;
  gint ret = -1;

  gst_init(&argc, &argv);

  pipeline = gst_pipeline_new("pipeline");

  src = gst_element_factory_make("autovideosrc", "autovideosrc");
  videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
  videoscale = gst_element_factory_make("videoscale", "videoscale");
  capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
  encoder = gst_element_factory_make("theoraenc", "theoraenc");
  muxer = gst_element_factory_make("oggmux", "oggmux");
  sink = gst_element_factory_make("tcpserversink", "tcpserversink");

  if (!pipeline || !src || !videoconvert || !videoscale || !capsfilter || !encoder || !muxer || !sink)
  {
    g_printerr("Failed to create elements\n");
    return -1;
  }

  caps = gst_caps_from_string("video/x-raw,width=640,height=480");
  g_object_set(capsfilter, "caps", caps, NULL);
  gst_caps_unref(caps);

  g_object_set(sink, "host", HOST, NULL);

  gst_bin_add_many(GST_BIN(pipeline), src, videoconvert, videoscale, capsfilter, encoder, muxer, sink, NULL);
  if (!gst_element_link_many(src, videoconvert, videoscale, capsfilter, encoder, muxer, sink, NULL))
  {
    g_printerr("Failed to link elements\n");
    return -1;
  }

  gst_element_set_state(pipeline, GST_STATE_PLAYING);
  g_print("Pipeline playing\n");

  loop = g_main_loop_new(NULL, FALSE);
  g_unix_signal_add(SIGINT, signal_handler, loop);

  g_print("Running the loop\n");
  g_main_loop_run(loop);

  g_print("Loop finished\n");

  gst_element_set_state(pipeline, GST_STATE_NULL);
  g_print("Closed successfully\n");

  g_main_loop_unref(loop);

  gst_object_unref(pipeline);
  gst_deinit();

  return ret;
}
