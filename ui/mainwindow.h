// mainwindow.h
#include <QLabel>
#include <QMainWindow>
#include <opencv2/opencv.hpp>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void procesarArchivos();
    void generarVideo();

private:
    QLabel* originalLabel;
    QLabel* equalizedLabel;
    QLabel* resaltadaLabel;

    void mostrarEnLabel(const cv::Mat& imagen, QLabel* label);
};
