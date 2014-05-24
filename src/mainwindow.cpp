#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <opencv2/imgproc/imgproc.hpp>

#include "image_processing/algorithms.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow{parent},
    ui{new Ui::MainWindow},
    scene{},
    timer{},
    pixmap{},
    videoCapture{0},
    cannyLowerThreshold{50},
    cannyHigherThreshold{200},
    minDistance{100}
{
    ui->setupUi(this);
    timer.setInterval(16);
    connect(&timer, &QTimer::timeout, this, &MainWindow::update_camera_view);
    timer.start();

    ui->graphicsView->setScene(&scene);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionExit_triggered()
{
    qApp->exit();
}

void MainWindow::update_camera_view()
{
    cv::Mat frame;
//    videoCapture >> frame;
    frame = cv::imread("../taxman/assets/ankieta.jpg", CV_LOAD_IMAGE_COLOR);
    cv::Mat i, smoothed, edges, bw;
    cv::GaussianBlur(frame, smoothed, cv::Size(3, 3), 0, 0);
    cv::cvtColor(smoothed, bw, CV_RGB2GRAY);
//    Algorithms::sobel(bw, edges);
    cv::Canny(bw, edges, cannyLowerThreshold, cannyHigherThreshold);
    cv::cvtColor(edges, i, CV_GRAY2RGB);

    //corners
//    std::vector<cv::Point2f> corners;
//    cv::goodFeaturesToTrack(edges, corners, 4, 0.1, minDistance);

//    for (size_t j = 0; j < corners.size(); ++j)
//        cv::circle(i, corners[j], 4, cv::Scalar(255, 0, 0), 4);

    auto qimage = QImage{i.data, i.cols, i.rows, QImage::Format_RGB888};
    ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addItem(new QGraphicsPixmapItem{QPixmap::fromImage(qimage)});
    ui->graphicsView->update();
}

void MainWindow::on_horizontalSlider_2_sliderMoved(int position)
{
    cannyLowerThreshold = position;
}

void MainWindow::on_horizontalSlider_sliderMoved(int position)
{
    cannyHigherThreshold = position;
}

void MainWindow::on_spinBox_valueChanged(const int value)
{
    minDistance = value;
}
