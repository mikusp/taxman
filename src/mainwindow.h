#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <opencv2/opencv.hpp>

#include <memory>
#include <vector>
#include <tuple>

namespace Ui {
class MainWindow;
}

enum class AnswerDecodingState {
    IDLE,
    WAITING_FOR_1ST_CLICK,
    WAITING_FOR_2ND_CLICK
};

struct Point {
    int x;
    int y;
};

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

    void on_captureReferenceFrameButton_released();

    void on_startSurveyDecodingButton_released();

    void on_graphicsView_clicked(int x, int y);

    void on_actionZapisz_triggered();

    void on_openAnsweredSurveyButton_released();

private:
    void calculateAnswers();

    Ui::MainWindow *ui;
    QGraphicsScene scene;
    QTimer timer;
    QGraphicsPixmapItem pixmap;
    cv::VideoCapture videoCapture;

    cv::Mat referenceFrame, frame;

    int numberOfQuestions;
    int numberOfAnswers;

    AnswerDecodingState answerDecodingState;
    std::vector<std::tuple<int, int>> results;

    Point leftUpper;
    Point rightLower;
};

#endif // MAINWINDOW_H
