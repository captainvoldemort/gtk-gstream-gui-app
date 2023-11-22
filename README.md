# **GTK GStreamer GUI App Documentation**

## **Introduction**

Welcome to the documentation for the GTK GStreamer GUI App. This repository contains a C++ code file named **`Video-GUI.cpp`** that implements a graphical user interface (GUI) for playing videos using GStreamer and GTK.

Before you can run or compile the application, you need to install the required dependencies, which include GTK and GStreamer.

## **Prerequisites**

Ensure that you have the following prerequisites installed on your system:

### **1. Install gtk+-3.0**

To install GTK, run the following command in your terminal:

```bash
bashCopy code
sudo apt-get install libgtk-3-dev

```

### **2. Install gstreamer1.0 and related packages**

To install GStreamer and its related packages, run the following command:

```bash
bashCopy code
sudo apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio
```

## **Getting Started**

Follow these steps to run the GTK GStreamer GUI App:

### **1. Clone the Repository**

```bash
bashCopy code
git clone https://github.com/yashreadytobox/gtk-gstream-gui-app.git

```

### **2. Compile the Code**

Navigate to the repository directory:

```bash
bashCopy code
cd gtk-gstream-gui-app

```

Compile the **`Video-GUI.cpp`** file using the following command:

```bash
bashCopy code
gcc -g Video-GUI.cpp -o Video-GUI `pkg-config --cflags --libs gtk+-3.0 gstreamer-1.0 gstreamer-video-1.0 gstreamer-base-1.0`

```

### **3. Run the Application**

After successful compilation, run the application:

```bash
bashCopy code
./Video-GUI

```

This will launch the GTK GStreamer GUI App, allowing you to interact with the video player interface.

## **Troubleshooting**

If you encounter any issues during the installation or compilation process, please refer to the official documentation for GTK and GStreamer for additional assistance.

- [GTK Documentation](https://developer.gnome.org/gtk3/stable/)
- [GStreamer Documentation](https://gstreamer.freedesktop.org/documentation/)

If you have questions or need further support, feel free to open an issue on the [GitHub repository](https://github.com/yashreadytobox/gtk-gstream-gui-app).
