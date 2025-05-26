#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <itkImage.h>
#include <opencv2/opencv.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Ejemplo: Conectar un botón a una función
    connect(ui->processButton, &QPushButton::clicked, this, &MainWindow::procesarArchivos);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::procesarArchivos() {
    // Lógica para procesar archivos .nii usando ITK y OpenCV
    std::cout << "Procesando archivos..." << std::endl;
}