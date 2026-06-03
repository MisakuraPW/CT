#include "pch.h"
#include "MetricsCalculator.h"

#include <opencv2/imgproc.hpp>

#include <cmath>

SegmentationMetrics CMetricsCalculator::Calculate(const cv::Mat& predMask, const cv::Mat& gtMask) const
{
    SegmentationMetrics metrics;
    if (predMask.empty() || gtMask.empty())
    {
        return metrics;
    }

    const cv::Mat pred = NormalizeMask(predMask, predMask.size());
    const cv::Mat gt = NormalizeMask(gtMask, predMask.size());

    for (int y = 0; y < pred.rows; ++y)
    {
        const uchar* predRow = pred.ptr<uchar>(y);
        const uchar* gtRow = gt.ptr<uchar>(y);
        for (int x = 0; x < pred.cols; ++x)
        {
            const bool p = predRow[x] != 0;
            const bool g = gtRow[x] != 0;
            if (p && g)
            {
                ++metrics.tp;
            }
            else if (p && !g)
            {
                ++metrics.fp;
            }
            else if (!p && g)
            {
                ++metrics.fn;
            }
            else
            {
                ++metrics.tn;
            }
        }
    }

    const double tp = static_cast<double>(metrics.tp);
    const double fp = static_cast<double>(metrics.fp);
    const double fn = static_cast<double>(metrics.fn);
    const double predArea = tp + fp;
    const double gtArea = tp + fn;

    metrics.dice = SafeDivide(2.0 * tp, 2.0 * tp + fp + fn);
    metrics.iou = SafeDivide(tp, tp + fp + fn);
    metrics.precision = SafeDivide(tp, tp + fp);
    metrics.recall = SafeDivide(tp, tp + fn);
    metrics.areaError = SafeDivide(std::abs(predArea - gtArea), gtArea);
    return metrics;
}

cv::Mat CMetricsCalculator::NormalizeMask(const cv::Mat& mask, const cv::Size& size) const
{
    cv::Mat gray;
    if (mask.channels() == 1)
    {
        gray = mask;
    }
    else if (mask.channels() == 3)
    {
        cv::cvtColor(mask, gray, cv::COLOR_BGR2GRAY);
    }
    else if (mask.channels() == 4)
    {
        cv::cvtColor(mask, gray, cv::COLOR_BGRA2GRAY);
    }

    cv::Mat resized;
    if (gray.size() == size)
    {
        resized = gray;
    }
    else
    {
        cv::resize(gray, resized, size, 0.0, 0.0, cv::INTER_NEAREST);
    }

    cv::Mat binary;
    cv::threshold(resized, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    return binary;
}

double CMetricsCalculator::SafeDivide(double numerator, double denominator) const
{
    if (denominator == 0.0)
    {
        return 0.0;
    }

    return numerator / denominator;
}
