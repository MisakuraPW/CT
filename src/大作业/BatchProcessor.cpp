#include "pch.h"
#include "BatchProcessor.h"

#include "ImageIO.h"
#include "InfectionAnalyzer.h"
#include "LungSegmenter.h"
#include "MetricsCalculator.h"
#include "OverlayVisualizer.h"

#include <Windows.h>

#include <opencv2/imgproc.hpp>

#include <fstream>
#include <iomanip>
#include <sstream>

namespace
{
    bool DirectoryExists(const std::wstring& path)
    {
        const DWORD attrs = GetFileAttributesW(path.c_str());
        return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY);
    }

    bool EnsureDirectory(const std::wstring& path)
    {
        if (DirectoryExists(path))
        {
            return true;
        }

        return CreateDirectoryW(path.c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS;
    }
}

bool CBatchProcessor::ProcessSlices(
    const std::vector<cv::Mat>& sourceSlices,
    const std::vector<cv::Mat>& gtMaskSlices,
    const std::vector<cv::Mat>& infectionMaskSlices,
    const BatchOptions& options,
    BatchProcessResult& result,
    CString& errorMessage) const
{
    result = BatchProcessResult{};
    errorMessage.Empty();

    if (sourceSlices.empty())
    {
        errorMessage = _T("没有可批量处理的 CT 切片。");
        return false;
    }

    if (!EnsureOutputDirectories(options.outputRoot, errorMessage))
    {
        return false;
    }

    const std::wstring manifestPath = JoinPath(JoinPath(options.outputRoot, L"csv"), L"run_manifest.txt");
    if (!WriteRunManifest(
        manifestPath,
        options,
        static_cast<int>(sourceSlices.size()),
        static_cast<int>(gtMaskSlices.size()),
        static_cast<int>(infectionMaskSlices.size()),
        errorMessage))
    {
        return false;
    }

    CLungSegmenter segmenter;
    CMetricsCalculator metricsCalculator;
    CInfectionAnalyzer infectionAnalyzer;

    std::vector<SegmentationMetrics> metricsRows(sourceSlices.size());
    std::vector<bool> hasMetrics(sourceSlices.size(), false);
    std::vector<InfectionStats> infectionRows(sourceSlices.size());
    std::vector<bool> hasInfection(sourceSlices.size(), false);

    result.totalSlices = static_cast<int>(sourceSlices.size());

    for (size_t i = 0; i < sourceSlices.size(); ++i)
    {
        if (sourceSlices[i].empty())
        {
            continue;
        }

        const LungSegmentationResult segmentation = segmenter.Segment(sourceSlices[i], options.segmentationOptions);
        if (segmentation.finalMask.empty())
        {
            continue;
        }

        COverlayVisualizer visualizer;
        const cv::Mat connectedColorMap = visualizer.MakeConnectedComponentColorMap(segmentation.thresholdMask);
        cv::Mat maskComparisonOverlay;
        cv::Mat infectionOverlay;
        if (i < infectionMaskSlices.size() && !infectionMaskSlices[i].empty())
        {
            const cv::Mat infectionMask = NormalizeMask(infectionMaskSlices[i], segmentation.finalMask.size());
            infectionRows[i] = infectionAnalyzer.Analyze(segmentation.finalMask, infectionMask);
            hasInfection[i] = true;
            infectionOverlay = infectionAnalyzer.MakeInfectionOverlay(sourceSlices[i], segmentation.finalMask, infectionMask);
            ++result.infectionSlices;
        }

        if (i < gtMaskSlices.size() && !gtMaskSlices[i].empty())
        {
            const cv::Mat gtMask = NormalizeMask(gtMaskSlices[i], segmentation.finalMask.size());
            metricsRows[i] = metricsCalculator.Calculate(segmentation.finalMask, gtMask);
            maskComparisonOverlay = visualizer.MakeComparisonOverlay(sourceSlices[i], segmentation.finalMask, gtMask);
            hasMetrics[i] = true;
            ++result.metricSlices;
        }

        if (!SaveSliceImages(
            options.outputRoot,
            static_cast<int>(i),
            segmentation.finalMask,
            segmentation.thresholdMask,
            segmentation.connectedMask,
            segmentation.morphologyMask,
            connectedColorMap,
            maskComparisonOverlay,
            infectionOverlay,
            options.saveIntermediate))
        {
            errorMessage = _T("批量处理结果图像保存失败。");
            return false;
        }

        ++result.processedSlices;
    }

    const std::wstring csvPath = JoinPath(JoinPath(options.outputRoot, L"csv"), L"batch_summary.csv");
    if (!WriteSummaryCsv(csvPath, metricsRows, hasMetrics, infectionRows, hasInfection, errorMessage))
    {
        return false;
    }

    result.summaryCsvPath = csvPath.c_str();
    result.manifestPath = manifestPath.c_str();
    return true;
}

bool CBatchProcessor::EnsureOutputDirectories(const std::wstring& outputRoot, CString& errorMessage) const
{
    if (outputRoot.empty())
    {
        errorMessage = _T("输出目录为空。");
        return false;
    }

    const std::wstring dirs[] = {
        outputRoot,
        JoinPath(outputRoot, L"masks"),
        JoinPath(outputRoot, L"intermediate"),
        JoinPath(outputRoot, L"overlays"),
        JoinPath(outputRoot, L"csv")
    };

    for (const auto& dir : dirs)
    {
        if (!EnsureDirectory(dir))
        {
            CString message;
            message.Format(_T("无法创建输出目录：%s"), dir.c_str());
            errorMessage = message;
            return false;
        }
    }

    return true;
}

bool CBatchProcessor::SaveSliceImages(
    const std::wstring& outputRoot,
    int sliceIndex,
    const cv::Mat& finalMask,
    const cv::Mat& thresholdMask,
    const cv::Mat& connectedMask,
    const cv::Mat& morphologyMask,
    const cv::Mat& connectedColorMap,
    const cv::Mat& maskComparisonOverlay,
    const cv::Mat& infectionOverlay,
    bool saveIntermediate) const
{
    if (!ImageIO::SaveImage(JoinPath(JoinPath(outputRoot, L"masks"), SliceName(sliceIndex, L"_final_mask.png")), finalMask))
    {
        return false;
    }

    if (saveIntermediate)
    {
        if (!thresholdMask.empty() &&
            !ImageIO::SaveImage(JoinPath(JoinPath(outputRoot, L"intermediate"), SliceName(sliceIndex, L"_threshold.png")), thresholdMask))
        {
            return false;
        }
        if (!connectedMask.empty() &&
            !ImageIO::SaveImage(JoinPath(JoinPath(outputRoot, L"intermediate"), SliceName(sliceIndex, L"_connected.png")), connectedMask))
        {
            return false;
        }
        if (!morphologyMask.empty() &&
            !ImageIO::SaveImage(JoinPath(JoinPath(outputRoot, L"intermediate"), SliceName(sliceIndex, L"_morphology.png")), morphologyMask))
        {
            return false;
        }
    }

    if (!connectedColorMap.empty() &&
        !ImageIO::SaveImage(JoinPath(JoinPath(outputRoot, L"overlays"), SliceName(sliceIndex, L"_components_color.png")), connectedColorMap))
    {
        return false;
    }

    if (!maskComparisonOverlay.empty() &&
        !ImageIO::SaveImage(JoinPath(JoinPath(outputRoot, L"overlays"), SliceName(sliceIndex, L"_mask_comparison.png")), maskComparisonOverlay))
    {
        return false;
    }

    if (!infectionOverlay.empty())
    {
        if (!ImageIO::SaveImage(JoinPath(JoinPath(outputRoot, L"overlays"), SliceName(sliceIndex, L"_infection_overlay.png")), infectionOverlay))
        {
            return false;
        }
    }

    return true;
}

bool CBatchProcessor::WriteSummaryCsv(
    const std::wstring& path,
    const std::vector<SegmentationMetrics>& metricsRows,
    const std::vector<bool>& hasMetrics,
    const std::vector<InfectionStats>& infectionRows,
    const std::vector<bool>& hasInfection,
    CString& errorMessage) const
{
    std::ofstream output(path, std::ios::binary);
    if (!output)
    {
        errorMessage = _T("无法写入批量汇总 CSV。");
        return false;
    }

    const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
    output.write(reinterpret_cast<const char*>(bom), sizeof(bom));

    output << "slice,has_metrics,dice,iou,precision,recall,area_error,tp,fp,tn,fn,"
        << "has_infection,lung_area,infection_area,infection_ratio,left_lung_area,left_infection_area,left_ratio,right_lung_area,right_infection_area,right_ratio\n";

    for (size_t i = 0; i < metricsRows.size(); ++i)
    {
        const SegmentationMetrics& metrics = metricsRows[i];
        const InfectionStats& infection = infectionRows[i];
        output << i << ','
            << (hasMetrics[i] ? 1 : 0) << ','
            << std::fixed << std::setprecision(6)
            << metrics.dice << ','
            << metrics.iou << ','
            << metrics.precision << ','
            << metrics.recall << ','
            << metrics.areaError << ','
            << metrics.tp << ','
            << metrics.fp << ','
            << metrics.tn << ','
            << metrics.fn << ','
            << (hasInfection[i] ? 1 : 0) << ','
            << infection.lungArea << ','
            << infection.infectionArea << ','
            << infection.infectionRatio << ','
            << infection.leftLungArea << ','
            << infection.leftInfectionArea << ','
            << infection.leftRatio << ','
            << infection.rightLungArea << ','
            << infection.rightInfectionArea << ','
            << infection.rightRatio << '\n';
    }

    return output.good();
}

bool CBatchProcessor::WriteRunManifest(
    const std::wstring& path,
    const BatchOptions& options,
    int sourceSliceCount,
    int gtMaskSliceCount,
    int infectionMaskSliceCount,
    CString& errorMessage) const
{
    std::ofstream output(path, std::ios::binary);
    if (!output)
    {
        errorMessage = _T("无法写入批处理运行参数 manifest。");
        return false;
    }

    const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
    output.write(reinterpret_cast<const char*>(bom), sizeof(bom));

    const CStringA caseNameUtf8(CW2A(options.caseName, CP_UTF8));
    output << "case_name=" << caseNameUtf8.GetString() << "\n";
    output << "source_slice_count=" << sourceSliceCount << "\n";
    output << "gt_mask_slice_count=" << gtMaskSliceCount << "\n";
    output << "infection_mask_slice_count=" << infectionMaskSliceCount << "\n";
    output << "save_intermediate=" << (options.saveIntermediate ? 1 : 0) << "\n";
    output << "\n[segmentation]\n";
    output << "threshold_gaussian_kernel_size=" << options.segmentationOptions.thresholdGaussianKernelSize << "\n";
    output << "min_component_area=" << options.segmentationOptions.minComponentArea << "\n";
    output << "min_component_area_divisor=" << options.segmentationOptions.minComponentAreaDivisor << "\n";
    output << "keep_component_count=" << options.segmentationOptions.keepComponentCount << "\n";
    output << "open_kernel_size=" << options.segmentationOptions.openKernelSize << "\n";
    output << "close_kernel_size=" << options.segmentationOptions.closeKernelSize << "\n";
    output << "morphology_iterations=" << options.segmentationOptions.morphologyIterations << "\n";

    return output.good();
}

cv::Mat CBatchProcessor::NormalizeMask(const cv::Mat& mask, const cv::Size& size) const
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

std::wstring CBatchProcessor::JoinPath(const std::wstring& lhs, const std::wstring& rhs) const
{
    if (lhs.empty())
    {
        return rhs;
    }

    if (lhs.back() == L'\\' || lhs.back() == L'/')
    {
        return lhs + rhs;
    }

    return lhs + L"\\" + rhs;
}

std::wstring CBatchProcessor::SliceName(int index, const wchar_t* suffix) const
{
    std::wostringstream stream;
    stream << L"slice_" << std::setw(4) << std::setfill(L'0') << index << suffix;
    return stream.str();
}
