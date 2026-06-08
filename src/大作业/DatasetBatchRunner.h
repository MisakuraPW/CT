#pragma once

#include "DatasetScanner.h"
#include "ProcessingOptions.h"
#include "BatchProcessor.h"

#include <opencv2/core.hpp>

#include <vector>

struct DatasetBatchOptions
{
	CString outputRoot;
	bool saveIntermediate = true;
	LungSegmentationOptions segmentationOptions;
	BatchProgressCallback progressCallback;
};

struct DatasetBatchSummary
{
	int totalCases = 0;
	int succeededCases = 0;
	int failedCases = 0;
	int totalSlices = 0;
	CString summaryCsvPath;
};

class CDatasetBatchRunner
{
public:
	bool Run(
		const DatasetScanResult& scanResult,
		const DatasetBatchOptions& options,
		DatasetBatchSummary& summary,
		CString& errorMessage) const;

private:
	struct CaseRunRow
	{
		CString datasetKind;
		CString caseId;
		CString imagePath;
		CString outputRoot;
		CString batchSummaryCsvPath;
		CString manifestPath;
		CString status;
		CString errorMessage;
		int totalSlices = 0;
		int processedSlices = 0;
		int metricSlices = 0;
		int infectionSlices = 0;
	};

	bool LoadCaseSlices(
		const DatasetCase& item,
		std::vector<cv::Mat>& sourceSlices,
		std::vector<cv::Mat>& lungMaskSlices,
		std::vector<cv::Mat>& infectionMaskSlices,
		CString& errorMessage) const;
	bool LoadPathAsSlices(const CString& path, int imageFlags, std::vector<cv::Mat>& slices, CString& errorMessage) const;
	bool WriteDatasetSummaryCsv(const CString& path, const std::vector<CaseRunRow>& rows, CString& errorMessage) const;
	CString BuildCaseOutputRoot(const CString& outputRoot, const DatasetCase& item) const;
	CString SanitizePathPart(CString value) const;
};
