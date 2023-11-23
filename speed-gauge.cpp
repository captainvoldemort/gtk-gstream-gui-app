#include <gtk/gtk.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

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

    // Set color based on speed range
    const char* colorClass;
    if (speed < 30) {
        colorClass = "green-label";
    } else if (speed < 70) {
        colorClass = "orange-label";
    } else {
        colorClass = "red-label";
    }
    gtk_widget_set_name(GTK_WIDGET(speedLabel), colorClass);
}

int main(int argc, char** argv) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    // Create the main window
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Digital Speedometer");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_widget_set_size_request(window, 200, 100);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Create the speedometer label
    GtkWidget* speedLabel = gtk_label_new("00");
    gtk_label_set_use_markup(GTK_LABEL(speedLabel), TRUE);
    gtk_widget_set_halign(speedLabel, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(speedLabel, GTK_ALIGN_CENTER);

    // Set up the timer to update the speedometer with random values
    g_timeout_add(1000, [](gpointer data) -> gboolean {
        updateSpeedometer(GTK_LABEL(data));
        return G_SOURCE_CONTINUE;
    }, speedLabel);

    // Load CSS for styling
    GtkCssProvider* cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(cssProvider, R"(
        .green-label { color: green; }
        .orange-label { color: orange; }
        .red-label { color: red; }
    )", -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Show the speedometer label
    gtk_container_add(GTK_CONTAINER(window), speedLabel);
    gtk_widget_show_all(window);

    // Start the GTK main loop
    gtk_main();

    return 0;
}
