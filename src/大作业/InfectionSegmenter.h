#pragma once

#include <opencv2/core.hpp>

struct InfectionSegmentationResult
{
	cv::Mat lungWindow;
	cv::Mat infectionMask;
};

class CInfectionSegmenter
{
public:
	InfectionSegmentationResult Segment(const cv::Mat& source,
		const cv::Mat& lungMask,
		double windowLevel = -600.0,
		double windowWidth = 1500.0) const;

private:
	cv::Mat ApplyLungWindow(const cv::Mat& source,
		double windowLevel, double windowWidth) const;
	cv::Mat SegmentByHUThreshold(const cv::Mat& source,
		const cv::Mat& lungMask) const;
};
