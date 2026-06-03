#include "pch.h"
#include "LungSegmenter.h"

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <numeric>
#include <vector>

namespace
{
    struct ComponentCandidate
    {
        int label = 0;
        int area = 0;
        double centerX = 0.0;
    };

    bool TouchesImageBorder(const cv::Mat& stats, int label, int width, int height)
    {
        const int x = stats.at<int>(label, cv::CC_STAT_LEFT);
        const int y = stats.at<int>(label, cv::CC_STAT_TOP);
        const int w = stats.at<int>(label, cv::CC_STAT_WIDTH);
        const int h = stats.at<int>(label, cv::CC_STAT_HEIGHT);
        return x <= 0 || y <= 0 || x + w >= width || y + h >= height;
    }

    void EnsureBinaryMask(cv::Mat& mask)
    {
        cv::threshold(mask, mask, 0, 255, cv::THRESH_BINARY);
    }
}

LungSegmentationResult CLungSegmenter::Segment(const cv::Mat& source) const
{
    LungSegmentationResult result;
    result.gray = MakeGray8(source);
    result.thresholdMask = ThresholdLungCandidates(result.gray);
    result.connectedMask = KeepMainComponents(result.thresholdMask);
    result.morphologyMask = RepairMask(result.connectedMask);
    result.finalMask = FillHoles(result.morphologyMask);
    return result;
}

cv::Mat CLungSegmenter::MakeGray8(const cv::Mat& source) const
{
    cv::Mat gray;
    if (source.channels() == 1)
    {
        gray = source.clone();
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

cv::Mat CLungSegmenter::ThresholdLungCandidates(const cv::Mat& gray) const
{
    if (gray.empty())
    {
        return {};
    }

    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0.0);

    cv::Mat mask;
    cv::threshold(blurred, mask, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    EnsureBinaryMask(mask);
    return mask;
}

cv::Mat CLungSegmenter::KeepMainComponents(const cv::Mat& binaryMask) const
{
    if (binaryMask.empty())
    {
        return {};
    }

    cv::Mat labels;
    cv::Mat stats;
    cv::Mat centroids;
    const int labelCount = cv::connectedComponentsWithStats(binaryMask, labels, stats, centroids, 8, CV_32S);

    const int imageArea = binaryMask.rows * binaryMask.cols;
    const int minArea = std::max(64, imageArea / 1000);

    std::vector<ComponentCandidate> candidates;
    for (int label = 1; label < labelCount; ++label)
    {
        const int area = stats.at<int>(label, cv::CC_STAT_AREA);
        if (area < minArea)
        {
            continue;
        }

        if (TouchesImageBorder(stats, label, binaryMask.cols, binaryMask.rows))
        {
            continue;
        }

        candidates.push_back({ label, area, centroids.at<double>(label, 0) });
    }

    if (candidates.empty())
    {
        for (int label = 1; label < labelCount; ++label)
        {
            const int area = stats.at<int>(label, cv::CC_STAT_AREA);
            if (area >= minArea)
            {
                candidates.push_back({ label, area, centroids.at<double>(label, 0) });
            }
        }
    }

    std::sort(candidates.begin(), candidates.end(), [](const ComponentCandidate& lhs, const ComponentCandidate& rhs) {
        return lhs.area > rhs.area;
    });

    cv::Mat output = cv::Mat::zeros(binaryMask.size(), CV_8UC1);
    const size_t keepCount = std::min<size_t>(2, candidates.size());
    for (size_t i = 0; i < keepCount; ++i)
    {
        output.setTo(255, labels == candidates[i].label);
    }

    EnsureBinaryMask(output);
    return output;
}

cv::Mat CLungSegmenter::RepairMask(const cv::Mat& binaryMask) const
{
    if (binaryMask.empty())
    {
        return {};
    }

    const cv::Mat openKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    const cv::Mat closeKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7, 7));

    cv::Mat repaired;
    cv::morphologyEx(binaryMask, repaired, cv::MORPH_OPEN, openKernel, cv::Point(-1, -1), 1);
    cv::morphologyEx(repaired, repaired, cv::MORPH_CLOSE, closeKernel, cv::Point(-1, -1), 1);
    EnsureBinaryMask(repaired);
    return repaired;
}

cv::Mat CLungSegmenter::FillHoles(const cv::Mat& binaryMask) const
{
    if (binaryMask.empty())
    {
        return {};
    }

    cv::Mat flood = binaryMask.clone();
    cv::floodFill(flood, cv::Point(0, 0), cv::Scalar(255));

    cv::Mat holes;
    cv::bitwise_not(flood, holes);

    cv::Mat filled = binaryMask | holes;
    EnsureBinaryMask(filled);
    return filled;
}
