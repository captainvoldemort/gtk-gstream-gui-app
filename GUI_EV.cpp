#include <gtk/gtk.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

typedef struct {
    GtkWidget *main_window;
    GtkWidget *speedLabel;
    GtkWidget *backButton;
} AppData;

// Function declarations
void updateSpeedometer(GtkLabel* speedLabel);
static gboolean update_speedometer(gpointer data);
static void miscButtonClicked(GtkWidget*, gpointer);
static void cameraButtonClicked(GtkWidget*, gpointer data);
static void backButtonClicked(GtkWidget*, gpointer data);
static void setup_home_screen(AppData *app_data);  // Declaration added here
static void setup_camera_screen(AppData *app_data);  // Declaration added here

int getRandomSpeed() {
    return rand() % 100;
}

void updateSpeedometer(GtkLabel* speedLabel) {
    int speed = getRandomSpeed();
    char speedText[5];
    sprintf(speedText, "%02d", speed);
    gtk_label_set_text(speedLabel, speedText);
}

static gboolean update_speedometer(gpointer data) {
    updateSpeedometer(GTK_LABEL(data));
    return G_SOURCE_CONTINUE;
}

static void miscButtonClicked(GtkWidget*, gpointer) {
    std::cout << "Sample Text" << std::endl;
}

static void cameraButtonClicked(GtkWidget*, gpointer data) {
    AppData *app_data = static_cast<AppData *>(data);

    // Remove all buttons from the grid
    GList *children, *iter;
    children = gtk_container_get_children(GTK_CONTAINER(app_data->main_window));
    for (iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

    // Setup the camera screen
    setup_camera_screen(app_data);
}

static void backButtonClicked(GtkWidget*, gpointer data) {
    AppData *app_data = static_cast<AppData *>(data);
    // Reset the grid by calling setup_home_screen
    gtk_container_remove(GTK_CONTAINER(app_data->main_window), gtk_bin_get_child(GTK_BIN(app_data->main_window)));
    setup_home_screen(app_data);
}

static void setup_home_screen(AppData *app_data) {
    app_data->speedLabel = gtk_label_new("00");
    gtk_label_set_use_markup(GTK_LABEL(app_data->speedLabel), TRUE);
    gtk_widget_set_halign(app_data->speedLabel, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(app_data->speedLabel, GTK_ALIGN_CENTER);

    g_timeout_add(1000, update_speedometer, app_data->speedLabel);

    GtkWidget *exitButton = gtk_button_new_with_label("Exit");
    GtkWidget *miscButton = gtk_button_new_with_label("Miscellaneous Feature");
    GtkWidget *cameraButton = gtk_button_new_with_label("Camera Feed");

    g_signal_connect(G_OBJECT(exitButton), "clicked", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(miscButton), "clicked", G_CALLBACK(miscButtonClicked), NULL);
    g_signal_connect(G_OBJECT(cameraButton), "clicked", G_CALLBACK(cameraButtonClicked), app_data);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), app_data->speedLabel, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), exitButton, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), miscButton, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), cameraButton, 2, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(app_data->main_window), grid);
    gtk_widget_show_all(app_data->main_window);
}

static void setup_camera_screen(AppData *app_data) {
    // Code to setup the camera feed screen goes here
    // You can add the camera feed widget and the back button
    app_data->backButton = gtk_button_new_with_label("Back");
    g_signal_connect(G_OBJECT(app_data->backButton), "clicked", G_CALLBACK(backButtonClicked), app_data);

    // Replace the following lines with your camera feed setup code
    GtkWidget *cameraFeedLabel = gtk_label_new("Camera Feed"); // Example label, replace with camera feed widget
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), cameraFeedLabel, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), app_data->backButton, 0, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(app_data->main_window), grid);
    gtk_widget_show_all(app_data->main_window);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    AppData app_data;
    app_data.main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app_data.main_window), "Electric Scooter GUI");
    gtk_window_set_default_size(GTK_WINDOW(app_data.main_window), 400, 300);
    g_signal_connect(G_OBJECT(app_data.main_window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Initial setup
    setup_home_screen(&app_data);

    gtk_main();

    return 0;
}
