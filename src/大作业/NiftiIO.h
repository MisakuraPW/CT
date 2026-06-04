#pragma once

#include <opencv2/core.hpp>

#include <string>
#include <vector>

struct NiftiVolume
{
    std::vector<cv::Mat> slices;
    int width = 0;
    int height = 0;
    int depth = 0;
    int datatype = 0;
    int bitpix = 0;
};

namespace NiftiIO
{
    bool IsNiftiPath(const std::wstring& path);
    bool LoadVolume(const std::wstring& path, NiftiVolume& volume, CString& errorMessage);
}
