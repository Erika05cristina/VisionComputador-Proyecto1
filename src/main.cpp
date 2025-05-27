// #include <iostream>
// #include "funciones.hpp"
// #include <opencv2/opencv.hpp>

// using namespace std;
// using namespace cv;


// int main() {
//     string rutaArchivo;
//     int sliceIndex;

//     cout << "\n=== 🧠 Lector de NIfTI con Procesamiento Avanzado ===\n";

//     cout << "Ruta del archivo .nii: ";
//     getline(cin, rutaArchivo);

//     cout << "Índice del slice a procesar: ";
//     while (!(cin >> sliceIndex)) {
//         cout << "Por favor, ingresa un número válido: ";
//         cin.clear();
//         cin.ignore(numeric_limits<streamsize>::max(), '\n');
//     }

//     // Crear directorios si no existen
//     asegurarDirectorios();

//     // Cargar slice
//     Mat slice = cargarSlice(rutaArchivo, sliceIndex);
//     if (slice.empty()) {
//         cerr << "❌ Error al cargar el slice. Verifica la ruta y el índice.\n";
//         return EXIT_FAILURE;
//     }

//     // Mostrar y guardar imagen original
//     mostrarYGuardar(slice, sliceIndex, "original");

//     // Procesamiento: Ecualización
//     Mat ecualizado;
//     equalizeHist(slice, ecualizado);
//     mostrarYGuardar(ecualizado, sliceIndex, "equalized");

//     // Resaltar área de interés
//     Mat areaResaltada = resaltarArea(slice);
//     mostrarYGuardar(areaResaltada, sliceIndex, "resaltada");

//     // Calcular estadísticas y guardar
//     guardarEstadisticas(slice, sliceIndex);

//     // Opción de generar video
//     cout << "\n¿Deseas generar un video con varios slices? (s/n): ";
//     char respuesta;
//     cin >> respuesta;

//     if (respuesta == 's' || respuesta == 'S') {
//         int inicio, fin;
//         cout << "Índice inicial: ";
//         while (!(cin >> inicio)) {
//             cout << "Ingresa un número válido para el inicio: ";
//             cin.clear();
//             cin.ignore(numeric_limits<streamsize>::max(), '\n');
//         }

//         cout << "Índice final: ";
//         while (!(cin >> fin)) {
//             cout << "Ingresa un número válido para el final: ";
//             cin.clear();
//             cin.ignore(numeric_limits<streamsize>::max(), '\n');
//         }

//         generarVideoSlices(rutaArchivo, inicio, fin);
//     }

//     cout << "\n✅ Proceso completado correctamente.\n";
//     cout << "   Archivos guardados en '../output/'.\n";

//     return EXIT_SUCCESS;
// }
#include <QApplication>
#include "../ui/mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
