#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <opencv2/opencv.hpp>

class Algorithms
{
public:
    static void sobel(cv::InputArray in, cv::OutputArray out);
    static void scharr(cv::InputArray in, cv::OutputArray out);
    static void canny(cv::InputArray in, cv::OutputArray out, int lowThreshold, int highThreshold);
    static void skel(cv::InputArray in, cv::OutputArray out);
};

#endif // ALGORITHMS_H
