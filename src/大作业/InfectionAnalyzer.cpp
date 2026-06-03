#include "pch.h"
#include "InfectionAnalyzer.h"

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <vector>

namespace
{
    struct ComponentInfo
    {
        int label = 0;
        int area = 0;
        double centerX = 0.0;
    };
}

InfectionStats CInfectionAnalyzer::Analyze(const cv::Mat& lungMask, const cv::Mat& infectionMask) const
{
    InfectionStats stats;
    if (lungMask.empty() || infectionMask.empty())
    {
        return stats;
    }

    const cv::Mat lung = NormalizeMask(lungMask, lungMask.size());
    const cv::Mat infection = NormalizeMask(infectionMask, lungMask.size());

    cv::Mat infectionInLung;
    cv::bitwise_and(infection, lung, infectionInLung);

    cv::Mat leftLung;
    cv::Mat rightLung;
    SplitLeftRightLung(lung, leftLung, rightLung);

    cv::Mat leftInfection;
    cv::Mat rightInfection;
    cv::bitwise_and(infectionInLung, leftLung, leftInfection);
    cv::bitwise_and(infectionInLung, rightLung, rightInfection);

    stats.lungArea = cv::countNonZero(lung);
    stats.infectionArea = cv::countNonZero(infectionInLung);
    stats.infectionRatio = SafeRatio(stats.infectionArea, stats.lungArea);

    stats.leftLungArea = cv::countNonZero(leftLung);
    stats.leftInfectionArea = cv::countNonZero(leftInfection);
    stats.leftRatio = SafeRatio(stats.leftInfectionArea, stats.leftLungArea);

    stats.rightLungArea = cv::countNonZero(rightLung);
    stats.rightInfectionArea = cv::countNonZero(rightInfection);
    stats.rightRatio = SafeRatio(stats.rightInfectionArea, stats.rightLungArea);

    return stats;
}

void CInfectionAnalyzer::SplitLeftRightLung(const cv::Mat& lungMask, cv::Mat& leftMask, cv::Mat& rightMask) const
{
    leftMask = cv::Mat::zeros(lungMask.size(), CV_8UC1);
    rightMask = cv::Mat::zeros(lungMask.size(), CV_8UC1);
    if (lungMask.empty())
    {
        return;
    }

    const cv::Mat lung = NormalizeMask(lungMask, lungMask.size());
    cv::Mat labels;
    cv::Mat stats;
    cv::Mat centroids;
    const int count = cv::connectedComponentsWithStats(lung, labels, stats, centroids, 8, CV_32S);

    std::vector<ComponentInfo> components;
    for (int label = 1; label < count; ++label)
    {
        const int area = stats.at<int>(label, cv::CC_STAT_AREA);
        if (area > 0)
        {
            components.push_back({ label, area, centroids.at<double>(label, 0) });
        }
    }

    std::sort(components.begin(), components.end(), [](const ComponentInfo& lhs, const ComponentInfo& rhs) {
        return lhs.area > rhs.area;
    });

    if (components.size() >= 2)
    {
        ComponentInfo first = components[0];
        ComponentInfo second = components[1];
        if (first.centerX > second.centerX)
        {
            std::swap(first, second);
        }

        leftMask.setTo(255, labels == first.label);
        rightMask.setTo(255, labels == second.label);
        return;
    }

    const int midX = lung.cols / 2;
    leftMask(cv::Rect(0, 0, midX, lung.rows)).setTo(255, lung(cv::Rect(0, 0, midX, lung.rows)));
    rightMask(cv::Rect(midX, 0, lung.cols - midX, lung.rows)).setTo(255, lung(cv::Rect(midX, 0, lung.cols - midX, lung.rows)));
}

cv::Mat CInfectionAnalyzer::MakeInfectionOverlay(const cv::Mat& source, const cv::Mat& lungMask, const cv::Mat& infectionMask) const
{
    if (source.empty() || lungMask.empty() || infectionMask.empty())
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

    cv::Mat gray8;
    if (gray.depth() == CV_8U)
    {
        gray8 = gray;
    }
    else
    {
        cv::normalize(gray, gray8, 0, 255, cv::NORM_MINMAX, CV_8U);
    }

    cv::Mat overlay;
    cv::cvtColor(gray8, overlay, cv::COLOR_GRAY2BGR);

    const cv::Mat lung = NormalizeMask(lungMask, gray8.size());
    const cv::Mat infection = NormalizeMask(infectionMask, gray8.size());
    cv::Mat infectionInLung;
    cv::bitwise_and(infection, lung, infectionInLung);

    cv::Mat redLayer = overlay.clone();
    redLayer.setTo(cv::Scalar(0, 0, 255), infectionInLung);
    cv::addWeighted(redLayer, 0.45, overlay, 0.55, 0.0, overlay);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(lung, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::drawContours(overlay, contours, -1, cv::Scalar(0, 255, 0), 1);
    return overlay;
}

cv::Mat CInfectionAnalyzer::NormalizeMask(const cv::Mat& mask, const cv::Size& size) const
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

double CInfectionAnalyzer::SafeRatio(long long numerator, long long denominator) const
{
    if (denominator == 0)
    {
        return 0.0;
    }

    return static_cast<double>(numerator) / static_cast<double>(denominator);
}
