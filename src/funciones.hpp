#ifndef FUNCIONES_HPP
#define FUNCIONES_HPP

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkExtractImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkCastImageFilter.h"
#include <opencv2/opencv.hpp>
#include <fstream>
#include <string>
#include <cstring> 
#include <filesystem>

// ✅ Añade esto aquí (tipos ITK usados en todo el código)
using PixelType = unsigned char;
constexpr unsigned int Dimension = 3;
using ImageType = itk::Image<PixelType, Dimension>;
using SliceImageType = itk::Image<PixelType, 2>;

// Asegura que las carpetas existan
void asegurarDirectorios()
{
    std::filesystem::create_directories("../output/slices");
    std::filesystem::create_directories("../output/processed");
    std::filesystem::create_directories("../output/stats");
    std::filesystem::create_directories("../output/video");
}


// Cargar un slice específico desde archivo .nii
cv::Mat cargarSlice(const std::string& nombreArchivo, int sliceIndex)
{
    using ReaderType = itk::ImageFileReader<ImageType>;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(nombreArchivo);

    try {
        reader->Update();
    }
    catch (itk::ExceptionObject& e) {
        std::cerr << "Error al cargar la imagen: " << e << std::endl;
        return cv::Mat();
    }

    ImageType::Pointer image = reader->GetOutput();
    ImageType::RegionType region = image->GetLargestPossibleRegion();
    ImageType::SizeType size = region.GetSize();

    if (sliceIndex < 0 || sliceIndex >= static_cast<int>(size[2])) {
        std::cerr << "Índice de slice fuera de rango." << std::endl;
        return cv::Mat();
    }

    // Extraer la slice
    ImageType::RegionType sliceRegion = region;
    sliceRegion.SetSize(2, 0); // Tamaño en Z es 0 (slice única)
    sliceRegion.SetIndex(2, sliceIndex);

    using ExtractFilterType = itk::ExtractImageFilter<ImageType, SliceImageType>;
    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
    extractFilter->SetExtractionRegion(sliceRegion);
    extractFilter->SetInput(image);
    extractFilter->SetDirectionCollapseToSubmatrix();

    try {
        extractFilter->Update();
    }
    catch (itk::ExceptionObject& e) {
        std::cerr << "Error al extraer la slice: " << e << std::endl;
        return cv::Mat();
    }

    SliceImageType::Pointer sliceImage = extractFilter->GetOutput();
    SliceImageType::RegionType sliceRegionFinal = sliceImage->GetLargestPossibleRegion();
    SliceImageType::SizeType sliceSize = sliceRegionFinal.GetSize();

        // Obtener el puntero al buffer
    const unsigned char* buffer = sliceImage->GetBufferPointer();

    // Crear un cv::Mat que apunte al buffer directamente (sin copiar)
    cv::Mat img(sliceSize[1], sliceSize[0], CV_8UC1, const_cast<unsigned char*>(buffer));

    // Clonamos la imagen para evitar liberar memoria no controlada por OpenCV
    return img.clone();

}

// Mostrar y guardar imagen
void mostrarYGuardar(const cv::Mat &img, int sliceIndex, const std::string &tipo)
{
    std::string ventana = "Slice - " + tipo;
    std::string ruta = "../output/" + std::string((tipo == "original") ? "slices" : "processed") +
                       "/slice_" + std::to_string(sliceIndex) + "_" + tipo + ".png";

    cv::imshow(ventana, img);
    cv::imwrite(ruta, img);
    cv::waitKey(0);
    cv::destroyWindow(ventana);
}

// Resaltar áreas de interés
cv::Mat resaltarArea(cv::Mat img)
{
    cv::Mat equalized, denoised, binary;

    // Ecualización de histograma
    cv::equalizeHist(img, equalized);

    // Suavizado
    cv::medianBlur(equalized, denoised, 5);

    // Umbralización
    double thresholdValue = 150;
    cv::threshold(denoised, binary, thresholdValue, 255, cv::THRESH_BINARY);

    // Morfología
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(binary, binary, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel);

    // Colorizar área resaltada
    cv::Mat colorMask;
    cv::cvtColor(binary, colorMask, cv::COLOR_GRAY2BGR);
    colorMask.setTo(cv::Scalar(0, 0, 255), binary); // Rojo

    // Combinar con imagen original
    cv::Mat result;
    cv::cvtColor(img, result, cv::COLOR_GRAY2BGR);
    cv::addWeighted(result, 1.0, colorMask, 0.5, 0.0, result);

    return result;
}

// Guardar estadísticas
void guardarEstadisticas(const cv::Mat &img, int sliceIndex)
{
    double minVal, maxVal;
    cv::minMaxLoc(img, &minVal, &maxVal);
    double mean = cv::mean(img)[0];

    std::ofstream out("../output/stats/slice_" + std::to_string(sliceIndex) + "_stats.txt");
    out << "Min: " << minVal << "\n";
    out << "Max: " << maxVal << "\n";
    out << "Mean: " << mean << "\n";
    out.close();
}

// Generar video con secuencia de slices
void generarVideoSlices(const std::string &archivo, int inicio, int fin)
{
    cv::VideoWriter video("../output/video/slice_sequence.avi",
                          cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
                          10,
                          cv::Size(256, 256),
                          true);

    for (int i = inicio; i <= fin; ++i)
    {
        cv::Mat slice = cargarSlice(archivo, i);
        if (slice.empty())
            continue;

        cv::Mat processed = resaltarArea(slice);
        cv::resize(processed, processed, cv::Size(256, 256));
        video.write(processed);
    }

    video.release();
    std::cout << "✅ Video generado: ../output/video/slice_sequence.avi\n";
}

#endif