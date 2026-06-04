#include "pch.h"
#include "OverlayVisualizer.h"

#include <opencv2/imgproc.hpp>

#include <vector>

cv::Mat COverlayVisualizer::MakeMaskOverlay(
	const cv::Mat& source,
	const cv::Mat& mask,
	const cv::Scalar& color,
	double alpha,
	bool drawContour) const
{
	if (source.empty() || mask.empty())
	{
		return {};
	}

	const cv::Mat gray8 = MakeGray8(source);
	const cv::Mat binary = NormalizeMask(mask, gray8.size());
	if (gray8.empty() || binary.empty())
	{
		return {};
	}

	cv::Mat overlay;
	cv::cvtColor(gray8, overlay, cv::COLOR_GRAY2BGR);

	cv::Mat colorLayer = overlay.clone();
	colorLayer.setTo(color, binary);
	cv::addWeighted(colorLayer, alpha, overlay, 1.0 - alpha, 0.0, overlay);

	if (drawContour)
	{
		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		cv::drawContours(overlay, contours, -1, color, 1);
	}

	return overlay;
}

cv::Mat COverlayVisualizer::MakeComparisonOverlay(const cv::Mat& source, const cv::Mat& predictedMask, const cv::Mat& groundTruthMask) const
{
	if (source.empty() || predictedMask.empty() || groundTruthMask.empty())
	{
		return {};
	}

	const cv::Mat gray8 = MakeGray8(source);
	const cv::Mat predicted = NormalizeMask(predictedMask, gray8.size());
	const cv::Mat groundTruth = NormalizeMask(groundTruthMask, gray8.size());
	if (gray8.empty() || predicted.empty() || groundTruth.empty())
	{
		return {};
	}

	cv::Mat overlay;
	cv::cvtColor(gray8, overlay, cv::COLOR_GRAY2BGR);

	cv::Mat overlap;
	cv::Mat predictedOnly;
	cv::Mat groundTruthOnly;
	cv::Mat notPredicted;
	cv::Mat notGroundTruth;
	cv::bitwise_and(predicted, groundTruth, overlap);
	cv::bitwise_not(predicted, notPredicted);
	cv::bitwise_not(groundTruth, notGroundTruth);
	cv::bitwise_and(predicted, notGroundTruth, predictedOnly);
	cv::bitwise_and(groundTruth, notPredicted, groundTruthOnly);

	cv::Mat colorLayer = overlay.clone();
	colorLayer.setTo(cv::Scalar(0, 180, 0), predictedOnly);
	colorLayer.setTo(cv::Scalar(255, 0, 0), groundTruthOnly);
	colorLayer.setTo(cv::Scalar(0, 255, 255), overlap);
	cv::addWeighted(colorLayer, 0.55, overlay, 0.45, 0.0, overlay);

	std::vector<std::vector<cv::Point>> predictedContours;
	std::vector<std::vector<cv::Point>> groundTruthContours;
	cv::findContours(predicted, predictedContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	cv::findContours(groundTruth, groundTruthContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	cv::drawContours(overlay, predictedContours, -1, cv::Scalar(0, 255, 0), 1);
	cv::drawContours(overlay, groundTruthContours, -1, cv::Scalar(255, 0, 0), 1);

	return overlay;
}

cv::Mat COverlayVisualizer::MakeConnectedComponentColorMap(const cv::Mat& binaryMask) const
{
	if (binaryMask.empty())
	{
		return {};
	}

	const cv::Mat binary = NormalizeMask(binaryMask, binaryMask.size());
	cv::Mat labels;
	const int labelCount = cv::connectedComponents(binary, labels, 8, CV_32S);

	cv::Mat colorMap = cv::Mat::zeros(binary.size(), CV_8UC3);
	for (int label = 1; label < labelCount; ++label)
	{
		const cv::Scalar color(
			(label * 53) % 220 + 35,
			(label * 97) % 220 + 35,
			(label * 151) % 220 + 35);
		colorMap.setTo(color, labels == label);
	}

	return colorMap;
}

cv::Mat COverlayVisualizer::MakeGray8(const cv::Mat& source) const
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

cv::Mat COverlayVisualizer::NormalizeMask(const cv::Mat& mask, const cv::Size& size) const
{
	if (mask.empty())
	{
		return {};
	}

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

	if (gray.empty())
	{
		return {};
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

	cv::Mat gray8;
	if (resized.depth() == CV_8U)
	{
		gray8 = resized;
	}
	else
	{
		cv::normalize(resized, gray8, 0, 255, cv::NORM_MINMAX, CV_8U);
	}

	cv::Mat binary;
	cv::threshold(gray8, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
	return binary;
}
