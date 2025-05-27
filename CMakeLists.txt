cmake_minimum_required(VERSION 3.15)
project(proyecto-vision LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt automoc y autorcc (pero desactivamos autouic porque no hay .ui)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)  # <-- desactivado

# Rutas personalizadas
set(OpenCV_DIR "/home/visionups/aplicaciones/Librerias/opencv/opencvi/lib/cmake/opencv4")
find_package(OpenCV REQUIRED)

set(VTK_DIR "/usr/lib/x86_64-linux-gnu/cmake/vtk-9.1")
find_package(VTK REQUIRED)

set(ITK_DIR "/usr/lib/x86_64-linux-gnu/cmake/ITK-5.3")
set(ITK_USE_VTK ON CACHE BOOL "Enable VTK support" FORCE)
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

set(Qt5_DIR "/usr/lib/x86_64-linux-gnu/cmake/Qt5")
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

# Ejecutable sin el .ui
add_executable(proyecto
    src/main.cpp
    src/funciones.hpp
    ui/mainwindow.cpp
    ui/mainwindow.h
)

target_include_directories(proyecto PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(proyecto PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    ${OpenCV_LIBS}
    ${VTK_LIBRARIES}
    ${ITK_LIBRARIES}
)

include(GNUInstallDirs)
install(TARGETS proyecto
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
