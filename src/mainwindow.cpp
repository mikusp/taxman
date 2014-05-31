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
    timer.setInterval(32);
    connect(&timer, &QTimer::timeout, this, &MainWindow::update_camera_view);
//    timer.start();

    connect(ui->graphicsView, &QClickableGraphicsView::mousePressed, this, &MainWindow::on_graphicsView_clicked);

    // update sliders labels
    ui->questionsInSurvey->valueChanged(ui->questionsInSurvey->value());
    ui->answersInSurvey->valueChanged(ui->answersInSurvey->value());

    this->numberOfAnswers = ui->answersInSurvey->value();
    this->numberOfQuestions = ui->questionsInSurvey->value();

    camera = this->ui->useCameraCheckbox->checkState() == Qt::Checked;

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
    videoCapture >> frame;
    cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);

    auto qimage = QImage{frame.data, frame.cols, frame.rows, QImage::Format_Indexed8};
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
    if (camera) {
        videoCapture >> referenceFrame;
        cv::cvtColor(referenceFrame, referenceFrame, CV_BGR2GRAY);
    }
    else {
        QFileDialog fileDialog {this};
        fileDialog.setFileMode(QFileDialog::ExistingFile);

        if (fileDialog.exec()) {
            auto selected = fileDialog.selectedFiles().first();
            referenceFrame = cv::imread(selected.toStdString(), CV_LOAD_IMAGE_GRAYSCALE);
        }
    }


    NiblackSauvolaWolfJolion(referenceFrame, referenceFrame, WOLFJOLION, 40, 40, 0.5, 128);
    qDebug() << "captured reference frame";
}

void MainWindow::on_startSurveyDecodingButton_released()
{
    if (this->answerDecodingState == AnswerDecodingState::IDLE) {
        if (camera) {
            this->timer.stop();
        }
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

    if (camera) {
         NiblackSauvolaWolfJolion(frame, frame, WOLFJOLION, 40, 40, 0.5, 128);
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
        text.append("Odpowiedź na pytanie ");
        text.append(std::to_string(std::get<0>(it)));
        text.append(": ");
        text.append(std::to_string(std::get<1>(it)));
        text.append("\n");
    }

    std::map<int, int> answersMap;
    for (auto it : answers) {
        answersMap.insert(std::pair<int,int>(std::get<0>(it), std::get<1>(it)));
    }

    std::map<int, int> resultsMap;
    for (auto it : results) {
        resultsMap.insert(std::pair<int,int>(std::get<0>(it), std::get<1>(it)));
    }

    auto score = 0;
    for (auto q = 1; q <= this->numberOfQuestions; ++q) {
        auto correct = false;
        try {
            correct = answersMap.at(q) == resultsMap.at(q);
        }
        catch (const std::out_of_range&) {
            // not sure if safe...
        }

        text.append("Pytanie ");
        text.append(std::to_string(q));

        if (correct) {
            text.append(" POPRAWNE\n");
            ++score;
        }
        else {
            text.append(" NIEPOPRAWNE\n");
        }
    }

    text.append("Wynik: ");
    text.append(std::to_string(score));
    text.append("/");
    text.append(std::to_string(numberOfQuestions));
    text.append("\n");

    ui->decodedAnswersTextArea->setPlainText(QString::fromStdString(text));
    answerDecodingState = AnswerDecodingState::IDLE;

    if (camera) {
        this->timer.start();
    }
}

void MainWindow::on_actionZapisz_triggered()
{
    QFileDialog fileDialog {this};
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);

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

void MainWindow::on_useCameraCheckbox_stateChanged(int state)
{
    if (state == Qt::Checked) {
        // use camera
        this->camera = true;
        timer.start();
        this->ui->openAnsweredSurveyButton->setEnabled(false);
    }
    else {
        // use files
        this->camera = false;
        timer.stop();
        this->ui->openAnsweredSurveyButton->setEnabled(true);
    }
}

void MainWindow::on_correctAnswersButton_released()
{
    QFileDialog fileDialog {this};
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setNameFilter(QString{"CSV (*.csv)"});

    if (fileDialog.exec()) {
        auto selected = fileDialog.selectedFiles().first();

        QFile file {selected};
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream str {&file};

        QString line;
        answers.clear();

        while ((line = str.readLine()) != "") {
            auto qa = line.split(",");
            bool qOk, aOk;
            auto q = qa.value(0, "-1").toInt(&qOk);
            auto a = qa.value(1, "-1").toInt(&aOk);
            if (qOk && aOk && q != -1 && a != -1)
                answers.emplace_back(std::make_tuple(q, a));
        }

        qDebug() << "read correct answers";
    }
}
