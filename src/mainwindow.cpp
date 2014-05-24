#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <opencv2/opencv.hpp>

#include "qclickablegraphicsview.h"

#include "image_processing/algorithms.h"
#include "image_processing/binarizewolfjolion.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow{parent},
    ui{new Ui::MainWindow},
    videoCapture{0},
    answerDecodingState{AnswerDecodingState::IDLE}
{
    ui->setupUi(this);
    timer.setInterval(100);
    connect(&timer, &QTimer::timeout, this, &MainWindow::update_camera_view);
    timer.start();

    connect(ui->graphicsView, &QClickableGraphicsView::mousePressed, this, &MainWindow::on_graphicsView_clicked);

    // update sliders labels
    ui->questionsInSurvey->valueChanged(ui->questionsInSurvey->value());
    ui->answersInSurvey->valueChanged(ui->answersInSurvey->value());

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
    videoCapture >> frame;
//    frame = cv::imread("../taxman/assets/krzyzyki.png", CV_LOAD_IMAGE_COLOR);
    cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);

    NiblackSauvolaWolfJolion(frame, frame, WOLFJOLION, 40, 40, 0.5, 128);

    i = frame;
    auto qimage = QImage{i.data, i.cols, i.rows, QImage::Format_Indexed8};
    ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addItem(new QGraphicsPixmapItem{QPixmap::fromImage(qimage)});
    ui->graphicsView->update();
}

void MainWindow::on_questionsInSurvey_sliderReleased()
{
    this->numberOfQuestions = ui->questionsInSurvey->value();
    qDebug() << "questions: " << this->numberOfQuestions;
}

void MainWindow::on_questionsInSurvey_valueChanged(int value)
{
    ui->questionsNumber->setText(QString::number(value));
}

void MainWindow::on_answersInSurvey_valueChanged(int value)
{
    ui->answersNumber->setText(QString::number(value));
}

void MainWindow::on_answersInSurvey_sliderReleased()
{
    this->numberOfAnswers = ui->answersInSurvey->value();
    qDebug() << "answers: " << this->numberOfAnswers;
}

void MainWindow::on_captureReferenceFrameButton_released()
{
    videoCapture >> referenceFrame;
    qDebug() << "captured reference frame";
}

void MainWindow::on_startSurveyDecodingButton_released()
{
    if (this->answerDecodingState == AnswerDecodingState::IDLE) {
        this->timer.stop();
        this->answerDecodingState = AnswerDecodingState::WAITING_FOR_1ST_CLICK;
        QApplication::setOverrideCursor(Qt::CrossCursor);
        qDebug() << "decoding started; waiting for 1st click";
    }
    else {
        qWarning() << "decoding already in progress";
    }
}

void MainWindow::on_graphicsView_clicked(int x, int y) {
    if (answerDecodingState == AnswerDecodingState::WAITING_FOR_1ST_CLICK) {
        leftUpper = {x, y};
        qDebug() << "left upper point is now at " << x << ", " << y;
        answerDecodingState = AnswerDecodingState::WAITING_FOR_2ND_CLICK;
        qDebug() << "waiting for 2nd click";
    }
    else if (answerDecodingState == AnswerDecodingState::WAITING_FOR_2ND_CLICK) {
        rightLower = {x, y};
        qDebug() << "right lower point is now at " << x << ", " << y;
        QApplication::restoreOverrideCursor();
        calculateAnswers();
    }
}

void MainWindow::calculateAnswers() {
    qDebug() << "calculating answers";

    auto areaHeight = rightLower.y - leftUpper.y;
    auto areaWidth  = rightLower.x - leftUpper.x;

    auto columnHeight = areaHeight / numberOfQuestions;
    auto columnWidth  = areaWidth / numberOfAnswers;

    auto offsetX = leftUpper.x;
    auto offsetY = leftUpper.y;

    std::vector<std::vector<std::tuple<double, double>>> centralPoints;
    for (auto j = 0; j < numberOfQuestions; ++j) {
        std::vector<std::tuple<double, double>> currentColumn;

        for (auto i = 0; i < numberOfAnswers; ++i) {
            auto current = std::make_tuple(
                        offsetX + j * columnWidth  + (columnWidth  / 2.0),
                        offsetY + i * columnHeight + (columnHeight / 2.0));
            currentColumn.push_back(current);
        }

        centralPoints.push_back(currentColumn);
    }

    cv::Mat cross, bg;
    videoCapture >> crossImg;
    cv::cvtColor(cross, cross, cv::COLOR_BGR2GRAY);
    cv::cvtColor(referenceFrame, bg, cv::COLOR_BGR2GRAY);

    NiblackSauvolaWolfJolion(cross, cross, WOLFJOLION, 40, 40, 0.5, 128);
    NiblackSauvolaWolfJolion(bg, bg, WOLFJOLION, 40, 40, 0.5, 128);

    cv::Mat absImg;
    cv::absdiff(cross, bg, absImg);
    NiblackSauvolaWolfJolion(absImg, absImg, WOLFJOLION, 40, 40, 0.5, 128);

    // TODO clean image
    // find how to compute bwmorph(in, 'clean') in opencv



    ui->decodedAnswersTextArea->setPlainText("bogus answers");
    answerDecodingState = AnswerDecodingState::IDLE;
    this->timer.start();
}
