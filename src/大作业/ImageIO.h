#pragma once

#include <opencv2/core.hpp>

#include <string>

namespace ImageIO
{
    cv::Mat LoadImage(const std::wstring& path, int flags);
    bool SaveImage(const std::wstring& path, const cv::Mat& image);
}
