#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
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
//    timer.setInterval(100);
//    connect(&timer, &QTimer::timeout, this, &MainWindow::update_camera_view);
//    timer.start();

    connect(ui->graphicsView, &QClickableGraphicsView::mousePressed, this, &MainWindow::on_graphicsView_clicked);

    // update sliders labels
    ui->questionsInSurvey->valueChanged(ui->questionsInSurvey->value());
    ui->answersInSurvey->valueChanged(ui->answersInSurvey->value());

    this->numberOfAnswers = ui->answersInSurvey->value();
    this->numberOfQuestions = ui->questionsInSurvey->value();

    ui->graphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
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
//    cv::Mat frame, i;
////    videoCapture >> frame;
//    frame = cv::imread("../taxman/assets/krzyzyki.png", CV_LOAD_IMAGE_COLOR);
//    cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);

//    NiblackSauvolaWolfJolion(frame, frame, WOLFJOLION, 40, 40, 0.5, 128);

//    i = frame;
//    auto qimage = QImage{i.data, i.cols, i.rows, QImage::Format_Indexed8};
//    ui->graphicsView->scene()->clear();
//    ui->graphicsView->scene()->addItem(new QGraphicsPixmapItem{QPixmap::fromImage(qimage)});
//    ui->graphicsView->update();
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
//    videoCapture >> referenceFrame;
    QFileDialog fileDialog {this};
    fileDialog.setFileMode(QFileDialog::ExistingFile);

    if (fileDialog.exec()) {
        auto selected = fileDialog.selectedFiles().first();
        referenceFrame = cv::imread(selected.toStdString(), CV_LOAD_IMAGE_GRAYSCALE);
        NiblackSauvolaWolfJolion(referenceFrame, referenceFrame, WOLFJOLION, 40, 40, 0.5, 128);
        qDebug() << "captured reference frame";
    }
}

void MainWindow::on_startSurveyDecodingButton_released()
{
    if (this->answerDecodingState == AnswerDecodingState::IDLE) {
//        this->timer.stop();
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

    qDebug() << "click at" << x << " " << y;
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
    for (auto j = 0; j < numberOfAnswers; ++j) {
        std::vector<std::tuple<double, double>> currentColumn;

        for (auto i = 0; i < numberOfQuestions; ++i) {
            auto current = std::make_tuple(
                        offsetX + j * columnWidth  + (columnWidth  / 2.0),
                        offsetY + i * columnHeight + (columnHeight / 2.0));
//                        offsetX + j * columnWidth  + (columnWidth  / 2.0));
            qDebug() << "point " << j << ", " << i << ": " << std::get<0>(current) << ", " << std::get<1>(current);
            currentColumn.push_back(current);
        }

        centralPoints.push_back(currentColumn);
    }

    cv::Mat absImg;
    cv::absdiff(frame, referenceFrame, absImg);
    qDebug() << "binarized absdiff";

    qDebug() << "non-zero points in abs" << cv::countNonZero(absImg);

    // TODO clean image
    // find how to compute bwmorph(in, 'clean') in opencv

    auto sliceHeight = (unsigned int)(columnHeight * 0.75);
    auto sliceWidth  = (unsigned int)(columnWidth * 0.75);

    auto answerTolerance = 30;
    results.clear();

    for (auto i = 0; i < numberOfQuestions; ++i) {
        for (auto j = 0; j < numberOfAnswers; ++j) {
            auto currentCell = centralPoints.at(j).at(i);

            auto yLow = (unsigned int)(std::get<1>(currentCell) - (double)(sliceHeight / 2.0) + 1);
            auto yHigh = (unsigned int)(std::get<1>(currentCell) + (double)(sliceHeight / 2.0) - 1);

            auto xLow = (unsigned int)(std::get<0>(currentCell) - (unsigned int)(sliceWidth / 2.0) + 1);
            auto xHigh = (unsigned int)(std::get<0>(currentCell) + (unsigned int)(sliceWidth / 2.0) - 1);
            qDebug() << xLow << " " << xHigh;
            qDebug() << yLow << " " << yHigh;

            auto height = yHigh - yLow;
            auto width = xHigh - xLow;

            auto rect = cv::Rect(xLow, yLow, width, height);
            auto window = absImg(rect);
            auto count = cv::countNonZero(window);

            qDebug() << "count for " << i+1 << ", " << j+1 << " is " << count;

            if (count > answerTolerance) {
                results.emplace_back(std::make_tuple(i+1, j+1));
            }
        }
    }

    std::string text;
    for (auto it : results) {
        text.append("Question ");
        text.append(std::to_string(std::get<0>(it)));
        text.append(" has answer ");
        text.append(std::to_string(std::get<1>(it)));
        text.append("\n");
    }

    ui->decodedAnswersTextArea->setPlainText(QString::fromStdString(text));
    answerDecodingState = AnswerDecodingState::IDLE;
//    this->timer.start();
}

void MainWindow::on_actionZapisz_triggered()
{
    QFileDialog fileDialog {this};
    fileDialog.setFileMode(QFileDialog::AnyFile);

    if (fileDialog.exec()) {
        auto selected = fileDialog.selectedFiles();

        QFile file {selected.first()};
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream str {&file};

        str << "question,answer" << endl;

        for (auto it : results) {
            str << QString::number(std::get<0>(it)) << "," << QString::number(std::get<1>(it)) << endl;
        }

        file.close();
    }
}

void MainWindow::on_openAnsweredSurveyButton_released()
{
    QFileDialog fileDialog {this};
    fileDialog.setFileMode(QFileDialog::ExistingFile);

    if (fileDialog.exec()) {
        auto selected = fileDialog.selectedFiles().first();
        frame = cv::imread(selected.toStdString(), CV_LOAD_IMAGE_GRAYSCALE);

        NiblackSauvolaWolfJolion(frame, frame, WOLFJOLION, 40, 40, 0.5, 128);

        auto qimage = QImage{frame.data, frame.cols, frame.rows, QImage::Format_Indexed8};
        ui->graphicsView->scene()->clear();
        ui->graphicsView->scene()->addItem(new QGraphicsPixmapItem{QPixmap::fromImage(qimage)});
        ui->graphicsView->update();
    }
}
