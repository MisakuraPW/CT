#pragma once

#include "ProcessingOptions.h"

#include <opencv2/core.hpp>

struct LungSegmentationResult
{
    cv::Mat gray;
    cv::Mat thresholdMask;
    cv::Mat connectedMask;
    cv::Mat morphologyMask;
    cv::Mat finalMask;
};

class CLungSegmenter
{
public:
    LungSegmentationResult Segment(const cv::Mat& source, const LungSegmentationOptions& options = LungSegmentationOptions{}) const;

private:
    cv::Mat MakeGray8(const cv::Mat& source) const;
    cv::Mat ThresholdLungCandidates(const cv::Mat& gray, const LungSegmentationOptions& options) const;
    cv::Mat KeepMainComponents(const cv::Mat& binaryMask, const LungSegmentationOptions& options) const;
    cv::Mat RepairMask(const cv::Mat& binaryMask, const LungSegmentationOptions& options) const;
    cv::Mat FillHoles(const cv::Mat& binaryMask) const;
    int NormalizeOddKernelSize(int kernelSize) const;
};
