#include "funciones.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <filesystem>
#include <fstream>
#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkExtractImageFilter.h>
#include <itkCastImageFilter.h>

namespace fs = std::filesystem;

void asegurarDirectorios() {
    fs::create_directories("../output/slices");
    fs::create_directories("../output/processed");
    fs::create_directories("../output/stats");
    fs::create_directories("../output/video");
}

cv::Mat cargarSlice(const std::string& rutaArchivo, int indice) {
    using ImageType = itk::Image<short, 3>;
    using ReaderType = itk::ImageFileReader<ImageType>;
    using ExtractFilterType = itk::ExtractImageFilter<ImageType, itk::Image<short, 2>>;

    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(rutaArchivo);

    try {
        reader->Update();
    } catch (...) {
        return cv::Mat(); // Devolver vacía si hay error
    }

    ImageType::RegionType region = reader->GetOutput()->GetLargestPossibleRegion();
    if (indice < 0 || indice >= region.GetSize()[2]) {
        return cv::Mat();
    }

    ImageType::SizeType size = region.GetSize();
    ImageType::IndexType start = {0, 0, static_cast<long>(indice)};
    ImageType::SizeType extractSize = {size[0], size[1], 0};

    ImageType::RegionType extractRegion;
    extractRegion.SetSize(extractSize);
    extractRegion.SetIndex(start);

    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
    extractFilter->SetExtractionRegion(extractRegion);
    extractFilter->SetInput(reader->GetOutput());
    extractFilter->SetDirectionCollapseToSubmatrix();

    try {
        extractFilter->Update();
    } catch (...) {
        return cv::Mat();
    }

    auto slice2D = extractFilter->GetOutput();
    if (!slice2D) return cv::Mat();

    cv::Mat output(slice2D->GetLargestPossibleRegion().GetSize()[1],
                   slice2D->GetLargestPossibleRegion().GetSize()[0],
                   CV_16SC1,
                   const_cast<short*>(slice2D->GetBufferPointer()));

    cv::normalize(output, output, 0, 255, cv::NORM_MINMAX);
    output.convertTo(output, CV_8UC1);
    return output;
}

void mostrarYGuardar(const cv::Mat& imagen, int indice, const std::string& tipo) {
    std::string carpeta;
    if (tipo == "original" || tipo == "equalized") {
        carpeta = "../output/slices/";
    } else if (tipo == "resaltada") {
        carpeta = "../output/processed/";
    } else {
        carpeta = "../output/";  // fallback general
    }
    std::string ruta = carpeta + "slice_" + std::to_string(indice) + "_" + tipo + ".png";
    cv::imwrite(ruta, imagen);
}

cv::Mat resaltarArea(const cv::Mat& slice) {
    cv::Mat suavizada, binarizada, morfologica, contornos;

    // 1. Suavizar para eliminar ruido
    cv::GaussianBlur(slice, suavizada, cv::Size(5, 5), 1.5);

    // 2. Umbral automático con Otsu
    cv::threshold(suavizada, binarizada, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // 3. Operaciones morfológicas para limpiar regiones pequeñas y cerrar bordes
    cv::morphologyEx(binarizada, morfologica, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1, -1), 2);  // Elimina ruido
    cv::morphologyEx(morfologica, morfologica, cv::MORPH_CLOSE, cv::Mat(), cv::Point(-1, -1), 2); // Rellena huecos

    // 4. Convertir la imagen original a BGR para dibujar colores
    cv::cvtColor(slice, contornos, cv::COLOR_GRAY2BGR);

    // 5. Detección de contornos
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(morfologica, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 6. Dibujar contornos (en rojo)
    cv::drawContours(contornos, contours, -1, cv::Scalar(0, 0, 255), 2);

    return contornos;
}

void guardarEstadisticas(const cv::Mat& slice, int indice) {
    cv::Scalar media, stddev;
    cv::meanStdDev(slice, media, stddev);
    double minVal, maxVal;
    cv::minMaxLoc(slice, &minVal, &maxVal);

    std::ofstream out("../output/stats/slice_" + std::to_string(indice) + "_stats.txt");
    out << "Media: " << media[0] << "\n";
    out << "Desviación estándar: " << stddev[0] << "\n";
    out << "Mínimo: " << minVal << "\n";
    out << "Máximo: " << maxVal << "\n";
    out.close();
}

void generarVideoSlices(const std::string& rutaArchivo, int inicio, int fin) {
    asegurarDirectorios();

    cv::Mat primerSliceValido;
    for (int i = inicio; i <= fin; ++i) {
        primerSliceValido = cargarSlice(rutaArchivo, i);
        if (!primerSliceValido.empty()) {
            break;
        }
    }

    if (primerSliceValido.empty()) {
        std::cerr << "[ERROR] No se encontraron slices válidos para generar el video.\n";
        return;
    }

    std::string videoPath = "../output/video/video_slices.avi";
    cv::Size frameSize = primerSliceValido.size();
    cv::VideoWriter writer(videoPath, cv::VideoWriter::fourcc('M','J','P','G'), 5, frameSize, false);

    if (!writer.isOpened()) {
        std::cerr << "[ERROR] No se pudo abrir el archivo de video para escritura.\n";
        return;
    }

    for (int i = inicio; i <= fin; ++i) {
        cv::Mat slice = cargarSlice(rutaArchivo, i);
        if (!slice.empty()) {
            if (slice.size() != frameSize) {
                std::cerr << "[ADVERTENCIA] Slice " << i << " tiene un tamaño diferente y será ignorado.\n";
                continue;
            }
            writer.write(slice);
        }
    }

    writer.release();
}
