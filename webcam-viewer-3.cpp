#include <gst/gst.h>
#include <glib-unix.h>
#include <signal.h>

#define HOST "127.0.0.1"

gboolean signal_handler(gpointer user_data) {
    GMainLoop *loop = (GMainLoop *)user_data;

    g_print("Closing the main loop\n");
    g_main_loop_quit(loop);

    return TRUE;
}

static gboolean bus_callback(GstBus *bus, GstMessage *message, gpointer data) {
    GMainLoop *loop = (GMainLoop *)data;

    switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_ERROR: {
            GError *err;
            gchar *debug;

            gst_message_parse_error(message, &err, &debug);
            g_printerr("Error: %s\nDebug details: %s\n", err->message, debug ? debug : "none");
            g_error_free(err);
            g_free(debug);

            g_main_loop_quit(loop);
            break;
        }
        case GST_MESSAGE_EOS:
            g_print("End of Stream reached\n");
            g_main_loop_quit(loop);
            break;
        default:
            break;
    }

    return TRUE;
}

int main(gint argc, gchar *argv[]) {
    GstElement *pipeline = NULL, *src = NULL, *videoconvert = NULL, *videoscale = NULL, *encoder = NULL,
               *capsfilter = NULL, *muxer = NULL, *sink = NULL;
    GstCaps *caps = NULL;
    GMainLoop *loop;
    GstBus *bus;

    gst_init(&argc, &argv);

    pipeline = gst_pipeline_new("pipeline");

    src = gst_element_factory_make("autovideosrc", "autovideosrc");
    // ... (Check and handle errors for other elements similarly)

    if (!pipeline || !src || !videoconvert || !videoscale || !capsfilter || !encoder || !muxer || !sink) {
        g_printerr("Failed to create GStreamer elements\n");
        goto cleanup;
    }

    // ... (Element creation and linking)

    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, bus_callback, loop);
    gst_object_unref(bus);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    g_print("Pipeline playing\n");

    loop = g_main_loop_new(NULL, FALSE);
    g_unix_signal_add(SIGINT, signal_handler, loop);
    g_main_loop_run(loop);

cleanup:
    gst_element_set_state(pipeline, GST_STATE_NULL);
    g_print("Closed successfully\n");

    if (pipeline != NULL) {
        gst_object_unref(pipeline);
    }
    gst_deinit();

    return 0;
}
