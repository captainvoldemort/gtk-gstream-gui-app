cmake_minimum_required(VERSION 3.5)

project(WebcamViewer)

find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(webcam_viewer main.cpp)

target_link_libraries(webcam_viewer Qt5::Widgets)
