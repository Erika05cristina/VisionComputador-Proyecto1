#include "mainwindow.h"
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QPushButton>
#include <QSlider>
#include "../src/funciones.hpp"
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkExtractImageFilter.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    QPushButton* cargarButton = new QPushButton("Cargar archivo .nii", this);
    QPushButton* videoButton = new QPushButton("Generar Video", this);
    slider = new QSlider(Qt::Horizontal);
    slider->setEnabled(false);

    originalLabel = new QLabel("Slice actual");
    equalizedLabel = new QLabel("Ecualizado");
    resaltadaLabel = new QLabel("Bordes resaltados");

    originalLabel->setAlignment(Qt::AlignCenter);
    equalizedLabel->setAlignment(Qt::AlignCenter);
    resaltadaLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(cargarButton);
    layout->addWidget(videoButton);
    layout->addWidget(slider);
    layout->addWidget(originalLabel);
    layout->addWidget(equalizedLabel);
    layout->addWidget(resaltadaLabel);

    setCentralWidget(centralWidget);

    connect(cargarButton, &QPushButton::clicked, this, &MainWindow::cargarArchivoNii);
    connect(videoButton, &QPushButton::clicked, this, &MainWindow::generarVideo);
    connect(slider, &QSlider::valueChanged, this, &MainWindow::mostrarSlice);
}

MainWindow::~MainWindow() {}

void MainWindow::cargarArchivoNii() {
    QString archivo = QFileDialog::getOpenFileName(this, "Selecciona un archivo NIfTI", "", "*.nii *.nii.gz");
    if (archivo.isEmpty()) return;

    using ImageType = itk::Image<short, 3>;
    using ReaderType = itk::ImageFileReader<ImageType>;
    using ExtractFilterType = itk::ExtractImageFilter<ImageType, itk::Image<short, 2>>;

    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(archivo.toStdString());

    try {
        reader->Update();
    } catch (...) {
        QMessageBox::critical(this, "Error", "No se pudo leer el archivo.");
        return;
    }

    currentFile = archivo.toStdString();

    auto region = reader->GetOutput()->GetLargestPossibleRegion();
    int totalSlices = region.GetSize()[2];
    slices.clear();

    for (int i = 0; i < totalSlices; ++i) {
        itk::Image<short, 3>::IndexType start = {0, 0, i};
        itk::Image<short, 3>::SizeType size = {region.GetSize()[0], region.GetSize()[1], 0};

        itk::Image<short, 3>::RegionType extractRegion;
        extractRegion.SetIndex(start);
        extractRegion.SetSize(size);

        ExtractFilterType::Pointer extract = ExtractFilterType::New();
        extract->SetExtractionRegion(extractRegion);
        extract->SetInput(reader->GetOutput());
        extract->SetDirectionCollapseToSubmatrix();
        extract->Update();

        auto slice2D = extract->GetOutput();
        cv::Mat mat(slice2D->GetLargestPossibleRegion().GetSize()[1],
                    slice2D->GetLargestPossibleRegion().GetSize()[0],
                    CV_16SC1,
                    const_cast<short*>(slice2D->GetBufferPointer()));

        cv::normalize(mat, mat, 0, 255, cv::NORM_MINMAX);
        mat.convertTo(mat, CV_8UC1);
        slices.push_back(mat.clone());
    }

    slider->setEnabled(true);
    slider->setRange(0, slices.size() - 1);
    slider->setValue(0);
    mostrarSlice(0);
}

void MainWindow::mostrarSlice(int indice) {
    if (indice >= 0 && indice < slices.size()) {
        cv::Mat original = slices[indice];
        mostrarEnLabel(original, originalLabel);

        cv::Mat ecualizado;
        cv::equalizeHist(original, ecualizado);
        mostrarEnLabel(ecualizado, equalizedLabel);

        cv::Mat resaltado = resaltarArea(original);
        mostrarEnLabel(resaltado, resaltadaLabel);
    }
}

void MainWindow::generarVideo() {
    if (currentFile.empty() || slices.empty()) {
        QMessageBox::warning(this, "Advertencia", "Primero debes cargar un archivo .nii válido.");
        return;
    }

    bool ok1, ok2;
    int inicio = QInputDialog::getInt(this, "Índice inicial", "Desde:", 0, 0, slices.size() - 1, 1, &ok1);
    if (!ok1) return;

    int fin = QInputDialog::getInt(this, "Índice final", "Hasta:", slices.size() - 1, 0, slices.size() - 1, 1, &ok2);
    if (!ok2 || fin < inicio) {
        QMessageBox::warning(this, "Error", "Índices inválidos.");
        return;
    }

    generarVideoSlices(currentFile, inicio, fin);
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