#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>

// Structure to hold application data
typedef struct {
  GtkWidget *main_window; // Main application window
  GtkWidget *video_widget; // Widget to hold the video overlay
  GstElement *pipeline; // GStreamer pipeline
} AppData;

// Function to initialize GStreamer pipeline
static void initialize_pipeline(AppData *app_data) {
  // Create GStreamer elements
  GstElement *pipeline = gst_pipeline_new("webcam_pipeline");
  GstElement *source = gst_element_factory_make("v4l2src", "webcam_source");
  GstElement *sink = gst_element_factory_make("autovideosink", "video_sink");

  // Check if elements are created successfully
  if (!pipeline || !source || !sink) {
    g_error("Failed to create GStreamer elements.");
    return;
  }

  // Set device for v4l2src (adjust the device path as needed)
  g_object_set(G_OBJECT(source), "device", "/dev/video0", NULL);

  // Add elements to the pipeline
  gst_bin_add_many(GST_BIN(pipeline), source, sink, NULL);

  // Link elements
  if (!gst_element_link(source, sink)) {
    g_error("Failed to link GStreamer elements.");
    gst_object_unref(pipeline);
    return;
  }

  // Set the pipeline to the application data structure
  app_data->pipeline = pipeline;
}

// Function to set up the GTK window and video overlay
static void setup_gui(AppData *app_data) {
  // Create the main window
  app_data->main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(app_data->main_window), "Webcam Viewer");
  gtk_window_set_default_size(GTK_WINDOW(app_data->main_window), 640, 480);
  g_signal_connect(G_OBJECT(app_data->main_window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

  // Create a drawing area for video overlay
  app_data->video_widget = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(app_data->main_window), app_data->video_widget);

  // Set up the video overlay
  gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(app_data->pipeline),
                                      GDK_WINDOW_XID(gtk_widget_get_window(app_data->video_widget)));

  // Show all widgets
  gtk_widget_show_all(app_data->main_window);
}

// Function to start the GStreamer pipeline
static void start_pipeline(AppData *app_data) {
  // Set the pipeline to the playing state
  gst_element_set_state(app_data->pipeline, GST_STATE_PLAYING);
}

int main(int argc, char *argv[]) {
  // Initialize GTK and GStreamer
  gtk_init(&argc, &argv);
  gst_init(&argc, &argv);

  // Create application data structure
  AppData app_data;

  // Initialize GStreamer pipeline
  initialize_pipeline(&app_data);

  // Set up the GTK window and video overlay
  setup_gui(&app_data);

  // Start the GStreamer pipeline
  start_pipeline(&app_data);

  // Run the GTK main loop
  gtk_main();

  // Clean up resources
  gst_element_set_state(app_data.pipeline, GST_STATE_NULL);
  gst_object_unref(app_data.pipeline);

  return 0;
}
