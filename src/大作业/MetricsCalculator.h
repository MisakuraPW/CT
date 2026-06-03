#pragma once

#include <opencv2/core.hpp>

struct SegmentationMetrics
{
    long long tp = 0;
    long long fp = 0;
    long long tn = 0;
    long long fn = 0;
    double dice = 0.0;
    double iou = 0.0;
    double precision = 0.0;
    double recall = 0.0;
    double areaError = 0.0;
};

class CMetricsCalculator
{
public:
    SegmentationMetrics Calculate(const cv::Mat& predMask, const cv::Mat& gtMask) const;

private:
    cv::Mat NormalizeMask(const cv::Mat& mask, const cv::Size& size) const;
    double SafeDivide(double numerator, double denominator) const;
};
