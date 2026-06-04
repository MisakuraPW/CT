#include "pch.h"
#include "DatasetBatchRunner.h"

#include "BatchProcessor.h"
#include "ImageIO.h"
#include "NiftiIO.h"

#include <opencv2/imgcodecs.hpp>

#include <fstream>

namespace
{
CString JoinPath(const CString& left, const CString& right)
{
	if (left.IsEmpty())
	{
		return right;
	}

	const TCHAR tail = left[left.GetLength() - 1];
	if (tail == _T('\\') || tail == _T('/'))
	{
		return left + right;
	}

	return left + _T("\\") + right;
}

bool DirectoryExists(const CString& path)
{
	const DWORD attrs = ::GetFileAttributes(path);
	return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool EnsureDirectory(const CString& path)
{
	if (path.IsEmpty() || DirectoryExists(path))
	{
		return true;
	}

	CString normalized(path);
	normalized.Replace(_T('/'), _T('\\'));

	int start = 0;
	if (normalized.GetLength() >= 3 && normalized[1] == _T(':') && normalized[2] == _T('\\'))
	{
		start = 3;
	}

	while (true)
	{
		const int slash = normalized.Find(_T('\\'), start);
		const CString partial = slash < 0 ? normalized : normalized.Left(slash);
		if (!partial.IsEmpty() && !DirectoryExists(partial))
		{
			if (!::CreateDirectory(partial, nullptr) && ::GetLastError() != ERROR_ALREADY_EXISTS)
			{
				return false;
			}
		}

		if (slash < 0)
		{
			break;
		}

		start = slash + 1;
	}

	return DirectoryExists(path);
}

CString CsvEscape(const CString& value)
{
	CString escaped(value);
	escaped.Replace(_T("\""), _T("\"\""));
	return _T("\"") + escaped + _T("\"");
}

void WriteCsvField(std::ofstream& file, const CString& value)
{
	const CString escaped = CsvEscape(value);
	const CStringA utf8(CW2A(escaped, CP_UTF8));
	file.write(utf8.GetString(), utf8.GetLength());
}
}

bool CDatasetBatchRunner::Run(
	const DatasetScanResult& scanResult,
	const DatasetBatchOptions& options,
	DatasetBatchSummary& summary,
	CString& errorMessage) const
{
	summary = DatasetBatchSummary();
	errorMessage.Empty();

	if (scanResult.cases.empty())
	{
		errorMessage = _T("没有可处理的数据集任务。");
		return false;
	}

	if (!EnsureDirectory(options.outputRoot))
	{
		errorMessage.Format(_T("无法创建数据集输出目录：%s"), options.outputRoot.GetString());
		return false;
	}

	CBatchProcessor processor;
	std::vector<CaseRunRow> rows;
	rows.reserve(scanResult.cases.size());

	summary.totalCases = static_cast<int>(scanResult.cases.size());

	for (const DatasetCase& item : scanResult.cases)
	{
		CaseRunRow row;
		row.datasetKind = CDatasetScanner::KindToDisplayName(item.kind);
		row.caseId = item.caseId;
		row.imagePath = item.imagePath;
		row.outputRoot = BuildCaseOutputRoot(options.outputRoot, item);

		if (!EnsureDirectory(row.outputRoot))
		{
			row.status = _T("failed");
			row.errorMessage.Format(_T("无法创建病例输出目录：%s"), row.outputRoot.GetString());
			rows.push_back(row);
			++summary.failedCases;
			continue;
		}

		std::vector<cv::Mat> sourceSlices;
		std::vector<cv::Mat> lungMaskSlices;
		std::vector<cv::Mat> infectionMaskSlices;
		CString caseError;

		if (!LoadCaseSlices(item, sourceSlices, lungMaskSlices, infectionMaskSlices, caseError))
		{
			row.status = _T("failed");
			row.errorMessage = caseError;
			rows.push_back(row);
			++summary.failedCases;
			continue;
		}

		BatchOptions batchOptions;
		batchOptions.outputRoot = row.outputRoot.GetString();
		batchOptions.caseName = item.caseId;
		batchOptions.saveIntermediate = options.saveIntermediate;
		batchOptions.segmentationOptions = options.segmentationOptions;

		BatchProcessResult batchResult;
		if (!processor.ProcessSlices(sourceSlices, lungMaskSlices, infectionMaskSlices, batchOptions, batchResult, caseError))
		{
			row.status = _T("failed");
			row.errorMessage = caseError;
			rows.push_back(row);
			++summary.failedCases;
			continue;
		}

		row.status = _T("ok");
		row.batchSummaryCsvPath = batchResult.summaryCsvPath;
		row.totalSlices = batchResult.totalSlices;
		row.processedSlices = batchResult.processedSlices;
		row.metricSlices = batchResult.metricSlices;
		row.infectionSlices = batchResult.infectionSlices;
		rows.push_back(row);

		++summary.succeededCases;
		summary.totalSlices += batchResult.totalSlices;
	}

	const CString summaryPath = JoinPath(options.outputRoot, _T("dataset_summary.csv"));
	if (!WriteDatasetSummaryCsv(summaryPath, rows, errorMessage))
	{
		return false;
	}

	summary.summaryCsvPath = summaryPath;
	if (summary.succeededCases == 0)
	{
		errorMessage.Format(_T("全部 %d 个数据集病例处理失败，详情见：%s"), summary.totalCases, summary.summaryCsvPath.GetString());
		return false;
	}

	return true;
}

bool CDatasetBatchRunner::LoadCaseSlices(
	const DatasetCase& item,
	std::vector<cv::Mat>& sourceSlices,
	std::vector<cv::Mat>& lungMaskSlices,
	std::vector<cv::Mat>& infectionMaskSlices,
	CString& errorMessage) const
{
	if (!LoadPathAsSlices(item.imagePath, cv::IMREAD_UNCHANGED, sourceSlices, errorMessage))
	{
		return false;
	}

	if (!item.lungMaskPath.IsEmpty() &&
		!LoadPathAsSlices(item.lungMaskPath, cv::IMREAD_GRAYSCALE, lungMaskSlices, errorMessage))
	{
		return false;
	}

	if (!item.infectionMaskPath.IsEmpty() &&
		!LoadPathAsSlices(item.infectionMaskPath, cv::IMREAD_GRAYSCALE, infectionMaskSlices, errorMessage))
	{
		return false;
	}

	return true;
}

bool CDatasetBatchRunner::LoadPathAsSlices(const CString& path, int imageFlags, std::vector<cv::Mat>& slices, CString& errorMessage) const
{
	slices.clear();
	if (path.IsEmpty())
	{
		return true;
	}

	const std::wstring widePath(path.GetString());
	if (NiftiIO::IsNiftiPath(widePath))
	{
		NiftiVolume volume;
		if (!NiftiIO::LoadVolume(widePath, volume, errorMessage))
		{
			return false;
		}

		slices = volume.slices;
		if (slices.empty())
		{
			errorMessage.Format(_T("NIfTI 文件没有可用切片：%s"), path.GetString());
			return false;
		}
		return true;
	}

	cv::Mat image = ImageIO::LoadImage(widePath, imageFlags);
	if (image.empty())
	{
		errorMessage.Format(_T("无法读取图像文件：%s"), path.GetString());
		return false;
	}

	slices.push_back(image);
	return true;
}

bool CDatasetBatchRunner::WriteDatasetSummaryCsv(const CString& path, const std::vector<CaseRunRow>& rows, CString& errorMessage) const
{
	const int backslash = path.ReverseFind(_T('\\'));
	const int forwardSlash = path.ReverseFind(_T('/'));
	const int slash = backslash > forwardSlash ? backslash : forwardSlash;
	if (slash >= 0 && !EnsureDirectory(path.Left(slash)))
	{
		errorMessage.Format(_T("无法创建汇总 CSV 输出目录：%s"), path.Left(slash).GetString());
		return false;
	}

	std::ofstream output(path.GetString(), std::ios::binary);
	if (!output.is_open())
	{
		errorMessage.Format(_T("无法写入数据集汇总 CSV：%s"), path.GetString());
		return false;
	}

	const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
	output.write(reinterpret_cast<const char*>(bom), sizeof(bom));
	output << "dataset_kind,case_id,status,total_slices,processed_slices,metric_slices,infection_slices,image_path,output_root,batch_summary_csv,error\n";

	for (const CaseRunRow& row : rows)
	{
		WriteCsvField(output, row.datasetKind);
		output << ",";
		WriteCsvField(output, row.caseId);
		output << ",";
		WriteCsvField(output, row.status);
		output << ",";
		output << row.totalSlices << ",";
		output << row.processedSlices << ",";
		output << row.metricSlices << ",";
		output << row.infectionSlices << ",";
		WriteCsvField(output, row.imagePath);
		output << ",";
		WriteCsvField(output, row.outputRoot);
		output << ",";
		WriteCsvField(output, row.batchSummaryCsvPath);
		output << ",";
		WriteCsvField(output, row.errorMessage);
		output << "\n";
	}

	errorMessage.Empty();
	return output.good();
}

CString CDatasetBatchRunner::BuildCaseOutputRoot(const CString& outputRoot, const DatasetCase& item) const
{
	return JoinPath(JoinPath(outputRoot, CDatasetScanner::KindToDisplayName(item.kind)), SanitizePathPart(item.caseId));
}

CString CDatasetBatchRunner::SanitizePathPart(CString value) const
{
	value.Trim();
	if (value.IsEmpty())
	{
		return _T("case");
	}

	const TCHAR invalidChars[] = _T("<>:\"/\\|?*");
	for (int i = 0; invalidChars[i] != 0; ++i)
	{
		CString from;
		from.Format(_T("%c"), invalidChars[i]);
		value.Replace(from, _T("_"));
	}

	return value;
}
