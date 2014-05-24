#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <opencv2/opencv.hpp>

#include "image_processing/algorithms.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow{parent},
    ui{new Ui::MainWindow},
    videoCapture{0}
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
    cv::Mat frame, i;
//    videoCapture >> frame;
    frame = cv::imread("../taxman/assets/ankieta.jpg", CV_LOAD_IMAGE_COLOR);

    cv::cvtColor(frame, i, CV_BGR2RGB);

    auto qimage = QImage{i.data, i.cols, i.rows, QImage::Format_RGB888};
    ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addItem(new QGraphicsPixmapItem{QPixmap::fromImage(qimage)});
    ui->graphicsView->update();
}
