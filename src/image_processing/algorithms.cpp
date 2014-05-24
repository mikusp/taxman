#include "algorithms.h"

#include <QtGlobal>

void Algorithms::sobel(cv::InputArray in, cv::OutputArray out)
{
    Q_ASSERT(in.depth() == CV_8U);

    cv::Mat grad_x, grad_y;
    cv::Sobel(in, grad_x, CV_16S, 1, 0);
    cv::Sobel(in, grad_y, CV_16S, 0, 1);

    cv::Mat abs_grad_x, abs_grad_y;
    cv::convertScaleAbs(grad_x, abs_grad_x);
    cv::convertScaleAbs(grad_y, abs_grad_y);

    cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, out);
}

void Algorithms::scharr(cv::InputArray in, cv::OutputArray out)
{
    Q_ASSERT(in.depth() == CV_8U);

    cv::Mat grad_x, grad_y;
    cv::Scharr(in, grad_x, CV_16S, 1, 0);
    cv::Scharr(in, grad_y, CV_16S, 0, 1);

    cv::Mat abs_grad_x, abs_grad_y;
    cv::convertScaleAbs(grad_x, abs_grad_x);
    cv::convertScaleAbs(grad_y, abs_grad_y);

    cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, out);
}

void Algorithms::canny(cv::InputArray in, cv::OutputArray out, int lowThreshold, int highThreshold)
{
    cv::Canny(in, out, lowThreshold, highThreshold);
}

void Algorithms::skel(cv::InputArray& in, cv::OutputArray& out) {
    Q_ASSERT(in.depth() == CV_8U);

    auto tmp = in.getMat();
    cv::threshold(tmp, tmp, 127, 255, cv::THRESH_BINARY);
    cv::Mat skel(tmp.size(), CV_8UC1, cv::Scalar(0));
    cv::Mat temp;
    cv::Mat eroded;

    cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));

    bool done;
    do
    {
      cv::erode(tmp, eroded, element);
      cv::dilate(eroded, temp, element); // temp = open(img)
      cv::subtract(tmp, temp, temp);
      cv::bitwise_or(skel, temp, skel);
      eroded.copyTo(tmp);

      done = (cv::countNonZero(tmp) == 0);
    } while (!done);

    tmp.copyTo(out);
}
