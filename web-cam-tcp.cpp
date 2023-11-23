#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <gdk/gdkx.h>

typedef struct {
    GtkWidget *main_window;
    GtkWidget *video_widget;
    GstElement *webcam_pipeline;
    GstElement *stream_pipeline;
    GstElement *video_overlay;
} AppData;

static gboolean bus_callback(GstBus *bus, GstMessage *message, gpointer data) {
    AppData *app_data = static_cast<AppData*>(data);

    switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_ERROR: {
            GError *err;
            gchar *debug_info;

            gst_message_parse_error(message, &err, &debug_info);
            g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(message->src), err->message);
            g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
            g_clear_error(&err);
            g_free(debug_info);

            gtk_main_quit();
            break;
        }
        default:
            break;
    }

    return TRUE;
}

static void realize_cb(GtkWidget *widget, gpointer data) {
    AppData *app_data = static_cast<AppData*>(data);

    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(app_data->video_overlay),
                                        GDK_WINDOW_XID(gtk_widget_get_window(app_data->video_widget)));
}

static void initialize_webcam_pipeline(AppData *app_data) {
    app_data->webcam_pipeline = gst_pipeline_new("webcam_pipeline");
    GstElement *src = gst_element_factory_make("v4l2src", "webcam_source");
    GstElement *sink = gst_element_factory_make("xvimagesink", "video_sink");

    if (!app_data->webcam_pipeline || !src || !sink) {
        g_error("Failed to create GStreamer elements for webcam pipeline.");
        return;
    }

    g_object_set(G_OBJECT(src), "device", "/dev/video0", NULL);

    gst_bin_add_many(GST_BIN(app_data->webcam_pipeline), src, sink, NULL);

    if (!gst_element_link(src, sink)) {
        g_error("Failed to link GStreamer elements for webcam pipeline.");
        gst_object_unref(app_data->webcam_pipeline);
        return;
    }

    app_data->video_overlay = gst_element_factory_make("xvimagesink", "video_overlay");

    if (!app_data->video_overlay) {
        g_error("Failed to create video overlay element.");
        gst_object_unref(app_data->webcam_pipeline);
        return;
    }

    gst_bin_add(GST_BIN(app_data->webcam_pipeline), app_data->video_overlay);
    gst_element_sync_state_with_parent(app_data->video_overlay);
}

static void initialize_stream_pipeline(AppData *app_data) {
    app_data->stream_pipeline = gst_pipeline_new("stream_pipeline");
    GstElement *videosrc = gst_element_factory_make("appsrc", "videosrc");
    GstElement *encoder = gst_element_factory_make("x264enc", "encoder");
    GstElement *h264parse = gst_element_factory_make("h264parse", "h264parse");
    GstElement *rtph264pay = gst_element_factory_make("rtph264pay", "rtph264pay");
    GstElement *tcpsink = gst_element_factory_make("tcpserversink", "tcpserversink");

    if (!app_data->stream_pipeline || !videosrc || !encoder || !h264parse || !rtph264pay || !tcpsink) {
        g_error("Failed to create GStreamer elements for stream pipeline.");
        return;
    }

    gst_bin_add_many(GST_BIN(app_data->stream_pipeline), videosrc, encoder, h264parse, rtph264pay, tcpsink, NULL);
    gst_element_link_many(videosrc, encoder, h264parse, rtph264pay, tcpsink, NULL);

    // Set caps for videosrc
    GstCaps *caps = gst_caps_from_string("video/x-raw,width=640,height=480");
    g_object_set(G_OBJECT(videosrc), "caps", caps, NULL);
    gst_caps_unref(caps);

    // Set properties for tcpsink
    g_object_set(G_OBJECT(tcpsink), "host", "127.0.0.1", "port", 5000, NULL);
}

static void setup_gui(AppData *app_data) {
    app_data->main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app_data->main_window), "Webcam Viewer");
    gtk_window_set_default_size(GTK_WINDOW(app_data->main_window), 640, 480);
    g_signal_connect(G_OBJECT(app_data->main_window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    app_data->video_widget = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(app_data->main_window), app_data->video_widget);

    // Connect the "realize" signal to the realize_cb function
    g_signal_connect(G_OBJECT(app_data->video_widget), "realize", G_CALLBACK(realize_cb), app_data);

    gtk_widget_show_all(app_data->main_window);

    GstBus *bus = gst_element_get_bus(app_data->webcam_pipeline);
    gst_bus_add_watch(bus, (GstBusFunc)bus_callback, app_data);
    gst_object_unref(bus);
}

static void start_pipelines(AppData *app_data) {
    // Set the webcam pipeline to the PAUSED state initially
    gst_element_set_state(app_data->webcam_pipeline, GST_STATE_PAUSED);

    // Set the stream pipeline to the PLAYING state
    gst_element_set_state(app_data->stream_pipeline, GST_STATE_PLAYING);

    // Start the GTK main loop
    gtk_main();

    // Set both pipelines to the NULL state after the GTK main loop has started
    gst_element_set_state(app_data->webcam_pipeline, GST_STATE_NULL);
    gst_element_set_state(app_data->stream_pipeline, GST_STATE_NULL);

    g_print("Closed successfully\n");
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    gst_init(&argc, &argv);

    AppData app_data;
    initialize_webcam_pipeline(&app_data);
    initialize_stream_pipeline(&app_data);
    setup_gui(&app_data);
    start_pipelines(&app_data);

    gst_object_unref(app_data.webcam_pipeline);
    gst_object_unref(app_data.stream_pipeline);

    return 0;
}
