#pragma once

#include <opencv2/core.hpp>

struct InfectionStats
{
    long long lungArea = 0;
    long long infectionArea = 0;
    double infectionRatio = 0.0;

    long long leftLungArea = 0;
    long long leftInfectionArea = 0;
    double leftRatio = 0.0;

    long long rightLungArea = 0;
    long long rightInfectionArea = 0;
    double rightRatio = 0.0;
};

class CInfectionAnalyzer
{
public:
    InfectionStats Analyze(const cv::Mat& lungMask, const cv::Mat& infectionMask) const;
    void SplitLeftRightLung(const cv::Mat& lungMask, cv::Mat& leftMask, cv::Mat& rightMask) const;
    cv::Mat MakeInfectionOverlay(const cv::Mat& source, const cv::Mat& lungMask, const cv::Mat& infectionMask) const;

private:
    cv::Mat NormalizeMask(const cv::Mat& mask, const cv::Size& size) const;
    double SafeRatio(long long numerator, long long denominator) const;
};
