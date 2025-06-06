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
    QLabel* maskLabel;          
    QLabel* resaltadaLabel;
    QSlider* slider;
    QLabel* sliceInfoLabel;
    QLabel* procesadaLabel;
    QLabel *labelThreshold, *labelStretching, *labelCanny;
    QLabel *labelLogic, *labelSuavizado, *labelBinarizacion;
    QLabel *labelAND, *labelOR, *labelXOR, *labelCLAHE;
    QLabel *labelLaplacian, *labelGabor;

    std::vector<cv::Mat> slices;
    std::vector<cv::Mat> maskSlices; 

    std::string currentFile;

    void mostrarEnLabel(const cv::Mat& imagen, QLabel* label);
    void guardarResultados();  

};
