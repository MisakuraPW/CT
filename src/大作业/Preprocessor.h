#pragma once

#include <opencv2/core.hpp>

class CPreprocessor
{
public:
	cv::Mat MakeGray8(const cv::Mat& source) const;
	cv::Mat ApplyWindow(const cv::Mat& source, double windowLevel, double windowWidth) const;
	cv::Mat ApplyLungWindow(const cv::Mat& source) const;
	cv::Mat ApplyGaussianBlur(const cv::Mat& source, int kernelSize = 5) const;
	cv::Mat ApplyMedianBlur(const cv::Mat& source, int kernelSize = 5) const;
	cv::Mat ApplyClahe(const cv::Mat& source, double clipLimit = 2.0, const cv::Size& tileGridSize = cv::Size(8, 8)) const;

private:
	int NormalizeOddKernelSize(int kernelSize) const;
};
