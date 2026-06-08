#pragma once

#include "InfectionAnalyzer.h"
#include "MetricsCalculator.h"
#include "ProcessingOptions.h"

#include <opencv2/core.hpp>

#include <functional>
#include <string>
#include <vector>

struct BatchProgressInfo
{
    CString scopeName;
    CString itemName;
    int current = 0;
    int total = 0;
};

using BatchProgressCallback = std::function<bool(const BatchProgressInfo&)>;

struct BatchOptions
{
    std::wstring outputRoot;
    CString caseName;
    bool saveIntermediate = true;
    LungSegmentationOptions segmentationOptions;
    BatchProgressCallback progressCallback;
};

struct BatchProcessResult
{
    int totalSlices = 0;
    int processedSlices = 0;
    int metricSlices = 0;
    int infectionSlices = 0;
    CString summaryCsvPath;
    CString manifestPath;
};

class CBatchProcessor
{
public:
    bool ProcessSlices(
        const std::vector<cv::Mat>& sourceSlices,
        const std::vector<cv::Mat>& gtMaskSlices,
        const std::vector<cv::Mat>& infectionMaskSlices,
        const BatchOptions& options,
        BatchProcessResult& result,
        CString& errorMessage) const;

private:
    bool EnsureOutputDirectories(const std::wstring& outputRoot, CString& errorMessage) const;
    bool SaveSliceImages(
        const std::wstring& outputRoot,
        int sliceIndex,
        const cv::Mat& finalMask,
        const cv::Mat& thresholdMask,
        const cv::Mat& connectedMask,
        const cv::Mat& morphologyMask,
        const cv::Mat& connectedColorMap,
        const cv::Mat& maskComparisonOverlay,
        const cv::Mat& infectionOverlay,
        bool saveIntermediate) const;
    bool WriteSummaryCsv(
        const std::wstring& path,
        const std::vector<SegmentationMetrics>& metricsRows,
        const std::vector<bool>& hasMetrics,
        const std::vector<InfectionStats>& infectionRows,
        const std::vector<bool>& hasInfection,
        CString& errorMessage) const;
    bool WriteRunManifest(
        const std::wstring& path,
        const BatchOptions& options,
        int sourceSliceCount,
        int gtMaskSliceCount,
        int infectionMaskSliceCount,
        CString& errorMessage) const;
    cv::Mat NormalizeMask(const cv::Mat& mask, const cv::Size& size) const;
    std::wstring JoinPath(const std::wstring& lhs, const std::wstring& rhs) const;
    std::wstring SliceName(int index, const wchar_t* suffix) const;
};
