#include "mainwindow.h"
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QPushButton>
#include "../src/funciones.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    QPushButton* cargarButton = new QPushButton("Cargar y Procesar Slice", this);
    QPushButton* videoButton = new QPushButton("Generar Video", this);

    originalLabel = new QLabel("Original");
    equalizedLabel = new QLabel("Ecualizada");
    resaltadaLabel = new QLabel("Resaltada");

    originalLabel->setAlignment(Qt::AlignCenter);
    equalizedLabel->setAlignment(Qt::AlignCenter);
    resaltadaLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(cargarButton);
    layout->addWidget(videoButton);
    layout->addWidget(originalLabel);
    layout->addWidget(equalizedLabel);
    layout->addWidget(resaltadaLabel);

    setCentralWidget(centralWidget);

    connect(cargarButton, &QPushButton::clicked, this, &MainWindow::procesarArchivos);
    connect(videoButton, &QPushButton::clicked, this, &MainWindow::generarVideo);
}

MainWindow::~MainWindow() {}

void MainWindow::procesarArchivos() {
    QString archivo = QFileDialog::getOpenFileName(this, "Selecciona un archivo NIfTI", "", "*.nii *.nii.gz");
    if (archivo.isEmpty()) return;

    bool ok;
    int sliceIndex = QInputDialog::getInt(this, "Índice del slice", "Ingresa el índice:", 0, 0, 1000, 1, &ok);
    if (!ok) return;

    asegurarDirectorios();
    cv::Mat slice = cargarSlice(archivo.toStdString(), sliceIndex);
    if (slice.empty()) {
        QMessageBox::warning(this, "Error", "No se pudo cargar el slice.");
        return;
    }

    mostrarYGuardar(slice, sliceIndex, "original");
    mostrarEnLabel(slice, originalLabel);

    cv::Mat ecualizado;
    cv::equalizeHist(slice, ecualizado);
    mostrarYGuardar(ecualizado, sliceIndex, "equalized");
    mostrarEnLabel(ecualizado, equalizedLabel);

    cv::Mat resaltado = resaltarArea(slice);
    mostrarYGuardar(resaltado, sliceIndex, "resaltada");
    mostrarEnLabel(resaltado, resaltadaLabel);

    guardarEstadisticas(slice, sliceIndex);

    QMessageBox::information(this, "Éxito", "Procesamiento completado.");
}


void MainWindow::generarVideo() {
    QString archivo = QFileDialog::getOpenFileName(this, "Selecciona un archivo NIfTI", "", "*.nii *.nii.gz");
    if (archivo.isEmpty()) return;

    bool ok1, ok2;
    int inicio = QInputDialog::getInt(this, "Índice inicial", "Desde:", 0, 0, 1000, 1, &ok1);
    if (!ok1) return;

    int fin = QInputDialog::getInt(this, "Índice final", "Hasta:", 0, 0, 1000, 1, &ok2);
    if (!ok2 || fin < inicio) {
        QMessageBox::warning(this, "Error", "Índices inválidos.");
        return;
    }

    generarVideoSlices(archivo.toStdString(), inicio, fin);
    QMessageBox::information(this, "Éxito", "Video generado exitosamente.");
}
void MainWindow::mostrarEnLabel(const cv::Mat& imagen, QLabel* label) {
    if (imagen.empty()) return;

    cv::Mat imagenRGB;
    if (imagen.channels() == 1) {
        cv::cvtColor(imagen, imagenRGB, cv::COLOR_GRAY2RGB);
    } else {
        imagenRGB = imagen.clone();
    }

    QImage qimg(imagenRGB.data, imagenRGB.cols, imagenRGB.rows, imagenRGB.step, QImage::Format_RGB888);
    label->setPixmap(QPixmap::fromImage(qimg).scaled(256, 256, Qt::KeepAspectRatio));
}
