#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QTimer>

class WebcamViewer : public QWidget
{
    Q_OBJECT

public:
    WebcamViewer(QWidget *parent = nullptr)
        : QWidget(parent), capture(0)
    {
        // Open the default camera using V4L2
        capture.open(0);

        if (!capture.isOpened())
        {
            qDebug() << "Error: Unable to open the camera.";
            return;
        }

        // Set up the GUI
        QVBoxLayout *layout = new QVBoxLayout(this);
        QLabel *imageLabel = new QLabel(this);
        layout->addWidget(imageLabel);

        // Create a timer to update the image periodically
        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(updateImage()));
        timer->start(33); // Update every 33 milliseconds (approx 30 fps)

        setLayout(layout);
    }

public slots:
    void updateImage()
    {
        cv::Mat frame;
        capture >> frame;

        if (frame.empty())
        {
            qDebug() << "Error: Unable to capture frame.";
            return;
        }

        // Convert OpenCV Mat to QImage
        QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_BGR888);
        img = img.rgbSwapped();

        // Display the image
        QLabel *imageLabel = findChild<QLabel *>();
        imageLabel->setPixmap(QPixmap::fromImage(img));
    }

private:
    cv::VideoCapture capture;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    WebcamViewer viewer;
    viewer.setWindowTitle("Webcam Viewer");
    viewer.setGeometry(100, 100, 640, 480);
    viewer.show();

    return app.exec();
}

#include "main.moc"  // For Qt's Meta-Object Compiler
