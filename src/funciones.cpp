#include "funciones.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
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
    fs::create_directories("../output/plots");
}

size_t obtenerUsoMemoriaKB() {
    std::ifstream status("/proc/self/status");
    std::string linea;
    while (std::getline(status, linea)) {
        if (linea.rfind("VmRSS:", 0) == 0) {
            std::istringstream iss(linea);
            std::string key, value, unit;
            iss >> key >> value >> unit;
            return std::stoul(value);
        }
    }
    return 0;
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
        return cv::Mat();
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
    if (tipo == "original" || tipo == "equalized" || tipo == "mask") {
        carpeta = "../output/slices/";
    } else if (tipo == "resaltada") {
        carpeta = "../output/processed/";
    } else {
        carpeta = "../output/";
    }
    std::string ruta = carpeta + "slice_" + std::to_string(indice) + "_" + tipo + ".png";
    cv::imwrite(ruta, imagen);
}


cv::Mat resaltarArea(const cv::Mat& slice, const cv::Mat& maskOriginal, cv::Mat& mascaraProcesadaOut) {

    // Procesar la máscara: binarización + morfología
    cv::Mat binarizada, morfologica;
    cv::threshold(maskOriginal, binarizada, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    cv::morphologyEx(binarizada, morfologica, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1, -1), 2);
    cv::morphologyEx(morfologica, morfologica, cv::MORPH_CLOSE, cv::Mat(), cv::Point(-1, -1), 2);

    // Salida opcional de la máscara procesada
    mascaraProcesadaOut = morfologica.clone();

    // Convertir la imagen a color
    cv::Mat resultado;
    cv::cvtColor(slice, resultado, cv::COLOR_GRAY2BGR);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(morfologica, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        cv::Rect bbox = cv::boundingRect(contour);

        bool areaValida = area > 300 && area < 5000;
        bool alejadoDeBordes = bbox.x > 20 && bbox.y > 20 &&
                               bbox.x + bbox.width < slice.cols - 20 &&
                               bbox.y + bbox.height < slice.rows - 20;

        if (areaValida && alejadoDeBordes) {

    cv::Mat mask = cv::Mat::zeros(slice.size(), CV_8UC1);
    cv::drawContours(mask, std::vector<std::vector<cv::Point>>{contour}, -1, 255, -1);  // ← contorno relleno real

    // Mezcla de rojo bajito solo dentro del contorno
    for (int y = 0; y < resultado.rows; ++y) {
        for (int x = 0; x < resultado.cols; ++x) {
            if (mask.at<uchar>(y, x)) {
                cv::Vec3b& pixel = resultado.at<cv::Vec3b>(y, x);
                pixel = cv::Vec3b(
                    static_cast<uchar>(0.6 * pixel[0] + 0.4 * 0),
                    static_cast<uchar>(0.6 * pixel[1] + 0.4 * 0),
                    static_cast<uchar>(0.6 * pixel[2] + 0.4 * 255)
                );
            }
        }
    }

    // Dibuja el borde real del contorno
    cv::drawContours(resultado, std::vector<std::vector<cv::Point>>{contour}, -1, cv::Scalar(0, 0, 255), 2);
}

    }

    return resultado;
}

std::map<std::string, cv::Mat> aplicarTecnicas(const cv::Mat& slice, const cv::Mat& mascaraProcesada) {
    std::map<std::string, cv::Mat> resultados;
    cv::Mat temp;

    // Crear contorno rojo a partir de la máscara procesada
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mascaraProcesada, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::Mat contorno = cv::Mat::zeros(slice.size(), CV_8UC3);
    cv::drawContours(contorno, contours, -1, cv::Scalar(0, 0, 255), 2);

    // 1. Thresholding (Otsu)
    cv::threshold(slice, temp, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    cv::cvtColor(temp, temp, cv::COLOR_GRAY2BGR);
    cv::addWeighted(temp, 1.0, contorno, 1.0, 0, temp);
    resultados["Thresholding"] = temp.clone();

    // 2. Contrast Stretching
    double minVal, maxVal;
    cv::minMaxLoc(slice, &minVal, &maxVal);
    slice.convertTo(temp, CV_8UC1, 255.0 / (maxVal - minVal), -minVal * 255.0 / (maxVal - minVal));
    cv::cvtColor(temp, temp, cv::COLOR_GRAY2BGR);
    cv::addWeighted(temp, 1.0, contorno, 1.0, 0, temp);
    resultados["Stretching"] = temp.clone();

    // 3. Binarización por umbral de color
    cv::inRange(slice, 100, 200, temp);
    cv::cvtColor(temp, temp, cv::COLOR_GRAY2BGR);
    cv::addWeighted(temp, 1.0, contorno, 1.0, 0, temp);
    resultados["Binarizacion"] = temp.clone();

    // 4. NOT
    cv::bitwise_not(slice, temp);
    cv::cvtColor(temp, temp, cv::COLOR_GRAY2BGR);
    cv::addWeighted(temp, 1.0, contorno, 1.0, 0, temp);
    resultados["NOT"] = temp.clone();

    // 5. Lógica: AND, OR, XOR
    cv::Mat threshold;
    cv::threshold(slice, threshold, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    cv::Mat notImage;
    cv::bitwise_not(slice, notImage);

    cv::bitwise_and(threshold, notImage, temp);
    cv::cvtColor(temp, temp, cv::COLOR_GRAY2BGR);
    cv::addWeighted(temp, 1.0, contorno, 1.0, 0, temp);
    resultados["AND"] = temp.clone();

    cv::bitwise_or(threshold, notImage, temp);
    cv::cvtColor(temp, temp, cv::COLOR_GRAY2BGR);
    cv::addWeighted(temp, 1.0, contorno, 1.0, 0, temp);
    resultados["OR"] = temp.clone();

    cv::bitwise_xor(threshold, notImage, temp);
    cv::cvtColor(temp, temp, cv::COLOR_GRAY2BGR);
    cv::addWeighted(temp, 1.0, contorno, 1.0, 0, temp);
    resultados["XOR"] = temp.clone();

    // 6. Canny
    cv::Canny(slice, temp, 100, 200);
    cv::cvtColor(temp, temp, cv::COLOR_GRAY2BGR);
    cv::addWeighted(temp, 1.0, contorno, 1.0, 0, temp);
    resultados["Canny"] = temp.clone();

    // 7. Suavizado
    cv::GaussianBlur(slice, temp, cv::Size(5, 5), 1.5);
    cv::cvtColor(temp, temp, cv::COLOR_GRAY2BGR);
    cv::addWeighted(temp, 1.0, contorno, 1.0, 0, temp);
    resultados["Suavizado"] = temp.clone();

    // 8. CLAHE
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(2.0);
    clahe->apply(slice, temp);
    cv::cvtColor(temp, temp, cv::COLOR_GRAY2BGR);
    cv::addWeighted(temp, 1.0, contorno, 1.0, 0, temp);
    resultados["CLAHE"] = temp.clone();

    // 9. Laplacian
    cv::Laplacian(slice, temp, CV_16S, 3);
    cv::convertScaleAbs(temp, temp);
    cv::cvtColor(temp, temp, cv::COLOR_GRAY2BGR);
    cv::addWeighted(temp, 1.0, contorno, 1.0, 0, temp);
    resultados["Laplacian"] = temp.clone();

    // 10. Top Hat
    cv::morphologyEx(slice, temp, cv::MORPH_TOPHAT, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15)));
    cv::cvtColor(temp, temp, cv::COLOR_GRAY2BGR);
    cv::addWeighted(temp, 1.0, contorno, 1.0, 0, temp);
    resultados["TopHat"] = temp.clone();

    return resultados;
}

void guardarEstadisticas(const cv::Mat& slice, const cv::Mat& mask, int indice) {

    std::vector<uchar> valoresSegmentados;
    for (int y = 0; y < slice.rows; ++y) {
        for (int x = 0; x < slice.cols; ++x) {
            if (mask.at<uchar>(y, x) > 0) {
                valoresSegmentados.push_back(slice.at<uchar>(y, x));
            }
        }
    }

    if (valoresSegmentados.empty()) return;

    std::sort(valoresSegmentados.begin(), valoresSegmentados.end());

    double suma = std::accumulate(valoresSegmentados.begin(), valoresSegmentados.end(), 0.0);
    double media = suma / valoresSegmentados.size();
    double minVal = valoresSegmentados.front();
    double maxVal = valoresSegmentados.back();

    double varianza = 0;
    for (uchar v : valoresSegmentados)
        varianza += (v - media) * (v - media);
    varianza /= valoresSegmentados.size();
    double desviacion = std::sqrt(varianza);

    size_t n = valoresSegmentados.size();
    auto get_percentile = [&](double p) -> double {
        double idx = p * (n - 1);
        size_t i = static_cast<size_t>(idx);
        double frac = idx - i;
        return valoresSegmentados[i] * (1 - frac) + valoresSegmentados[std::min(i + 1, n - 1)] * frac;
    };

    double Q1 = get_percentile(0.25);
    double Q3 = get_percentile(0.75);
    double IQR = Q3 - Q1;

    double lowerFence = Q1 - 1.5 * IQR;
    double upperFence = Q3 + 1.5 * IQR;
    int outliers = std::count_if(valoresSegmentados.begin(), valoresSegmentados.end(), [&](double v) {
        return v < lowerFence || v > upperFence;
    });

    std::ofstream out("../output/stats/slice_" + std::to_string(indice) + "_stats.txt");
    out << "Media (zona): " << media << "\n";
    out << "Desviación estándar (zona): " << desviacion << "\n";
    out << "Mínimo (zona): " << minVal << "\n";
    out << "Máximo (zona): " << maxVal << "\n";
    out << "Área segmentada (pixeles): " << valoresSegmentados.size() << "\n";
    out << "Q1 (25%): " << Q1 << "\n";
    out << "Q3 (75%): " << Q3 << "\n";
    out << "IQR: " << IQR << "\n";
    out << "Outliers detectados: " << outliers << "\n";
    out.close();

    std::string csvPath = "../output/plots/slice_" + std::to_string(indice) + "_valores.csv";
    std::ofstream csv(csvPath);
    for (uchar v : valoresSegmentados)
        csv << static_cast<int>(v) << "\n";
    csv.close();

    std::string plotPath = "../output/plots/slice_" + std::to_string(indice) + "_boxplot.png";
    std::string scriptPath = "../scripts/boxplot_generator.py";
    std::string command = "python3 \"" + scriptPath + "\" \"" + csvPath + "\" \"" + plotPath + "\"";
    system(command.c_str());

 // Obtener uso actual de memoria
    size_t memoria = obtenerUsoMemoriaKB();

    // Mostrar en consola
    std::cout << "[INFO] Slice " << indice << " - Uso de memoria (KB): " << memoria << "\n";

    // Guardar log en archivo
    std::ofstream memlog("../output/stats/slice_" + std::to_string(indice) + "_memoria.txt");
    memlog << "Uso de memoria (KB): " << memoria << "\n";
}