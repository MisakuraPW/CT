#include "pch.h"
#include "InfectionSegmenter.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <vector>

namespace
{
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

	cv::Mat MakeGray(const cv::Mat& source)
	{
		if (source.empty()) return {};
		if (source.channels() == 1) return source.clone();
		cv::Mat gray;
		cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);
		return gray;
	}
}

cv::Mat CInfectionSegmenter::SegmentByHUThreshold(const cv::Mat& source,
	const cv::Mat& lungMask) const
{
	cv::Mat gray = MakeGray(source);
	cv::Mat binaryLung;
	cv::threshold(lungMask, binaryLung, 127, 255, cv::THRESH_BINARY);

	std::string debugDir = "debug_output/";
	CreateDirectoryA(debugDir.c_str(), NULL);
	cv::imwrite(debugDir + "01_lung_mask.png", binaryLung);

	// 计算肺区像素统计
	std::vector<double> lungPixels;
	for (int r = 0; r < gray.rows; ++r)
		for (int c = 0; c < gray.cols; ++c)
			if (binaryLung.at<uchar>(r, c) > 0)
				lungPixels.push_back(GetPixelValue(gray, r, c));
	
	if (lungPixels.empty()) return {};
	std::sort(lungPixels.begin(), lungPixels.end());

	// ✅ 关键：使用较高的百分位(85%)作为血管阈值，中等百分位(55%)作为GGO阈值
	double vesselThresh = lungPixels[static_cast<size_t>(lungPixels.size() * 0.85)];
	double ggoThresh = lungPixels[static_cast<size_t>(lungPixels.size() * 0.55)];

	// 提取血管（高亮度区域）
	cv::Mat vessel(gray.size(), CV_8U, cv::Scalar(0));
	for (int r = 0; r < gray.rows; ++r)
		for (int c = 0; c < gray.cols; ++c)
			if (binaryLung.at<uchar>(r, c) > 0 && GetPixelValue(gray, r, c) >= vesselThresh)
				vessel.at<uchar>(r, c) = 255;
	
	// 膨胀血管确保覆盖
	cv::Mat k3 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
	cv::dilate(vessel, vessel, k3);
	cv::imwrite(debugDir + "02_vessel.png", vessel);

	// 提取GGO候选（中等亮度区域）
	cv::Mat ggo(gray.size(), CV_8U, cv::Scalar(0));
	for (int r = 0; r < gray.rows; ++r)
		for (int c = 0; c < gray.cols; ++c)
			if (binaryLung.at<uchar>(r, c) > 0 && GetPixelValue(gray, r, c) >= ggoThresh)
				ggo.at<uchar>(r, c) = 255;
	cv::imwrite(debugDir + "03_ggo_candidate.png", ggo);

	// 减法：GGO - 血管
	cv::Mat result(gray.size(), CV_8U, cv::Scalar(0));
	for (int r = 0; r < gray.rows; ++r)
		for (int c = 0; c < gray.cols; ++c)
			if (ggo.at<uchar>(r, c) > 0 && vessel.at<uchar>(r, c) == 0)
				result.at<uchar>(r, c) = 255;
	cv::imwrite(debugDir + "04_after_subtraction.png", result);

	// 形态学处理：开运算去噪 + 闭运算桥接
	cv::Mat k5 = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
	cv::morphologyEx(result, result, cv::MORPH_OPEN, k3);
	cv::morphologyEx(result, result, cv::MORPH_CLOSE, k5);
	cv::imwrite(debugDir + "05_after_morphology.png", result);

	// 连通域过滤：去除过小区域
	cv::Mat labels, stats;
	int nLabels = cv::connectedComponentsWithStats(result, labels, stats, cv::noArray(), 8, CV_32S);
	
	cv::Mat final(gray.size(), CV_8U, cv::Scalar(0));
	for (int i = 1; i < nLabels; ++i)
		if (stats.at<int>(i, cv::CC_STAT_AREA) >= 50)
		{
			cv::Mat mask = (labels == i);
			mask.convertTo(mask, CV_8UC1, 255.0);
			cv::bitwise_or(final, mask, final);
		}
	
	cv::imwrite(debugDir + "06_final_result.png", final);
	return final;
}

InfectionSegmentationResult CInfectionSegmenter::Segment(const cv::Mat& source,
	const cv::Mat& lungMask, double windowLevel, double windowWidth) const
{
	InfectionSegmentationResult result;
	if (source.empty() || lungMask.empty()) return result;
	
	// Apply lung window
	cv::Mat gray = MakeGray(source);
	double lower = windowLevel - windowWidth / 2.0;
	double scale = 255.0 / windowWidth;
	cv::Mat lungWindow;
	gray.convertTo(lungWindow, CV_32F);
	cv::subtract(lungWindow, cv::Scalar(lower), lungWindow);
	cv::multiply(lungWindow, cv::Scalar(scale), lungWindow);
	cv::threshold(lungWindow, lungWindow, 255.0, 255.0, cv::THRESH_TRUNC);
	cv::threshold(lungWindow, lungWindow, 0.0, 0.0, cv::THRESH_TOZERO);
	lungWindow.convertTo(result.lungWindow, CV_8U);
	
	// Segment infection
	result.infectionMask = SegmentByHUThreshold(source, lungMask);
	return result;
}
