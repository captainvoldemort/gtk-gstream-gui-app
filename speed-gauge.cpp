#include <gtk/gtk.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

// Function to generate random speed values for testing
double getRandomSpeed() {
    return rand() % 100 + 1; // Generate a random speed between 1 and 100
}

// Callback function for the slider value change
void on_slider_changed(GtkWidget* slider, gpointer data) {
    double value = gtk_range_get_value(GTK_RANGE(slider));

    // Update the speed gauge value
    GtkLabel* speedLabel = GTK_LABEL(data);
    char speedText[50];
    sprintf(speedText, "Speed: %.2f km/h", value);
    gtk_label_set_text(speedLabel, speedText);
}

int main(int argc, char** argv) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Create the main window
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Speed Gauge");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_widget_set_size_request(window, 400, 200);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create a vertical box to hold the components
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create the speed gauge label
    GtkLabel* speedLabel = GTK_LABEL(gtk_label_new("Speed: 0.00 km/h"));
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(speedLabel), FALSE, FALSE, 0);

    // Create the drawing area for the speed gauge
    GtkWidget* drawingArea = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), drawingArea, TRUE, TRUE, 0);

    // Create the slider for testing
    GtkWidget* slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_box_pack_start(GTK_BOX(vbox), slider, FALSE, FALSE, 0);
    g_signal_connect(slider, "value-changed", G_CALLBACK(on_slider_changed), speedLabel);

    // Set up the random seed for testing
    srand(time(NULL));

    // Set up the timer to update the speed gauge with random values
    g_timeout_add(1000, [slider](gpointer data) -> gboolean {
        double speed = getRandomSpeed();
        char speedText[50];
        GtkLabel* speedLabel = GTK_LABEL(data);
        sprintf(speedText, "Speed: %.2f km/h", speed);
        gtk_label_set_text(speedLabel, speedText);
        gtk_range_set_value(GTK_RANGE(slider), speed);
        return G_SOURCE_CONTINUE;
    }, speedLabel);

    // Show all components
    gtk_widget_show_all(window);

    // Start the GTK main loop
    gtk_main();

    return 0;
}
