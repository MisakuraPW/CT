#pragma once

#include "InfectionAnalyzer.h"
#include "MetricsCalculator.h"

#include <string>

namespace CsvExporter
{
    bool ExportMetrics(const std::wstring& path, const CString& filename, const SegmentationMetrics& metrics);
    bool ExportInfectionStats(const std::wstring& path, const CString& filename, const InfectionStats& stats);
}
