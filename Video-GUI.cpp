// required header for gtk window
#include<gtk/gtk.h>
#include <gdk/gdk.h>
#if defined (GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
#endif
// required header for gstreamer
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
// video window handle
static guintptr video_window_handle = 0;
GtkWidget *video_window;
GtkWidget *main_window;
GtkWidget *hbox;
GtkWidget *vbox;
GtkWidget *control;
GtkWidget *Play;
GtkWidget *Pause;
GtkWidget *Open;
GstElement *pipeline, *src, *sink;
GstBus *bus;
static void video_widget_realize_cb (GtkWidget * widget, gpointer data);
static GstBusSyncReply bus_sync_handler (GstBus * bus, GstMessage * message,
                     gpointer user_data);
/* This function is called when the PLAY button is clicked */
static void
play_cb (GtkButton * button, GstElement * pipeline)
{
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
}
/* This function is called when the PAUSE button is clicked */
static void
pause_cb (GtkButton * button, GstElement * pipeline)
{
  gst_element_set_state (pipeline, GST_STATE_PAUSED);
}
/* This function is called when the PAUSE button is clicked */
static void
open_cb (GtkButton * button, GstElement * pipeline)
{
  gst_element_set_state (pipeline, GST_STATE_READY);
  GtkWidget *dialog;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
  gint res;
  dialog = gtk_file_chooser_dialog_new ("Open File",
                    NULL,
                    action,
                    "Cancel",
                    GTK_RESPONSE_CANCEL,
                    "_Open", GTK_RESPONSE_ACCEPT, NULL);
  res = gtk_dialog_run (GTK_DIALOG (dialog));
  if (res == GTK_RESPONSE_ACCEPT)
    {
      char *filename;
      char video_loc[100] = "file://";
      GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
      filename = gtk_file_chooser_get_filename (chooser);
      strcat (video_loc, filename);
      g_object_set (G_OBJECT (pipeline), "uri", video_loc, NULL);
      gst_element_set_state (pipeline, GST_STATE_PLAYING);
      g_free (filename);
    }
  gtk_widget_destroy (dialog);
}
int
main (int argc, char *argv[])
{
  // Initialize GStreamer
  gst_init (&argc, &argv);
  // init gtk library 
  gtk_init (&argc, &argv);
  pipeline = gst_element_factory_make ("playbin", "play");
  g_object_set (G_OBJECT (pipeline), "uri",
        "http://docs.gstreamer.com/media/sintel_trailer-480p.webm",
        NULL);
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_element_set_state (pipeline, GST_STATE_READY);
  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (main_window), 800, 600);
  g_signal_connect (main_window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
  video_window = gtk_drawing_area_new ();
  g_signal_connect (video_window, "realize",
            G_CALLBACK (video_widget_realize_cb), NULL);
  g_object_set(video_window, "double-buffered", FALSE, NULL);
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  control = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  // add video_window in hbox
  gtk_box_pack_start (GTK_BOX (hbox), video_window, TRUE, TRUE, 0);
  // create buttons
  Play = gtk_button_new_with_label ("Play");
  g_signal_connect (G_OBJECT (Play), "clicked", G_CALLBACK (play_cb),
            pipeline);
  Pause = gtk_button_new_with_label ("Pause");
  g_signal_connect (G_OBJECT (Pause), "clicked", G_CALLBACK (pause_cb),
            pipeline);
  Open = gtk_button_new_with_label ("Open");
  g_signal_connect (G_OBJECT (Open), "clicked", G_CALLBACK (open_cb),
            pipeline);
  // add button box to hbox_controller
  gtk_box_pack_start (GTK_BOX (control), Play, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (control), Pause, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (control), Open, TRUE, TRUE, 0);
  // add vbox to main window
  gtk_container_add (GTK_CONTAINER (main_window), vbox);
  // add hbox containing video_window to vbox
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), control, FALSE, FALSE, 0);
  gtk_widget_show_all (main_window);
  gtk_widget_realize (video_window);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  // set up sync handler for setting the xid once the pipeline is started
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_set_sync_handler (bus, (GstBusSyncHandler) bus_sync_handler, NULL,
                NULL);
  gst_object_unref (bus);
  // run main loop
  gtk_main ();
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
  return 0;
}
static void
video_widget_realize_cb (GtkWidget * widget, gpointer data)
{
#ifdef GDK_WINDOWING_X11
  {
    gulong xid = GDK_WINDOW_XID (gtk_widget_get_window (video_window));
    video_window_handle = xid;
  }
#endif
}
static GstBusSyncReply
bus_sync_handler (GstBus * bus, GstMessage * message, gpointer user_data)
{
  // ignore anything but 'prepare-window-handle' element messages
  if (!gst_is_video_overlay_prepare_window_handle_message (message))
    return GST_BUS_PASS;
  if (video_window_handle != 0)
    {
      GstVideoOverlay *overlay;
      // GST_MESSAGE_SRC (message) will be the video sink element
      overlay = GST_VIDEO_OVERLAY (GST_MESSAGE_SRC (message));
      gst_video_overlay_set_window_handle (overlay, video_window_handle);
    }
  else
    {
      g_warning ("Should have obtained video_window_handle by now!");
    }
}
