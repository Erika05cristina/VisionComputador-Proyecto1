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
    void cargarArchivoNii();
    void mostrarSlice(int indice);
    void generarVideo();

private:
    QLabel* originalLabel;
    QLabel* equalizedLabel;
    QLabel* resaltadaLabel;
    QSlider* slider;

    std::vector<cv::Mat> slices;
    std::string currentFile;

    void mostrarEnLabel(const cv::Mat& imagen, QLabel* label);
};
