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

}



// Callback function for exiting the program

void exitProgram(GtkWidget* widget, gpointer data) {

    gtk_main_quit();

}



int main(int argc, char** argv) {

    // Initialize GTK

    gtk_init(&argc, &argv);



    // Create the main window

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_widget_set_name(window, "main-window"); // Set the style class name for the window

    gtk_window_set_title(GTK_WINDOW(window), "Digital Speedometer");

    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    gtk_widget_set_size_request(window, 200, 150); // Increased height to accommodate the button

    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);



    // Create the speedometer label

    GtkWidget* speedLabel = gtk_label_new("00");

    gtk_widget_set_name(speedLabel, "speed-label"); // Set the style class name for the label

    gtk_label_set_use_markup(GTK_LABEL(speedLabel), TRUE);

    gtk_widget_set_halign(speedLabel, GTK_ALIGN_CENTER);

    gtk_widget_set_valign(speedLabel, GTK_ALIGN_CENTER);

    

    // Create the exit button

    GtkWidget* exitButton = gtk_button_new_with_label("Exit");

    gtk_widget_set_name(exitButton, "exit-button"); // Set the style class name for the button

    

    

    // Set up the timer to update the speedometer with random values

    g_timeout_add(1000, [](gpointer data) -> gboolean {

        updateSpeedometer(GTK_LABEL(data));

        return G_SOURCE_CONTINUE;

    }, speedLabel);



    // Load CSS for styling

    GtkCssProvider* cssProvider = gtk_css_provider_new();

    gtk_css_provider_load_from_path(cssProvider, "styles.css", NULL);



    // Apply the CSS to the window and label

    GtkStyleContext* styleContextWindow = gtk_widget_get_style_context(window);

    GtkStyleContext* styleContextLabel = gtk_widget_get_style_context(speedLabel);

    GtkStyleContext* styleContextExitButton = gtk_widget_get_style_context(exitButton);

    gtk_style_context_add_provider(styleContextWindow, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_style_context_add_provider(styleContextLabel, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_style_context_add_provider(styleContextExitButton, GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);



    // Connect the exit button to the exit program callback

    g_signal_connect(exitButton, "clicked", G_CALLBACK(exitProgram), NULL);



    // Create a vertical box to arrange the label and button

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5); // 5 is the spacing between child widgets

    gtk_container_add(GTK_CONTAINER(window), vbox);



    // Add the speedometer label to the vbox

    gtk_box_pack_start(GTK_BOX(vbox), speedLabel, TRUE, TRUE, 0);



    // Add the exit button to the vbox

    gtk_box_pack_end(GTK_BOX(vbox), exitButton, FALSE, FALSE, 0);



    // Show all widgets

    gtk_widget_show_all(window);



    // Start the GTK main loop

    gtk_main();



    return 0;

}

