#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <gdk/gdkx.h>

typedef struct {
    GtkWidget *main_window;
    GtkWidget *video_area;
    GtkWidget *start_button;
    GtkWidget *stop_button;
    GtkWidget *exit_button;
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

static void start_pipeline(AppData *app_data) {
    gst_element_set_state(app_data->pipeline, GST_STATE_PLAYING);
}

static void stop_pipeline(AppData *app_data) {
    gst_element_set_state(app_data->pipeline, GST_STATE_NULL);
}

static void on_start_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = static_cast<AppData *>(data);
    start_pipeline(app_data);
    gtk_widget_hide(app_data->start_button);
    gtk_widget_show(app_data->stop_button);
    gtk_widget_show(app_data->exit_button);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), app_data->video_area, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), app_data->stop_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), app_data->exit_button, FALSE, FALSE, 0);

    gtk_container_remove(GTK_CONTAINER(app_data->main_window), app_data->start_button);
    gtk_container_add(GTK_CONTAINER(app_data->main_window), vbox);
}

static void on_stop_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = static_cast<AppData *>(data);
    stop_pipeline(app_data);
    gtk_widget_hide(app_data->stop_button);
    gtk_widget_hide(app_data->exit_button);
    gtk_widget_show(app_data->start_button);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), app_data->video_area, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), app_data->start_button, FALSE, FALSE, 0);

    gtk_container_remove(GTK_CONTAINER(app_data->main_window), gtk_widget_get_parent(app_data->video_area));
    gtk_container_add(GTK_CONTAINER(app_data->main_window), vbox);
}

static void on_exit_clicked(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

static void setup_gui(AppData *app_data) {
    app_data->main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app_data->main_window), "Webcam Viewer");
    gtk_window_set_default_size(GTK_WINDOW(app_data->main_window), 640, 480);
    g_signal_connect(G_OBJECT(app_data->main_window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app_data->main_window), vbox);

    app_data->video_area = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), app_data->video_area, TRUE, TRUE, 0);

    app_data->start_button = gtk_button_new_with_label("Start");
    g_signal_connect(G_OBJECT(app_data->start_button), "clicked", G_CALLBACK(on_start_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(vbox), app_data->start_button, FALSE, FALSE, 0);

    app_data->stop_button = gtk_button_new_with_label("Stop");
    g_signal_connect(G_OBJECT(app_data->stop_button), "clicked", G_CALLBACK(on_stop_clicked), app_data);
    gtk_widget_hide(app_data->stop_button);

    app_data->exit_button = gtk_button_new_with_label("Exit");
    g_signal_connect(G_OBJECT(app_data->exit_button), "clicked", G_CALLBACK(on_exit_clicked), app_data);
    gtk_widget_hide(app_data->exit_button);

    gtk_widget_show_all(app_data->main_window);
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
