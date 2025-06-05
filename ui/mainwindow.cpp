#include "mainwindow.h"
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QPushButton>
#include <QSlider>
#include <QTabWidget>
#include "../src/funciones.hpp"
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkExtractImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkMaskImageFilter.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    resize(1000, 600);

    // Crear tabs
    QTabWidget* tabs = new QTabWidget(this);
    QWidget* tabPrincipal = new QWidget(this);
    QWidget* tabTecnicas = new QWidget(this);

    // --------- TAB PRINCIPAL ---------
    QVBoxLayout* layoutPrincipal = new QVBoxLayout(tabPrincipal);

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
    resaltadaLabel = new QLabel("Bordes resaltados sobre imagen");
    sliceInfoLabel = new QLabel("Slice actual: - / -", this);
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
    imageLayout->addWidget(procesadaLabel);
    imageLayout->addWidget(resaltadaLabel);

    QHBoxLayout* textLayout = new QHBoxLayout();
    textLayout->addWidget(textoOriginal);
    textLayout->addWidget(textoMascara);
    textLayout->addWidget(textoProcesada);
    textLayout->addWidget(textoResaltada);

    layoutPrincipal->addWidget(cargarButton);
    layoutPrincipal->addWidget(videoButton);
    layoutPrincipal->addWidget(guardarButton);
    layoutPrincipal->addWidget(slider);
    layoutPrincipal->addLayout(imageLayout);
    layoutPrincipal->addLayout(textLayout);
    layoutPrincipal->addWidget(sliceInfoLabel);

    // --------- TAB TÉCNICAS ---------
    QVBoxLayout* layoutTecnicas = new QVBoxLayout(tabTecnicas);

    labelThreshold = new QLabel("Thresholding", this);
    labelStretching = new QLabel("Stretching", this);
    labelCanny = new QLabel("Canny", this);
    labelLogic = new QLabel("Lógica NOT", this);
    labelSuavizado = new QLabel("Suavizado", this);
    labelBinarizacion = new QLabel("Binarización por Color", this);
    labelAND = new QLabel("AND", this);
    labelOR = new QLabel("OR", this);
    labelXOR = new QLabel("XOR", this);
    labelCLAHE = new QLabel("CLAHE", this);
    labelLaplacian = new QLabel("Laplacian", this);
    labelTopHat = new QLabel("Top Hat", this);
// y no olvides .setFixedSize y .setAlignment como los otros


  



    labelThreshold->setFixedSize(256, 256);
    labelStretching->setFixedSize(256, 256);
    labelCanny->setFixedSize(256, 256);
    labelLogic->setFixedSize(256, 256);
    labelSuavizado->setFixedSize(256, 256);
    labelBinarizacion->setFixedSize(256, 256);

    labelThreshold->setAlignment(Qt::AlignCenter);
    labelStretching->setAlignment(Qt::AlignCenter);
    labelCanny->setAlignment(Qt::AlignCenter);
    labelLogic->setAlignment(Qt::AlignCenter);
    labelSuavizado->setAlignment(Qt::AlignCenter);
    labelBinarizacion->setAlignment(Qt::AlignCenter);

    QGridLayout* gridTecnicas = new QGridLayout();
    // FILA 1
    gridTecnicas->addWidget(new QLabel("Thresholding"), 0, 0);
    gridTecnicas->addWidget(new QLabel("Stretching"), 0, 1);
    gridTecnicas->addWidget(new QLabel("Canny"), 0, 2);
    gridTecnicas->addWidget(new QLabel("NOT"), 0, 3);

    gridTecnicas->addWidget(labelThreshold, 1, 0);
    gridTecnicas->addWidget(labelStretching, 1, 1);
    gridTecnicas->addWidget(labelCanny, 1, 2);
    gridTecnicas->addWidget(labelLogic, 1, 3);

    // FILA 2
    gridTecnicas->addWidget(new QLabel("Suavizado"), 2, 0);
    gridTecnicas->addWidget(new QLabel("Binarización por Color"), 2, 1);
    gridTecnicas->addWidget(new QLabel("AND"), 2, 2);
    gridTecnicas->addWidget(new QLabel("OR"), 2, 3);

    gridTecnicas->addWidget(labelSuavizado, 3, 0);
    gridTecnicas->addWidget(labelBinarizacion, 3, 1);
    gridTecnicas->addWidget(labelAND, 3, 2);
    gridTecnicas->addWidget(labelOR, 3, 3);

    // FILA 3
    gridTecnicas->addWidget(new QLabel("XOR"), 4, 0);
    gridTecnicas->addWidget(new QLabel("CLAHE"), 4, 1);
    gridTecnicas->addWidget(new QLabel("Laplacian"), 4, 2);
    gridTecnicas->addWidget(new QLabel("Top Hat"), 4, 3);

    gridTecnicas->addWidget(labelXOR, 5, 0);
    gridTecnicas->addWidget(labelCLAHE, 5, 1);
    gridTecnicas->addWidget(labelLaplacian, 5, 2);
    gridTecnicas->addWidget(labelTopHat, 5, 3);



    layoutTecnicas->addLayout(gridTecnicas);

    for (QLabel* lbl : {labelThreshold, labelStretching, labelCanny, labelLogic, labelSuavizado, labelBinarizacion}) {
    lbl->setFixedSize(256, 256);
    lbl->setAlignment(Qt::AlignCenter);
    }
    for (QLabel* lbl : {labelAND, labelOR, labelXOR, labelCLAHE, labelLaplacian, labelTopHat}) {
        lbl->setFixedSize(256, 256);
        lbl->setAlignment(Qt::AlignCenter);
    }

    // Agregar tabs al widget principal
    tabs->addTab(tabPrincipal, "Principal");
    tabs->addTab(tabTecnicas, "Técnicas");

    setCentralWidget(tabs);

    // Conexiones
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
        
        auto tecnicas = aplicarTecnicas(original);
        mostrarEnLabel(tecnicas["Thresholding"], labelThreshold);
        mostrarEnLabel(tecnicas["Stretching"], labelStretching);
        mostrarEnLabel(tecnicas["Canny"], labelCanny);
        mostrarEnLabel(tecnicas["Suavizado"], labelSuavizado);
        mostrarEnLabel(tecnicas["Binarizacion"], labelBinarizacion);
        mostrarEnLabel(tecnicas["AND"], labelAND);
        mostrarEnLabel(tecnicas["NOT"], labelLogic);
        mostrarEnLabel(tecnicas["OR"], labelOR);
        mostrarEnLabel(tecnicas["XOR"], labelXOR);
        mostrarEnLabel(tecnicas["CLAHE"], labelCLAHE);
        mostrarEnLabel(tecnicas["Laplacian"], labelLaplacian);
        mostrarEnLabel(tecnicas["TopHat"], labelTopHat);


        
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
    mostrarYGuardar(original, indice, "original");
    mostrarYGuardar(mask, indice, "mask");

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

    guardarEstadisticas(original, mask, indice);

    QMessageBox::information(this, "Éxito", "Slice y estadísticas guardadas correctamente.");
}