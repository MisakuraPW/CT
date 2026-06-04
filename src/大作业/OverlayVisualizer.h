#pragma once

#include <opencv2/core.hpp>

class COverlayVisualizer
{
public:
	cv::Mat MakeMaskOverlay(
		const cv::Mat& source,
		const cv::Mat& mask,
		const cv::Scalar& color,
		double alpha = 0.45,
		bool drawContour = true) const;
	cv::Mat MakeComparisonOverlay(const cv::Mat& source, const cv::Mat& predictedMask, const cv::Mat& groundTruthMask) const;
	cv::Mat MakeConnectedComponentColorMap(const cv::Mat& binaryMask) const;

private:
	cv::Mat MakeGray8(const cv::Mat& source) const;
	cv::Mat NormalizeMask(const cv::Mat& mask, const cv::Size& size) const;
};
