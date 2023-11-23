#include <gtk/gtk.h>

#include <gst/gst.h>

#include <gst/video/videooverlay.h>

#include <gdk/gdkx.h>  // Add this line for GDK_WINDOW_XID



typedef struct {

    GtkWidget *main_window;

    GtkWidget *video_widget;

    GstElement *pipeline;

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



static void initialize_pipeline(AppData *app_data) {

    GstElement *pipeline = gst_pipeline_new("webcam_pipeline");

    GstElement *source = gst_element_factory_make("v4l2src", "webcam_source");

    GstElement *sink = gst_element_factory_make("autovideosink", "video_sink");



    if (!pipeline || !source || !sink) {

        g_error("Failed to create GStreamer elements.");

        return;

    }



    g_object_set(G_OBJECT(source), "device", "/dev/video0", NULL);



    gst_bin_add_many(GST_BIN(pipeline), source, sink, NULL);



    if (!gst_element_link(source, sink)) {

        g_error("Failed to link GStreamer elements.");

        gst_object_unref(pipeline);

        return;

    }



    app_data->pipeline = pipeline;

    app_data->video_overlay = gst_element_factory_make("gtksink", "video_overlay");



    if (!app_data->video_overlay) {

        g_error("Failed to create video overlay element.");

        gst_object_unref(pipeline);

        return;

    }



    gst_bin_add(GST_BIN(pipeline), app_data->video_overlay);

    gst_element_sync_state_with_parent(app_data->video_overlay);

}



static void setup_gui(AppData *app_data) {

    app_data->main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(app_data->main_window), "Webcam Viewer");

    gtk_window_set_default_size(GTK_WINDOW(app_data->main_window), 640, 480);

    g_signal_connect(G_OBJECT(app_data->main_window), "destroy", G_CALLBACK(gtk_main_quit), NULL);



    app_data->video_widget = gtk_drawing_area_new();

    gtk_container_add(GTK_CONTAINER(app_data->main_window), app_data->video_widget);



    gtk_widget_show_all(app_data->main_window);



    GstBus *bus = gst_element_get_bus(app_data->pipeline);

    gst_bus_add_watch(bus, (GstBusFunc)bus_callback, app_data);

    gst_object_unref(bus);



    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(app_data->video_overlay),

                                        GDK_WINDOW_XID(gtk_widget_get_window(app_data->video_widget)));

}



static void start_pipeline(AppData *app_data) {

    gst_element_set_state(app_data->pipeline, GST_STATE_PLAYING);

}



int main(int argc, char *argv[]) {

    gtk_init(&argc, &argv);

    gst_init(&argc, &argv);



    AppData app_data;

    initialize_pipeline(&app_data);

    setup_gui(&app_data);

    start_pipeline(&app_data);



    gtk_main();



    gst_element_set_state(app_data.pipeline, GST_STATE_NULL);

    gst_object_unref(app_data.pipeline);



    return 0;

}

