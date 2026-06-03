#include "pch.h"
#include "ImageIO.h"

#include <opencv2/imgcodecs.hpp>

#include <fstream>
#include <vector>

namespace
{
    std::string ExtensionFromPath(const std::wstring& path)
    {
        const size_t dot = path.find_last_of(L'.');
        if (dot == std::wstring::npos)
        {
            return ".png";
        }

        const std::wstring wideExt = path.substr(dot);
        std::string extension;
        extension.reserve(wideExt.size());
        for (wchar_t ch : wideExt)
        {
            extension.push_back(ch <= 127 ? static_cast<char>(ch) : '?');
        }
        return extension;
    }
}

namespace ImageIO
{
    cv::Mat LoadImage(const std::wstring& path, int flags)
    {
        std::ifstream input(path, std::ios::binary);
        if (!input)
        {
            return {};
        }

        std::vector<unsigned char> buffer(
            (std::istreambuf_iterator<char>(input)),
            std::istreambuf_iterator<char>());
        if (buffer.empty())
        {
            return {};
        }

        return cv::imdecode(buffer, flags);
    }

    bool SaveImage(const std::wstring& path, const cv::Mat& image)
    {
        if (image.empty())
        {
            return false;
        }

        std::vector<unsigned char> buffer;
        if (!cv::imencode(ExtensionFromPath(path), image, buffer))
        {
            return false;
        }

        std::ofstream output(path, std::ios::binary);
        if (!output)
        {
            return false;
        }

        output.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
        return output.good();
    }
}
