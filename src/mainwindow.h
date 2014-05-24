#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <opencv2/highgui/highgui.hpp>

#include <memory>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();
    void update_camera_view();

    void on_horizontalSlider_2_sliderMoved(int position);

    void on_horizontalSlider_sliderMoved(int position);

    void on_spinBox_valueChanged(const int value);
private:
    Ui::MainWindow *ui;
    QGraphicsScene scene;
    QTimer timer;
    QGraphicsPixmapItem pixmap;
    cv::VideoCapture videoCapture;
    int cannyLowerThreshold, cannyHigherThreshold, minDistance;
};

#endif // MAINWINDOW_H
