// mainwindow.h
#include <QLabel>
#include <QMainWindow>
#include <QSlider>
#include <opencv2/opencv.hpp>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void cargarImagenYMascara();
    void mostrarSlice(int indice);
    void generarVideo();

private:
    QLabel* originalLabel;
    QLabel* maskLabel;           // <- NUEVO: para mostrar la máscara de segmentación
    QLabel* resaltadaLabel;
    QSlider* slider;
    QLabel* sliceInfoLabel;
    QLabel* procesadaLabel;



    std::vector<cv::Mat> slices;
    std::vector<cv::Mat> maskSlices; // <- NUEVO: para guardar los slices de la máscara

    std::string currentFile;

    void mostrarEnLabel(const cv::Mat& imagen, QLabel* label);
    void guardarResultados();  

};
