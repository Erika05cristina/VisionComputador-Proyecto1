#ifndef FUNCIONES_HPP
#define FUNCIONES_HPP

#include <string>
#include <opencv2/opencv.hpp>

void asegurarDirectorios();
cv::Mat cargarSlice(const std::string& rutaArchivo, int indice);
void mostrarYGuardar(const cv::Mat& imagen, int indice, const std::string& tipo);
cv::Mat resaltarArea(const cv::Mat& slice, const cv::Mat& maskOriginal, cv::Mat& mascaraProcesadaOut);
void guardarEstadisticas(const cv::Mat& slice, const cv::Mat& mask, int indice);
void generarVideoSlices(const std::string& rutaArchivo, int inicio, int fin);

#endif // FUNCIONES_HPP
