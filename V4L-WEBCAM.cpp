#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <gdk/gdkx.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

typedef struct {
    GtkWidget *main_window;
    GtkWidget *video_widget;
    int fd; // File descriptor for the V4L2 device
} AppData;

#define WIDTH 640
#define HEIGHT 480

static void initialize_v4l2_device(AppData *app_data) {
    const char *device_path = "/dev/video0";

    app_data->fd = open(device_path, O_RDWR | O_NONBLOCK, 0);
    if (app_data->fd == -1) {
        g_error("Error opening V4L2 device %s", device_path);
        return;
    }

    struct v4l2_format fmt = {0};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(app_data->fd, VIDIOC_S_FMT, &fmt) == -1) {
        g_error("Error setting pixel format");
        close(app_data->fd);
        return;
    }
}

static gboolean bus_callback(GstBus *bus, GstMessage *message, gpointer data) {
    AppData *app_data = (AppData *)data;

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

static void setup_gui(AppData *app_data) {
    app_data->main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app_data->main_window), "V4L2 GTK Viewer");
    gtk_window_set_default_size(GTK_WINDOW(app_data->main_window), WIDTH, HEIGHT);
    g_signal_connect(G_OBJECT(app_data->main_window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    app_data->video_widget = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(app_data->main_window), app_data->video_widget);

    gtk_widget_show_all(app_data->main_window);
}

static void start_pipeline(AppData *app_data) {
    GstElement *pipeline, *v4l2src, *jpegdec, *gtksink;
    GstBus *bus;

    // Initialize GStreamer
    if (!gst_init_check(NULL, NULL)) {
        g_error("Failed to initialize GStreamer.");
        return;
    }

    // Initialize GTK
    if (!gdk_init_check(NULL, NULL)) {
        g_error("Failed to initialize GTK.");
        return;
    }

    pipeline = gst_pipeline_new("v4l2_pipeline");
    v4l2src = gst_element_factory_make("v4l2src", "v4l2src");
    jpegdec = gst_element_factory_make("jpegdec", "jpegdec");
    gtksink = gst_element_factory_make("gtksink", "gtksink");

    if (!pipeline || !v4l2src || !jpegdec || !gtksink) {
        g_error("Failed to create GStreamer elements.");
        return;
    }

    // Check if file descriptor is valid
    if (app_data->fd == -1) {
        g_error("File descriptor is not valid.");
        return;
    }

    g_object_set(G_OBJECT(v4l2src), "device", app_data->fd, NULL);

    gst_bin_add_many(GST_BIN(pipeline), v4l2src, jpegdec, gtksink, NULL);
    if (!gst_element_link_many(v4l2src, jpegdec, gtksink, NULL)) {
        g_error("Failed to link GStreamer elements.");
        gst_object_unref(pipeline);
        return;
    }

    bus = gst_element_get_bus(pipeline);
    gst_bus_add_watch(bus, (GstBusFunc)bus_callback, app_data);
    gst_object_unref(bus);

    // Check if gtksink supports the video overlay interface
    if (GST_IS_VIDEO_OVERLAY(gtksink)) {
        GdkWindow *window = gtk_widget_get_window(app_data->video_widget);
        if (window != NULL) {
            gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(gtksink), GDK_WINDOW_XID(window));
        } else {
            g_warning("Failed to get GDK window.");
        }
    } else {
        g_warning("The gtksink element does not support the video overlay interface.");
    }

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

int main(int argc, char *argv[]) {
    AppData app_data;

    // Initialize V4L2 device
    initialize_v4l2_device(&app_data);

    // Setup GUI
    setup_gui(&app_data);

    // Start GStreamer pipeline
    start_pipeline(&app_data);

    // Run GTK main loop
    gtk_main();

    // Close V4L2 device
    close(app_data.fd);

    return 0;
}
