#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <gdk/gdkx.h>  // Add this line for GDK_WINDOW_XID

typedef struct {
    GtkWidget *main_window;
    GtkWidget *video_widget;
    GstElement *pipeline;
    GstElement *video_sink;
} AppData;

static gboolean bus_callback(GstBus *bus, GstMessage *message, gpointer data) {
    AppData *app_data = static_cast<AppData *>(data);

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
    GstElement *video_sink = gst_element_factory_make("xvimagesink", "video_sink");

    if (!pipeline || !source || !video_sink) {
        g_error("Failed to create GStreamer elements.");
        return;
    }

    g_object_set(G_OBJECT(source), "device", "/dev/video0", NULL);

    gst_bin_add_many(GST_BIN(pipeline), source, video_sink, NULL);

    if (!gst_element_link(source, video_sink)) {
        g_error("Failed to link GStreamer elements.");
        gst_object_unref(pipeline);
        return;
    }

    app_data->pipeline = pipeline;
    app_data->video_sink = video_sink;
}

static void on_start_button_clicked(GtkWidget *widget, gpointer user_data) {
    AppData *app_data = static_cast<AppData *>(user_data);
    start_pipeline(app_data);
}

static void on_stop_button_clicked(GtkWidget *widget, gpointer user_data) {
    AppData *app_data = static_cast<AppData *>(user_data);
    gst_element_set_state(app_data->pipeline, GST_STATE_NULL);
}

static void setup_gui(AppData *app_data) {
    app_data->main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app_data->main_window), "Webcam Viewer");
    gtk_window_set_default_size(GTK_WINDOW(app_data->main_window), 640, 480);
    g_signal_connect(G_OBJECT(app_data->main_window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    app_data->video_widget = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(app_data->main_window), app_data->video_widget);

    GtkWidget *start_button = gtk_button_new_with_label("Start");
    GtkWidget *stop_button = gtk_button_new_with_label("Stop");
    GtkWidget *exit_button = gtk_button_new_with_label("Exit");

    g_signal_connect(G_OBJECT(start_button), "clicked", G_CALLBACK(on_start_button_clicked), app_data);
    g_signal_connect(G_OBJECT(stop_button), "clicked", G_CALLBACK(on_stop_button_clicked), app_data);
    g_signal_connect(G_OBJECT(exit_button), "clicked", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_box), GTK_BUTTONBOX_CENTER);
    gtk_container_add(GTK_CONTAINER(button_box), start_button);
    gtk_container_add(GTK_CONTAINER(button_box), stop_button);
    gtk_container_add(GTK_CONTAINER(button_box), exit_button);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(main_box), app_data->video_widget, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), button_box, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(app_data->main_window), main_box);

    gtk_widget_show_all(app_data->main_window);

    GstBus *bus = gst_element_get_bus(app_data->pipeline);
    gst_bus_add_watch(bus, (GstBusFunc)bus_callback, app_data);
    gst_object_unref(bus);

    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(app_data->video_sink),
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

    gtk_main();

    gst_element_set_state(app_data.pipeline, GST_STATE_NULL);
    gst_object_unref(app_data.pipeline);

    return 0;
}
