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
#include <itkCastImageFilter.h>
#include <itkMaskImageFilter.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    QPushButton* cargarButton = new QPushButton("Cargar imagen y máscara", this);
    QPushButton* videoButton = new QPushButton("Generar Video", this);
    slider = new QSlider(Qt::Horizontal);
    slider->setEnabled(false);

    originalLabel = new QLabel("Slice actual");
    maskLabel = new QLabel("Máscara");
    resaltadaLabel = new QLabel("Bordes resaltados sobre imagen");

    originalLabel->setAlignment(Qt::AlignCenter);
    maskLabel->setAlignment(Qt::AlignCenter);
    resaltadaLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(cargarButton);
    layout->addWidget(videoButton);
    layout->addWidget(slider);
    layout->addWidget(originalLabel);
    layout->addWidget(maskLabel);
    layout->addWidget(resaltadaLabel);

    setCentralWidget(centralWidget);

    connect(cargarButton, &QPushButton::clicked, this, &MainWindow::cargarImagenYMascara);
    connect(videoButton, &QPushButton::clicked, this, &MainWindow::generarVideo);
    connect(slider, &QSlider::valueChanged, this, &MainWindow::mostrarSlice);
}

MainWindow::~MainWindow() {}

void MainWindow::cargarImagenYMascara() {
    QString imagenPath = QFileDialog::getOpenFileName(this, "Selecciona la imagen (post_1.hdr)", "", "*.hdr");
    if (imagenPath.isEmpty()) return;

    QString mascaraPath = QFileDialog::getOpenFileName(this, "Selecciona la máscara (segmentation.hdr)", "", "*.hdr");
    if (mascaraPath.isEmpty()) return;

    using ImageType = itk::Image<short, 3>;
    using MaskType = itk::Image<short, 3>;  // Asumimos que la máscara también es de tipo short
    using ReaderType = itk::ImageFileReader<ImageType>;
    using MaskReaderType = itk::ImageFileReader<MaskType>;
    using ExtractFilterType = itk::ExtractImageFilter<ImageType, itk::Image<short, 2>>;
    using MaskExtractFilterType = itk::ExtractImageFilter<MaskType, itk::Image<short, 2>>;

    ReaderType::Pointer imagenReader = ReaderType::New();
    ReaderType::Pointer mascaraReader = ReaderType::New();

    imagenReader->SetFileName(imagenPath.toStdString());
    mascaraReader->SetFileName(mascaraPath.toStdString());

    try {
        imagenReader->Update();
        mascaraReader->Update();
    } catch (...) {
        QMessageBox::critical(this, "Error", "No se pudo leer la imagen o la máscara.");
        return;
    }

    currentFile = imagenPath.toStdString();

    auto region = imagenReader->GetOutput()->GetLargestPossibleRegion();
    int totalSlices = region.GetSize()[2];
    slices.clear();
    maskSlices.clear();

    for (int i = 0; i < totalSlices; ++i) {
        itk::Image<short, 3>::IndexType start = {0, 0, i};
        itk::Image<short, 3>::SizeType size = {region.GetSize()[0], region.GetSize()[1], 0};

        itk::Image<short, 3>::RegionType extractRegion;
        extractRegion.SetIndex(start);
        extractRegion.SetSize(size);

        ExtractFilterType::Pointer imagenExtract = ExtractFilterType::New();
        imagenExtract->SetExtractionRegion(extractRegion);
        imagenExtract->SetInput(imagenReader->GetOutput());
        imagenExtract->SetDirectionCollapseToSubmatrix();
        imagenExtract->Update();

        ExtractFilterType::Pointer mascaraExtract = ExtractFilterType::New();
        mascaraExtract->SetExtractionRegion(extractRegion);
        mascaraExtract->SetInput(mascaraReader->GetOutput());
        mascaraExtract->SetDirectionCollapseToSubmatrix();
        mascaraExtract->Update();

        auto imagenSlice = imagenExtract->GetOutput();
        auto mascaraSlice = mascaraExtract->GetOutput();

        cv::Mat img(imagenSlice->GetLargestPossibleRegion().GetSize()[1],
                    imagenSlice->GetLargestPossibleRegion().GetSize()[0],
                    CV_16SC1,
                    const_cast<short*>(imagenSlice->GetBufferPointer()));

        cv::Mat mask(mascaraSlice->GetLargestPossibleRegion().GetSize()[1],
                     mascaraSlice->GetLargestPossibleRegion().GetSize()[0],
                     CV_16SC1,
                     const_cast<short*>(mascaraSlice->GetBufferPointer()));

        cv::normalize(img, img, 0, 255, cv::NORM_MINMAX);
        img.convertTo(img, CV_8UC1);
        slices.push_back(img.clone());

        cv::normalize(mask, mask, 0, 255, cv::NORM_MINMAX);
        mask.convertTo(mask, CV_8UC1);
        maskSlices.push_back(mask.clone());
    }

    slider->setEnabled(true);
    slider->setRange(0, slices.size() - 1);
    slider->setValue(0);
    mostrarSlice(0);
}

void MainWindow::mostrarSlice(int indice) {
    if (indice >= 0 && indice < slices.size()) {
        cv::Mat original = slices[indice];
        cv::Mat mask = maskSlices[indice];
        mostrarEnLabel(original, originalLabel);
        mostrarEnLabel(mask, maskLabel);

        cv::Mat contornos;
        cv::cvtColor(original, contornos, cv::COLOR_GRAY2BGR);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        cv::drawContours(contornos, contours, -1, cv::Scalar(0, 0, 255), 2);

        mostrarEnLabel(contornos, resaltadaLabel);
    }
}

void MainWindow::generarVideo() {
    if (currentFile.empty() || slices.empty()) {
        QMessageBox::warning(this, "Advertencia", "Primero debes cargar una imagen válida.");
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
