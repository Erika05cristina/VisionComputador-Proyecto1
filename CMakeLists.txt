cmake_minimum_required(VERSION 3.10)
project(ProyectoIntegrador LANGUAGES CXX)

# Configuración del compilador
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontrar Qt5
set(Qt5_DIR "/usr/lib/x86_64-linux-gnu/cmake/Qt5")
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

# Configuración de ITK
set(ITK_DIR "/home/cristina/aplicaciones/itk_instalado/lib/cmake/ITK-6.0")
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# Configuración de OpenCV
find_package(OpenCV REQUIRED)

# Agregar el código fuente principal
add_executable(proyecto
    src/main.cpp
    src/funciones.hpp
    ui/mainwindow.cpp
    ui/mainwindow.h
    ui/mainwindow.ui
)

# Habilitar herramientas de Qt5
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Incluir directorios de OpenCV
target_include_directories(proyecto PRIVATE ${OpenCV_INCLUDE_DIRS})

# Enlazar bibliotecas de Qt5, ITK y OpenCV
target_link_libraries(proyecto PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    ${ITK_LIBRARIES}
    ${OpenCV_LIBS}
)

# Instalar el ejecutable
include(GNUInstallDirs)
install(TARGETS proyecto
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)