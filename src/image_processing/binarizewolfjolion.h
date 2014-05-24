#ifndef BINARIZEWOLFJOLION_H
#define BINARIZEWOLFJOLION_H

#include <opencv2/opencv.hpp>

enum NiblackVersion
{
    NIBLACK=0,
    SAUVOLA,
    WOLFJOLION,
};


void NiblackSauvolaWolfJolion(cv::Mat in, cv::Mat out, NiblackVersion version,
                             int winx, int winy, double k, double dR);

#endif // BINARIZEWOLFJOLION_H
