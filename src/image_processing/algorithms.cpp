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
