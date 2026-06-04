#include "pch.h"
#include "Preprocessor.h"

#include <opencv2/imgproc.hpp>

#include <algorithm>

cv::Mat CPreprocessor::MakeGray8(const cv::Mat& source) const
{
	if (source.empty())
	{
		return {};
	}

	cv::Mat gray;
	if (source.channels() == 1)
	{
		gray = source;
	}
	else if (source.channels() == 3)
	{
		cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
	}
	else if (source.channels() == 4)
	{
		cv::cvtColor(source, gray, cv::COLOR_BGRA2GRAY);
	}

	if (gray.empty())
	{
		return {};
	}

	cv::Mat gray8;
	if (gray.depth() == CV_8U)
	{
		gray8 = gray;
	}
	else
	{
		cv::normalize(gray, gray8, 0, 255, cv::NORM_MINMAX, CV_8U);
	}

	return gray8.clone();
}

cv::Mat CPreprocessor::ApplyWindow(const cv::Mat& source, double windowLevel, double windowWidth) const
{
	if (source.empty() || windowWidth <= 0.0)
	{
		return {};
	}

	cv::Mat gray;
	if (source.channels() == 1)
	{
		gray = source;
	}
	else if (source.channels() == 3)
	{
		cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
	}
	else if (source.channels() == 4)
	{
		cv::cvtColor(source, gray, cv::COLOR_BGRA2GRAY);
	}

	if (gray.empty())
	{
		return {};
	}

	const double lower = windowLevel - windowWidth / 2.0;
	const double scale = 255.0 / windowWidth;

	cv::Mat floatImage;
	gray.convertTo(floatImage, CV_32F);
	floatImage = (floatImage - lower) * scale;

	cv::Mat clipped;
	cv::threshold(floatImage, clipped, 255.0, 255.0, cv::THRESH_TRUNC);
	cv::threshold(clipped, clipped, 0.0, 0.0, cv::THRESH_TOZERO);

	cv::Mat output;
	clipped.convertTo(output, CV_8U);
	return output;
}

cv::Mat CPreprocessor::ApplyLungWindow(const cv::Mat& source) const
{
	return ApplyWindow(source, -600.0, 1500.0);
}

cv::Mat CPreprocessor::ApplyGaussianBlur(const cv::Mat& source, int kernelSize) const
{
	const cv::Mat gray = MakeGray8(source);
	if (gray.empty())
	{
		return {};
	}

	cv::Mat output;
	const int normalizedKernel = NormalizeOddKernelSize(kernelSize);
	cv::GaussianBlur(gray, output, cv::Size(normalizedKernel, normalizedKernel), 0.0);
	return output;
}

cv::Mat CPreprocessor::ApplyMedianBlur(const cv::Mat& source, int kernelSize) const
{
	const cv::Mat gray = MakeGray8(source);
	if (gray.empty())
	{
		return {};
	}

	cv::Mat output;
	cv::medianBlur(gray, output, NormalizeOddKernelSize(kernelSize));
	return output;
}

cv::Mat CPreprocessor::ApplyClahe(const cv::Mat& source, double clipLimit, const cv::Size& tileGridSize) const
{
	const cv::Mat gray = MakeGray8(source);
	if (gray.empty())
	{
		return {};
	}

	cv::Mat output;
	const cv::Size normalizedTileGridSize(std::max(1, tileGridSize.width), std::max(1, tileGridSize.height));
	cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(std::max(0.1, clipLimit), normalizedTileGridSize);
	clahe->apply(gray, output);
	return output;
}

int CPreprocessor::NormalizeOddKernelSize(int kernelSize) const
{
	int normalized = std::max(3, kernelSize);
	if (normalized % 2 == 0)
	{
		++normalized;
	}
	return normalized;
}
