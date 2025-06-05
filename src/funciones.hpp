#ifndef FUNCIONES_HPP
#define FUNCIONES_HPP

#include <string>
#include <opencv2/opencv.hpp>
#include <map>
#include <string>


void asegurarDirectorios();
cv::Mat cargarSlice(const std::string& rutaArchivo, int indice);
void mostrarYGuardar(const cv::Mat& imagen, int indice, const std::string& tipo);
cv::Mat resaltarArea(const cv::Mat& slice, const cv::Mat& maskOriginal, cv::Mat& mascaraProcesadaOut);
void guardarEstadisticas(const cv::Mat& slice, const cv::Mat& mask, int indice);
void generarVideoSlices(const std::string& rutaArchivo, int inicio, int fin);
std::map<std::string, cv::Mat> aplicarTecnicas(const cv::Mat& slice);

#endif 