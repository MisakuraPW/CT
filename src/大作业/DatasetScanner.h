#pragma once

#include <vector>

enum class DatasetKind
{
	FindingLungs2D,
	FindingLungs3D,
	Covid19
};

struct DatasetCase
{
	DatasetKind kind = DatasetKind::FindingLungs2D;
	CString caseId;
	CString imagePath;
	CString lungMaskPath;
	CString infectionMaskPath;
};

struct DatasetScanResult
{
	std::vector<DatasetCase> cases;
	int finding2DCount = 0;
	int finding3DCount = 0;
	int covidCount = 0;
	int missingMaskCount = 0;
};

class CDatasetScanner
{
public:
	bool Scan(const CString& lungDatasetRoot, const CString& covidDatasetRoot, DatasetScanResult& result, CString& errorMessage) const;
	bool ExportTaskListCsv(const CString& outputPath, const DatasetScanResult& result, CString& errorMessage) const;

	static CString KindToDisplayName(DatasetKind kind);

private:
	void ScanFindingLungs(const CString& root, DatasetScanResult& result) const;
	void ScanCovid19(const CString& root, DatasetScanResult& result) const;
};
