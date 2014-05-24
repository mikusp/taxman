#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <opencv2/opencv.hpp>

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

    void on_questionsInSurvey_sliderReleased();

    void on_questionsInSurvey_valueChanged(int value);

    void on_answersInSurvey_valueChanged(int value);

    void on_answersInSurvey_sliderReleased();

private:
    Ui::MainWindow *ui;
    QGraphicsScene scene;
    QTimer timer;
    QGraphicsPixmapItem pixmap;
    cv::VideoCapture videoCapture;

    int numberOfQuestions;
    int numberOfAnswers;
};

#endif // MAINWINDOW_H
