#include <gtk/gtk.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

const int WIDTH = 400;
const int HEIGHT = 300;
const double MAX_SPEED = 200.0;  // Maximum speed for the gauge

class SpeedGauge {
public:
    SpeedGauge(GtkDrawingArea* drawing_area) : drawing_area(drawing_area) {
        g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(on_draw), this);
        g_timeout_add(1000, G_SOURCE_FUNC(on_timeout), this);
    }

    void draw(GtkWidget* widget, cairo_t* cr) {
        // Clear the drawing area
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_paint(cr);

        // Draw the speed gauge
        double speed = get_random_speed();
        draw_gauge(cr, speed);
    }

private:
    GtkDrawingArea* drawing_area;

    static gboolean on_draw(GtkWidget* widget, cairo_t* cr, gpointer data) {
        reinterpret_cast<SpeedGauge*>(data)->draw(widget, cr);
        return FALSE;
    }

    static gboolean on_timeout(gpointer data) {
        GtkDrawingArea* drawing_area = reinterpret_cast<SpeedGauge*>(data)->drawing_area;
        gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
        return G_SOURCE_CONTINUE;
    }

    double get_random_speed() const {
        // Generate a random speed between 0 and MAX_SPEED
        return static_cast<double>(rand()) / RAND_MAX * MAX_SPEED;
    }

    void draw_gauge(cairo_t* cr, double speed) const {
        double angle = M_PI / 4 + (speed / MAX_SPEED) * (3 * M_PI / 2 - M_PI / 4);

        int center_x = WIDTH / 2;
        int center_y = HEIGHT;

        // Draw the arc representing the speed
        cairo_set_source_rgb(cr, 0.2, 0.8, 0.2);
        cairo_set_line_width(cr, 10);
        cairo_arc(cr, center_x, center_y, HEIGHT / 2 - 20, M_PI / 4, angle);
        cairo_stroke(cr);

        // Draw the needle
        cairo_set_source_rgb(cr, 0.8, 0, 0);
        cairo_set_line_width(cr, 3);
        cairo_move_to(cr, center_x, center_y);
        cairo_line_to(cr, center_x + cos(angle) * (HEIGHT / 2 - 20), center_y - sin(angle) * (HEIGHT / 2 - 20));
        cairo_stroke(cr);

        // Draw the speed text
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 20);
        cairo_set_source_rgb(cr, 0, 0, 0);

        char speed_text[50];
        snprintf(speed_text, sizeof(speed_text), "Speed: %.2f", speed);
        cairo_move_to(cr, 10, 30);
        cairo_show_text(cr, speed_text);
    }
};

int main(int argc, char* argv[]) {
    srand(time(NULL));

    gtk_init(&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget* drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, WIDTH, HEIGHT);

    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    SpeedGauge speed_gauge(drawing_area);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
