#include "pch.h"
#include "InfectionSegmenter.h"

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <vector>

namespace
{
	cv::Mat MakeGray(const cv::Mat& source)
	{
		if (source.empty())
			return {};

		if (source.channels() == 1)
			return source.clone();

		cv::Mat gray;
		cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
		return gray;
	}

	double GetPixelValue(const cv::Mat& image, int r, int c)
	{
		int type = image.type();
		if (type == CV_8U)  return static_cast<double>(image.at<uchar>(r, c));
		if (type == CV_8S)  return static_cast<double>(image.at<char>(r, c));
		if (type == CV_16U) return static_cast<double>(image.at<ushort>(r, c));
		if (type == CV_16S) return static_cast<double>(image.at<short>(r, c));
		if (type == CV_32S) return static_cast<double>(image.at<int>(r, c));
		if (type == CV_32F) return static_cast<double>(image.at<float>(r, c));
		if (type == CV_64F) return image.at<double>(r, c);
		return 0.0;
	}
}

cv::Mat CInfectionSegmenter::ApplyLungWindow(const cv::Mat& source,
	double windowLevel, double windowWidth) const
{
	cv::Mat gray = MakeGray(source);

	double lower = windowLevel - windowWidth / 2.0;
	double scale = 255.0 / windowWidth;

	cv::Mat result;
	gray.convertTo(result, CV_32F);

	cv::subtract(result, cv::Scalar(lower), result);
	cv::multiply(result, cv::Scalar(scale), result);

	cv::Mat clipped;
	cv::threshold(result, clipped, 255.0, 255.0, cv::THRESH_TRUNC);
	cv::threshold(clipped, clipped, 0.0, 0.0, cv::THRESH_TOZERO);

	cv::Mat out;
	clipped.convertTo(out, CV_8U);
	return out;
}

cv::Mat CInfectionSegmenter::SegmentByHUThreshold(const cv::Mat& source,
	const cv::Mat& lungMask) const
{
	cv::Mat gray = MakeGray(source);
	cv::Mat binaryLung;
	cv::threshold(lungMask, binaryLung, 127, 255, cv::THRESH_BINARY);

	// 收集肺内所有像素值
	std::vector<double> lungPixels;
	lungPixels.reserve(static_cast<size_t>(gray.rows) * gray.cols);

	for (int r = 0; r < gray.rows; ++r)
	{
		for (int c = 0; c < gray.cols; ++c)
		{
			if (binaryLung.at<uchar>(r, c) == 0)
				continue;

			double v = GetPixelValue(gray, r, c);
			lungPixels.push_back(v);
		}
	}

	if (lungPixels.empty())
		return {};

	// 第 60 百分位数作为正常组织上限，高于此值为潜在感染区域（含血管）
	std::sort(lungPixels.begin(), lungPixels.end());
	size_t idx60 = static_cast<size_t>(lungPixels.size() * 0.60);
	double threshold60 = lungPixels[idx60];

	// 创建感染 mask
	cv::Mat infection(gray.size(), CV_8U, cv::Scalar(0));
	for (int r = 0; r < gray.rows; ++r)
	{
		for (int c = 0; c < gray.cols; ++c)
		{
			if (binaryLung.at<uchar>(r, c) == 0)
				continue;

			double v = GetPixelValue(gray, r, c);
			if (v > threshold60)
				infection.at<uchar>(r, c) = 255;
		}
	}

	// 形态学清理
	cv::Mat kernel5 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
	cv::Mat kernel7 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7, 7));

	// 开运算去除细小孤立噪点
	cv::morphologyEx(infection, infection, cv::MORPH_OPEN, kernel5);

	// 连通域分析：移除面积过小和过于细长的成分（血管特征）
	cv::Mat labels, stats, centroids;
	int nLabels = cv::connectedComponentsWithStats(infection, labels, stats, centroids, 8, CV_32S);

	cv::Mat cleaned = cv::Mat::zeros(infection.size(), CV_8U);
	for (int label = 1; label < nLabels; ++label)
	{
		int area = stats.at<int>(label, cv::CC_STAT_AREA);
		int width = stats.at<int>(label, cv::CC_STAT_WIDTH);
		int height = stats.at<int>(label, cv::CC_STAT_HEIGHT);

		double aspectRatio = (width > 0 && height > 0)
			? static_cast<double>(std::max(width, height)) / std::min(width, height)
			: 99.0;

		double density = (width > 0 && height > 0)
			? static_cast<double>(area) / (width * height)
			: 0.0;

		// 过滤血管状成分：细长且低填充率，面积不太大
		bool vesselLike = (aspectRatio > 4.0 && density < 0.5 && area < 500);

		if (area >= 30 && !vesselLike)
		{
			cv::Mat componentMask = (labels == label);
			cv::bitwise_or(cleaned, componentMask * 255, cleaned);
		}
	}

	// 闭运算填充孔洞
	cv::morphologyEx(cleaned, cleaned, cv::MORPH_CLOSE, kernel7);

	return cleaned;
}

InfectionSegmentationResult CInfectionSegmenter::Segment(const cv::Mat& source,
	const cv::Mat& lungMask,
	double windowLevel, double windowWidth) const
{
	InfectionSegmentationResult result;

	if (source.empty() || lungMask.empty())
		return result;

	result.lungWindow = ApplyLungWindow(source, windowLevel, windowWidth);
	result.infectionMask = SegmentByHUThreshold(source, lungMask);

	return result;
}
