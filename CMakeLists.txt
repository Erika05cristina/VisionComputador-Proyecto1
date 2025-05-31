cmake_minimum_required(VERSION 3.15)
project(proyecto-vision LANGUAGES C CXX)

# Configuración del compilador
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Habilitar herramientas de Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Usar siempre variables de entorno para las rutas
set(Qt5_DIR $ENV{Qt5_DIR})
set(ITK_DIR $ENV{ITK_DIR})
set(OpenCV_DIR $ENV{OpenCV_DIR})

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
find_package(OpenCV REQUIRED)

add_executable(proyecto
    src/main.cpp
    src/funciones.cpp 
    ui/mainwindow.cpp
)

target_include_directories(proyecto PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(proyecto PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    ${ITK_LIBRARIES}
    ${OpenCV_LIBS}
)

include(GNUInstallDirs)
install(TARGETS proyecto
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
