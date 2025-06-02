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

    resize(1000, 600);  // Aumentar el tamaño inicial de la ventana

    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    QPushButton* cargarButton = new QPushButton("Cargar imagen y máscara", this);
    QPushButton* videoButton = new QPushButton("Generar Video", this);
    QPushButton* guardarButton = new QPushButton("Guardar Resultados", this); 

    QLabel* textoOriginal = new QLabel("Slice original");
    QLabel* textoMascara = new QLabel("Máscara original");
    QLabel* textoProcesada = new QLabel("Máscara procesada");
    QLabel* textoResaltada = new QLabel("Área resaltada");

    slider = new QSlider(Qt::Horizontal);
    slider->setEnabled(false);

    originalLabel = new QLabel("Slice actual");
    maskLabel = new QLabel("Máscara");
    sliceInfoLabel = new QLabel("Slice actual: - / -", this);
    resaltadaLabel = new QLabel("Bordes resaltados sobre imagen");
    procesadaLabel = new QLabel("Máscara procesada");

    textoOriginal->setAlignment(Qt::AlignCenter);
    textoMascara->setAlignment(Qt::AlignCenter);
    textoProcesada->setAlignment(Qt::AlignCenter);
    textoResaltada->setAlignment(Qt::AlignCenter);

    
    originalLabel->setFixedSize(320, 320);
    maskLabel->setFixedSize(320, 320);
    resaltadaLabel->setFixedSize(320, 320);
    procesadaLabel->setFixedSize(320, 320);

    originalLabel->setAlignment(Qt::AlignCenter);
    maskLabel->setAlignment(Qt::AlignCenter);
    resaltadaLabel->setAlignment(Qt::AlignCenter);
    sliceInfoLabel->setAlignment(Qt::AlignCenter);
    procesadaLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout* imageLayout = new QHBoxLayout();
    imageLayout->addWidget(originalLabel);
    imageLayout->addWidget(maskLabel);
    imageLayout->addWidget(procesadaLabel);  // ← nuevo
    imageLayout->addWidget(resaltadaLabel);

    QHBoxLayout* textLayout = new QHBoxLayout();
    textLayout->addWidget(textoOriginal);
    textLayout->addWidget(textoMascara);
    textLayout->addWidget(textoProcesada);
    textLayout->addWidget(textoResaltada);


    mainLayout->addWidget(cargarButton);
    mainLayout->addWidget(videoButton);
    mainLayout->addWidget(guardarButton);
    mainLayout->addWidget(slider);
    mainLayout->addLayout(imageLayout);
    mainLayout->addLayout(textLayout);
    mainLayout->addWidget(sliceInfoLabel);

    setCentralWidget(centralWidget);

    connect(cargarButton, &QPushButton::clicked, this, &MainWindow::cargarImagenYMascara);
    connect(videoButton, &QPushButton::clicked, this, &MainWindow::generarVideo);
    connect(guardarButton, &QPushButton::clicked, this, &MainWindow::guardarResultados);
    connect(slider, &QSlider::valueChanged, this, &MainWindow::mostrarSlice);
}

MainWindow::~MainWindow() {}

void MainWindow::cargarImagenYMascara() {
    QString imagenPath = QFileDialog::getOpenFileName(this, "Selecciona la imagen (post_1.hdr)", "", "*.hdr");
    if (imagenPath.isEmpty()) return;

    QString mascaraPath = QFileDialog::getOpenFileName(this, "Selecciona la máscara (segmentation.hdr)", "", "*.hdr");
    if (mascaraPath.isEmpty()) return;

    using ImageType = itk::Image<short, 3>;
    using ReaderType = itk::ImageFileReader<ImageType>;
    using ExtractFilterType = itk::ExtractImageFilter<ImageType, itk::Image<short, 2>>;

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

        // Mostrar imágenes originales
        mostrarEnLabel(original, originalLabel);
        mostrarEnLabel(mask, maskLabel);

        // Declarar salida para la máscara procesada
        cv::Mat morfologica;

        // Usar la función actualizada que procesa la máscara y resalta
        cv::Mat resaltada = resaltarArea(original, mask, morfologica);

        // Mostrar la máscara procesada y la imagen resaltada
        mostrarEnLabel(morfologica, procesadaLabel);
        mostrarEnLabel(resaltada, resaltadaLabel);

        // Actualizar etiqueta de índice
        sliceInfoLabel->setText(QString("Slice actual: %1 / %2")
                                .arg(indice)
                                .arg(slices.size() - 1));
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
        cv::cvtColor(imagen, imagenRGB, cv::COLOR_BGR2RGB);
    }

    QImage qimg(imagenRGB.data, imagenRGB.cols, imagenRGB.rows, imagenRGB.step, QImage::Format_RGB888);
    label->setPixmap(QPixmap::fromImage(qimg).scaled(256, 256, Qt::KeepAspectRatio));
}

void MainWindow::guardarResultados() {

    asegurarDirectorios();

    int indice = slider->value();

    if (indice < 0 || indice >= slices.size()) {
        std::cerr << "[ERROR] Índice inválido: " << indice << std::endl;
        QMessageBox::warning(this, "Error", "Índice inválido.");
        return;
    }

    cv::Mat original = slices[indice];
    cv::Mat mask = maskSlices[indice];


    if (original.empty()) std::cerr << "[ERROR] Slice original vacío\n";
    if (mask.empty()) std::cerr << "[ERROR] Máscara vacía\n";


    // Guardar slice original procesado y máscara en /slices
    mostrarYGuardar(original, indice, "original");  // guarda en /output/slices/
    mostrarYGuardar(mask, indice, "mask");          // guarda en /output/slices/

    // Generar imagen resaltada
    cv::Mat resaltada;
    cv::cvtColor(original, resaltada, cv::COLOR_GRAY2BGR);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
  
    cv::Mat filledMask = cv::Mat::zeros(mask.size(), CV_8UC1);
    cv::drawContours(filledMask, contours, -1, 255, -1);

    for (int y = 0; y < resaltada.rows; ++y) {
        for (int x = 0; x < resaltada.cols; ++x) {
            if (filledMask.at<uchar>(y, x)) {
                cv::Vec3b& pixel = resaltada.at<cv::Vec3b>(y, x);
                pixel = 0.6 * pixel + 0.4 * cv::Vec3b(0, 0, 255);
            }
        }
    }

    mostrarYGuardar(resaltada, indice, "resaltada");

    guardarEstadisticas(original, indice);

    QMessageBox::information(this, "Éxito", "Slice y estadísticas guardadas correctamente.");
}