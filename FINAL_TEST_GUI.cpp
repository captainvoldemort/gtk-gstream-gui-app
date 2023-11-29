#include <gtk/gtk.h>

#include <iostream>

#include <cstdlib>

#include <ctime>

#include <gst/gst.h>

#include <gst/video/videooverlay.h>

#include <gdk/gdkx.h>



typedef struct {

    GtkWidget* main_window;

    GtkWidget* speedLabel;

    GtkWidget* exitButton;

    GtkWidget* cameraButton;

    GtkWidget* backButton;

    GtkWidget* videoWidget;

    GstElement* pipeline;

    GstElement* videoSink;

    const gchar* selectedDevice;  // Add this line

} AppData;



// Function declarations

void destroyWidgets(AppData* app_data);

int getRandomSpeed();

void updateSpeedometer(GtkLabel* speedLabel);

void exitProgram(GtkWidget* widget, gpointer data);

void switchToCameraFeed(GtkWidget* widget, gpointer data);

void backToMainWindow(GtkWidget* widget, gpointer data);

void setupMainWindow(AppData* app_data);

void setupCameraFeed(AppData* app_data);

void initializeGStreamer(AppData* app_data, const gchar* device);

void setupCameraFeedForDevice(AppData* app_data, const gchar* device);

void switchToFrontCamera(GtkWidget* widget, gpointer data);

void switchToRearCamera(GtkWidget* widget, gpointer data);

void createCameraSelectionButtons(AppData* app_data);



// Function to handle GStreamer messages

static gboolean busCallback(GstBus* bus, GstMessage* message, gpointer data) {

    AppData* app_data = static_cast<AppData*>(data);



    switch (GST_MESSAGE_TYPE(message)) {

        case GST_MESSAGE_ERROR: {

            GError* error = nullptr;

            gchar* debugInfo = nullptr;

            gst_message_parse_error(message, &error, &debugInfo);

            g_critical("Error received from element %s: %s", GST_OBJECT_NAME(message->src), error->message);

            g_error_free(error);

            g_free(debugInfo);



            // Handle the error appropriately, e.g., stop the pipeline and switch back to the main window

            backToMainWindow(app_data->backButton, app_data);

            break;

        }

        default:

            break;

    }



    return TRUE;

}



// Function to set up the timer for updating the speedometer

void setupSpeedUpdateTimer(AppData* app_data) {

    g_timeout_add(1000, [](gpointer data) -> gboolean {

        AppData* app_data = static_cast<AppData*>(data);



        // Check if the label is still valid before updating

        if (GTK_IS_LABEL(app_data->speedLabel)) {

            updateSpeedometer(GTK_LABEL(app_data->speedLabel));

        }



        return G_SOURCE_CONTINUE;

    }, app_data);

}



// Function to generate random speed values for testing

int getRandomSpeed() {

    return rand() % 100; // Generate a random speed between 0 and 99

}



// Callback function for updating the speedometer display

void updateSpeedometer(GtkLabel* speedLabel) {

    int speed = getRandomSpeed();

    char speedText[5];

    sprintf(speedText, "%02d", speed); // Format the speed to always have two digits

    gtk_label_set_text(speedLabel, speedText);

}



// Callback function for exiting the program

void exitProgram(GtkWidget* widget, gpointer data) {

    gtk_main_quit();

}



// Function to create the camera feed window

void setupCameraFeedForDevice(AppData* app_data, const gchar* device) {

    // Create a new grid for the camera feed window

    GtkWidget* grid = gtk_grid_new();

    gtk_container_add(GTK_CONTAINER(app_data->main_window), grid);



    // Create the back button

    app_data->backButton = gtk_button_new_with_label("Back");

    gtk_widget_set_name(app_data->backButton, "exit-button");

    g_signal_connect(G_OBJECT(app_data->backButton), "clicked", G_CALLBACK(backToMainWindow), app_data);



    // Load CSS for styling

    GtkCssProvider* cssProvider = gtk_css_provider_new();

    if (gtk_css_provider_load_from_path(cssProvider, "styles.css", NULL)) {

        GtkStyleContext* styleContextBackButton = gtk_widget_get_style_context(app_data->backButton);

        gtk_style_context_add_provider(styleContextBackButton, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);



        // GStreamer initialization

        initializeGStreamer(app_data, device);



        // Drawing area for video feed

        app_data->videoWidget = gtk_drawing_area_new();

        gtk_widget_set_size_request(app_data->videoWidget, 640, 480);



        // Set up the grid to arrange the video feed and back button

        gtk_grid_attach(GTK_GRID(grid), app_data->videoWidget, 0, 0, 2, 1);

        gtk_grid_attach(GTK_GRID(grid), app_data->backButton, 0, 1, 2, 1);



        // Show all widgets

        gtk_widget_show_all(app_data->main_window);



        // Start the GStreamer pipeline

        gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(app_data->videoSink),

                                            GDK_WINDOW_XID(gtk_widget_get_window(app_data->videoWidget)));

        gst_element_set_state(app_data->pipeline, GST_STATE_PLAYING);

    } else {

        g_warning("Failed to load CSS file.");

    }

}





// Callback function for switching to the front camera feed window

void switchToFrontCamera(GtkWidget* widget, gpointer data) {

    AppData* app_data = static_cast<AppData*>(data);

    app_data->selectedDevice = "/dev/video0";



    // Destroy existing widgets

    destroyWidgets(app_data);



    // Setup camera feed for the selected device

    setupCameraFeedForDevice(app_data, app_data->selectedDevice);

}



// Callback function for switching to the rear camera feed window

void switchToRearCamera(GtkWidget* widget, gpointer data) {

    AppData* app_data = static_cast<AppData*>(data);

    app_data->selectedDevice = "/dev/video1";



    // Destroy existing widgets

    destroyWidgets(app_data);



    // Setup camera feed for the selected device

    setupCameraFeedForDevice(app_data, app_data->selectedDevice);

}



// Function to destroy existing widgets in the main window

void destroyWidgets(AppData* app_data) {

    GList* children, * iter;

    children = gtk_container_get_children(GTK_CONTAINER(app_data->main_window));

    for (iter = children; iter != NULL; iter = g_list_next(iter)) {

        gtk_widget_destroy(GTK_WIDGET(iter->data));

    }

    g_list_free(children);

}



// Function to create the camera selection buttons

void createCameraSelectionButtons(AppData* app_data) {

    // Create the front camera button

    GtkWidget* frontCamButton = gtk_button_new_with_label("Front Cam");

    gtk_widget_set_name(frontCamButton, "exit-button");

    g_signal_connect(G_OBJECT(frontCamButton), "clicked", G_CALLBACK(switchToFrontCamera), app_data);



    // Create the rear camera button

    GtkWidget* rearCamButton = gtk_button_new_with_label("Rear Cam");

    gtk_widget_set_name(rearCamButton, "exit-button");

    g_signal_connect(G_OBJECT(rearCamButton), "clicked", G_CALLBACK(switchToRearCamera), app_data);

    

    GtkWidget* backButton = gtk_button_new_with_label("Back");

    gtk_widget_set_name(backButton, "exit-button");

    g_signal_connect(G_OBJECT(backButton), "clicked", G_CALLBACK(backToMainWindow), app_data);



    // Set up the grid to arrange the camera selection buttons

    GtkWidget* grid = gtk_grid_new();

    gtk_grid_attach(GTK_GRID(grid), frontCamButton, 0, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), rearCamButton, 1, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), backButton, 0, 1, 2, 1);



    // Load CSS for styling

    GtkCssProvider* cssProvider = gtk_css_provider_new();

    gtk_css_provider_load_from_path(cssProvider, "styles.css", NULL);



    // Apply the CSS to the buttons in the grid

    GtkStyleContext* styleContextFrontButton = gtk_widget_get_style_context(frontCamButton);

    GtkStyleContext* styleContextRearButton = gtk_widget_get_style_context(rearCamButton);

    GtkStyleContext* styleContextBackButton = gtk_widget_get_style_context(backButton);



    gtk_style_context_add_provider(styleContextFrontButton, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_style_context_add_provider(styleContextRearButton, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_style_context_add_provider(styleContextBackButton, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);



    // Center the grid in the window

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(app_data->main_window), vbox);



    // Show all widgets

    gtk_widget_show_all(app_data->main_window);

}



// Function to switch to the camera feed window

void switchToCameraFeed(GtkWidget* widget, gpointer data) {

    AppData* app_data = static_cast<AppData*>(data);



    // Remove all widgets from the main window

    GList* children, * iter;

    children = gtk_container_get_children(GTK_CONTAINER(app_data->main_window));

    for (iter = children; iter != NULL; iter = g_list_next(iter)) {

        gtk_widget_destroy(GTK_WIDGET(iter->data));

    }

    g_list_free(children);



    // Setup the camera selection buttons

    createCameraSelectionButtons(app_data);

}





void backToMainWindow(GtkWidget* widget, gpointer data) {

    AppData* app_data = static_cast<AppData*>(data);



    g_message("Back button clicked. Stopping GStreamer pipeline.");



    // Stop the GStreamer pipeline and release resources

    gst_element_set_state(app_data->pipeline, GST_STATE_NULL);

    gst_object_unref(app_data->pipeline);



    g_message("GStreamer pipeline stopped. Destroying widgets.");



    // Remove all widgets from the main window

    GList* children, * iter;

    children = gtk_container_get_children(GTK_CONTAINER(app_data->main_window));



    for (iter = children; iter != NULL; iter = g_list_next(iter)) {

        gtk_widget_destroy(GTK_WIDGET(iter->data));

    }



    g_list_free(children);



    g_message("Widgets destroyed. Setting up the main window.");



    // Setup the main window

    setupMainWindow(app_data);



    g_message("Main window setup complete.");

}





// Function to create the main window

void setupMainWindow(AppData* app_data) {

    //Create main window label

    gtk_widget_set_name(app_data->main_window, "main-window"); // Set the style class name for the label

    

    // Create the speedometer label

    app_data->speedLabel = gtk_label_new("00");

    gtk_widget_set_name(app_data->speedLabel, "speed-label"); // Set the style class name for the label

    gtk_label_set_use_markup(GTK_LABEL(app_data->speedLabel), TRUE);

    gtk_widget_set_halign(app_data->speedLabel, GTK_ALIGN_CENTER);

    gtk_widget_set_valign(app_data->speedLabel, GTK_ALIGN_CENTER);



    // Create the exit button

    app_data->exitButton = gtk_button_new_with_label("Exit");

    gtk_widget_set_name(app_data->exitButton, "exit-button"); // Set the style class name for the button

    g_signal_connect(G_OBJECT(app_data->exitButton), "clicked", G_CALLBACK(exitProgram), NULL);



    // Create the camera feed button

    app_data->cameraButton = gtk_button_new_with_label("View Camera Feed");

    gtk_widget_set_name(app_data->cameraButton, "exit-button"); // Set the same style as the exit button

    g_signal_connect(G_OBJECT(app_data->cameraButton), "clicked", G_CALLBACK(switchToCameraFeed), app_data);



    // Set up the timer to update the speedometer with random values

    g_timeout_add(1000, [](gpointer data) -> gboolean {

    AppData* app_data = static_cast<AppData*>(data);



    // Check if the label is still valid before updating

    if (GTK_IS_LABEL(app_data->speedLabel)) {

        updateSpeedometer(GTK_LABEL(app_data->speedLabel));

    }



    return G_SOURCE_CONTINUE;

    }, &app_data);



    // Load CSS for styling

    GtkCssProvider* cssProvider = gtk_css_provider_new();

    gtk_css_provider_load_from_path(cssProvider, "styles.css", NULL);



    // Apply the CSS to the window, label, and buttons

    GtkStyleContext* styleContextWindow = gtk_widget_get_style_context(app_data->main_window);

    GtkStyleContext* styleContextLabel = gtk_widget_get_style_context(app_data->speedLabel);

    GtkStyleContext* styleContextExitButton = gtk_widget_get_style_context(app_data->exitButton);

    GtkStyleContext* styleContextCameraButton = gtk_widget_get_style_context(app_data->cameraButton);



    gtk_style_context_add_provider(styleContextWindow, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_style_context_add_provider(styleContextLabel, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_style_context_add_provider(styleContextExitButton, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_style_context_add_provider(styleContextCameraButton, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);



    // Create a vertical box to arrange the label and buttons

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5); // 5 is the spacing between child widgets

    gtk_box_pack_start(GTK_BOX(vbox), app_data->speedLabel, TRUE, TRUE, 0);

    gtk_box_pack_end(GTK_BOX(vbox), app_data->exitButton, FALSE, FALSE, 0);

    gtk_box_pack_end(GTK_BOX(vbox), app_data->cameraButton, FALSE, FALSE, 0);



    // Add the vertical box to the main window

    gtk_container_add(GTK_CONTAINER(app_data->main_window), vbox);



    // Show all widgets

    gtk_widget_show_all(app_data->main_window);

    

    // Set up the timer to update the speedometer with random values

    setupSpeedUpdateTimer(app_data);

}



// Function to create the camera feed window

void setupCameraFeed(AppData* app_data) {

    // Create the back button

    app_data->backButton = gtk_button_new_with_label("Back");

    gtk_widget_set_name(app_data->backButton, "exit-button"); // Set the same style as the exit button

    g_signal_connect(G_OBJECT(app_data->backButton), "clicked", G_CALLBACK(backToMainWindow), app_data);

    

    // Load CSS for styling

    GtkCssProvider* cssProvider = gtk_css_provider_new();

    gtk_css_provider_load_from_path(cssProvider, "styles.css", NULL);

    

    GtkStyleContext* styleContextBackButton = gtk_widget_get_style_context(app_data->backButton);

    gtk_style_context_add_provider(styleContextBackButton, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    

    // GStreamer initialization

    initializeGStreamer(app_data, app_data->selectedDevice); 



    // Drawing area for video feed

    app_data->videoWidget = gtk_drawing_area_new();

    gtk_widget_set_size_request(app_data->videoWidget, 640, 480);



    // Set up the grid to arrange the video feed and back button

    GtkWidget* grid = gtk_grid_new();

    gtk_grid_attach(GTK_GRID(grid), app_data->videoWidget, 0, 0, 2, 1);

    gtk_grid_attach(GTK_GRID(grid), app_data->backButton, 0, 1, 2, 1);



    // Add the grid to the camera feed window

    gtk_container_add(GTK_CONTAINER(app_data->main_window), grid);



    // Show all widgets

    gtk_widget_show_all(app_data->main_window);



    // Start the GStreamer pipeline

    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(app_data->videoSink),

                                        GDK_WINDOW_XID(gtk_widget_get_window(app_data->videoWidget)));

    gst_element_set_state(app_data->pipeline, GST_STATE_PLAYING);

}



// Function to initialize GStreamer pipeline

void initializeGStreamer(AppData* app_data, const gchar* device) {

    GstElement* pipeline = gst_pipeline_new("webcam_pipeline");

    GstElement* source = gst_element_factory_make("v4l2src", "webcam_source");

    app_data->videoSink = gst_element_factory_make("xvimagesink", "video_sink");



    if (!pipeline || !source || !app_data->videoSink) {

        g_error("Failed to create GStreamer elements.");

        return;

    }



    g_object_set(G_OBJECT(source), "device", device, NULL);



    gst_bin_add_many(GST_BIN(pipeline), source, app_data->videoSink, NULL);



    if (!gst_element_link(source, app_data->videoSink)) {

        g_error("Failed to link GStreamer elements.");

        gst_object_unref(pipeline);

        return;

    }



    app_data->pipeline = pipeline;

    

    // Get the bus for the pipeline and add a watch for messages

    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(app_data->pipeline));

    gst_bus_add_watch(bus, busCallback, app_data);

    gst_object_unref(bus);

}





int main(int argc, char* argv[]) {

    gtk_init(&argc, &argv);

    gst_init(&argc, &argv);



    AppData app_data;

    app_data.main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(app_data.main_window), "Digital Speedometer");

    gtk_window_set_default_size(GTK_WINDOW(app_data.main_window), 800, 600);

    gtk_window_fullscreen(GTK_WINDOW(app_data.main_window));

    gtk_window_set_position(GTK_WINDOW(app_data.main_window), GTK_WIN_POS_CENTER);

    //gtk_window_set_resizable(GTK_WINDOW(app_data.main_window), FALSE);

    g_signal_connect(G_OBJECT(app_data.main_window), "destroy", G_CALLBACK(gtk_main_quit), NULL);



    // Initialize the selectedDevice member

    app_data.selectedDevice = nullptr;

    

    // Setup the main window

    setupMainWindow(&app_data);



    gtk_main();



    // Clean up GStreamer pipeline

    gst_element_set_state(app_data.pipeline, GST_STATE_NULL);

    gst_object_unref(app_data.pipeline);



    return 0;

}

