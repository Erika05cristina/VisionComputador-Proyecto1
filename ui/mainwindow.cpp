#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    // Crear el widget central y un layout vertical
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    // Crear el botón
    processButton = new QPushButton("Procesar", this);

    // Agregar el botón al layout
    layout->addWidget(processButton);

    // Ajustar el layout al widget central
    centralWidget->setLayout(layout);

    // Establecer el widget central de la ventana
    setCentralWidget(centralWidget);

    // Conectar la señal clicked del botón al slot procesarArchivos
    connect(processButton, &QPushButton::clicked, this, &MainWindow::procesarArchivos);
}

MainWindow::~MainWindow() {
    // Qt elimina los widgets hijos automáticamente, no necesitas delete manual aquí.
}

void MainWindow::procesarArchivos() {
    std::cout << "Procesando archivos..." << std::endl;
    // Aquí va la lógica para procesar archivos .nii con ITK y OpenCV
}
