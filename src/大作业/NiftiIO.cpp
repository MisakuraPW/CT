#include "pch.h"
#include "NiftiIO.h"

#include <Windows.h>

#include <algorithm>
#include <cstdint>
#include <cwctype>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>

namespace
{
    constexpr int kNiftiHeaderSize = 348;

    std::wstring ToLower(std::wstring value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](wchar_t ch) {
            return static_cast<wchar_t>(::towlower(ch));
        });
        return value;
    }

    bool EndsWith(const std::wstring& value, const std::wstring& suffix)
    {
        if (value.size() < suffix.size())
        {
            return false;
        }

        return std::equal(suffix.rbegin(), suffix.rend(), value.rbegin());
    }

    template <typename T>
    T ReadValue(const std::vector<unsigned char>& bytes, size_t offset)
    {
        T value{};
        std::memcpy(&value, bytes.data() + offset, sizeof(T));
        return value;
    }

    template <typename T>
    T SwapBytes(T value)
    {
        auto* ptr = reinterpret_cast<unsigned char*>(&value);
        std::reverse(ptr, ptr + sizeof(T));
        return value;
    }

    int CvTypeFromDatatype(int datatype)
    {
        switch (datatype)
        {
        case 2:
            return CV_8UC1;
        case 4:
            return CV_16SC1;
        case 8:
            return CV_32SC1;
        case 16:
            return CV_32FC1;
        case 64:
            return CV_64FC1;
        case 256:
            return CV_8SC1;
        case 512:
            return CV_16UC1;
        case 768:
            return CV_32SC1;
        default:
            return -1;
        }
    }

    int BytesPerVoxel(int datatype, int bitpix)
    {
        if (bitpix > 0)
        {
            return bitpix / 8;
        }

        switch (datatype)
        {
        case 2:
        case 256:
            return 1;
        case 4:
        case 512:
            return 2;
        case 8:
        case 16:
        case 768:
            return 4;
        case 64:
            return 8;
        default:
            return 0;
        }
    }

    std::wstring Quote(const std::wstring& value)
    {
        std::wstring escaped = L"\"";
        for (wchar_t ch : value)
        {
            if (ch == L'"')
            {
                escaped += L"\\\"";
            }
            else
            {
                escaped += ch;
            }
        }
        escaped += L"\"";
        return escaped;
    }

    bool RunProcess(const std::wstring& commandLine)
    {
        std::vector<wchar_t> buffer(commandLine.begin(), commandLine.end());
        buffer.push_back(L'\0');

        STARTUPINFOW startupInfo{};
        startupInfo.cb = sizeof(startupInfo);
        startupInfo.dwFlags = STARTF_USESHOWWINDOW;
        startupInfo.wShowWindow = SW_HIDE;

        PROCESS_INFORMATION processInfo{};
        if (!CreateProcessW(nullptr, buffer.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &processInfo))
        {
            return false;
        }

        WaitForSingleObject(processInfo.hProcess, INFINITE);

        DWORD exitCode = 1;
        GetExitCodeProcess(processInfo.hProcess, &exitCode);
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        return exitCode == 0;
    }

    bool CreateTempFilePath(std::wstring& path)
    {
        wchar_t tempPath[MAX_PATH]{};
        if (GetTempPathW(MAX_PATH, tempPath) == 0)
        {
            return false;
        }

        wchar_t tempFile[MAX_PATH]{};
        if (GetTempFileNameW(tempPath, L"nii", 0, tempFile) == 0)
        {
            return false;
        }

        path = tempFile;
        return true;
    }

    bool DecompressGzipWithPython(const std::wstring& sourcePath, std::wstring& outputPath)
    {
        if (!CreateTempFilePath(outputPath))
        {
            return false;
        }

        const std::wstring script =
            L"import gzip,shutil,sys;"
            L"src=gzip.open(sys.argv[1],'rb');"
            L"dst=open(sys.argv[2],'wb');"
            L"shutil.copyfileobj(src,dst);"
            L"src.close();dst.close()";

        const std::wstring args = L" -c " + Quote(script) + L" " + Quote(sourcePath) + L" " + Quote(outputPath);
        if (RunProcess(L"python" + args))
        {
            return true;
        }

        if (RunProcess(L"py -3" + args))
        {
            return true;
        }

        DeleteFileW(outputPath.c_str());
        outputPath.clear();
        return false;
    }

    bool ReadAllBytes(const std::wstring& path, std::vector<unsigned char>& bytes)
    {
        std::ifstream input(path, std::ios::binary);
        if (!input)
        {
            return false;
        }

        bytes.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
        return !bytes.empty();
    }
}

namespace NiftiIO
{
    bool IsNiftiPath(const std::wstring& path)
    {
        const std::wstring lower = ToLower(path);
        return EndsWith(lower, L".nii") || EndsWith(lower, L".nii.gz");
    }

    bool LoadVolume(const std::wstring& path, NiftiVolume& volume, CString& errorMessage)
    {
        volume = NiftiVolume{};
        errorMessage.Empty();

        std::wstring readPath = path;
        std::wstring tempPath;
        if (EndsWith(ToLower(path), L".nii.gz"))
        {
            if (!DecompressGzipWithPython(path, tempPath))
            {
                errorMessage = _T(".nii.gz 解压失败。请确认本机 PATH 中可调用 python 或 py，并且 Python 标准库 gzip 可用。");
                return false;
            }
            readPath = tempPath;
        }

        std::vector<unsigned char> bytes;
        if (!ReadAllBytes(readPath, bytes))
        {
            errorMessage = _T("NIfTI 文件读取失败。");
            if (!tempPath.empty())
            {
                DeleteFileW(tempPath.c_str());
            }
            return false;
        }

        if (!tempPath.empty())
        {
            DeleteFileW(tempPath.c_str());
        }

        if (bytes.size() < kNiftiHeaderSize)
        {
            errorMessage = _T("NIfTI 文件太小，无法读取头信息。");
            return false;
        }

        int sizeofHdr = ReadValue<int>(bytes, 0);
        bool needSwap = false;
        if (sizeofHdr != kNiftiHeaderSize)
        {
            sizeofHdr = SwapBytes(sizeofHdr);
            needSwap = sizeofHdr == kNiftiHeaderSize;
        }

        if (sizeofHdr != kNiftiHeaderSize)
        {
            errorMessage = _T("不是有效的 NIfTI-1 单文件格式。");
            return false;
        }

        auto readShort = [&](size_t offset) {
            short value = ReadValue<short>(bytes, offset);
            return needSwap ? SwapBytes(value) : value;
        };
        auto readFloat = [&](size_t offset) {
            float value = ReadValue<float>(bytes, offset);
            return needSwap ? SwapBytes(value) : value;
        };

        const short dimCount = readShort(40);
        const int width = readShort(42);
        const int height = readShort(44);
        const int depth = dimCount >= 3 ? std::max<int>(1, readShort(46)) : 1;
        const int datatype = readShort(70);
        const int bitpix = readShort(72);
        const float voxOffsetValue = readFloat(108);
        const size_t voxOffset = std::max<size_t>(352, static_cast<size_t>(voxOffsetValue));

        if (width <= 0 || height <= 0 || depth <= 0)
        {
            errorMessage = _T("NIfTI 维度无效。");
            return false;
        }

        const int cvType = CvTypeFromDatatype(datatype);
        const int bytesPerVoxel = BytesPerVoxel(datatype, bitpix);
        if (cvType < 0 || bytesPerVoxel <= 0)
        {
            CString message;
            message.Format(_T("暂不支持该 NIfTI datatype=%d bitpix=%d。"), datatype, bitpix);
            errorMessage = message;
            return false;
        }

        const size_t sliceBytes = static_cast<size_t>(width) * height * bytesPerVoxel;
        const size_t requiredBytes = voxOffset + sliceBytes * static_cast<size_t>(depth);
        if (bytes.size() < requiredBytes)
        {
            errorMessage = _T("NIfTI 数据区长度不足。");
            return false;
        }

        volume.width = width;
        volume.height = height;
        volume.depth = depth;
        volume.datatype = datatype;
        volume.bitpix = bitpix;
        volume.slices.reserve(depth);

        for (int z = 0; z < depth; ++z)
        {
            const size_t offset = voxOffset + sliceBytes * static_cast<size_t>(z);
            cv::Mat slice(height, width, cvType);
            std::memcpy(slice.data, bytes.data() + offset, sliceBytes);

            if (needSwap && bytesPerVoxel > 1)
            {
                for (size_t i = 0; i < sliceBytes; i += bytesPerVoxel)
                {
                    std::reverse(slice.data + i, slice.data + i + bytesPerVoxel);
                }
            }

            volume.slices.push_back(slice.clone());
        }

        return true;
    }
}
