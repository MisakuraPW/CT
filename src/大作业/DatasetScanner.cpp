#include "pch.h"
#include "DatasetScanner.h"

#include <algorithm>
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

bool FileExists(const CString& path)
{
	const DWORD attrs = ::GetFileAttributes(path);
	return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

void EnsureDirectory(const CString& path)
{
	if (path.IsEmpty() || DirectoryExists(path))
	{
		return;
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
		CString partial = slash < 0 ? normalized : normalized.Left(slash);
		if (!partial.IsEmpty() && !DirectoryExists(partial))
		{
			::CreateDirectory(partial, nullptr);
		}

		if (slash < 0)
		{
			break;
		}

		start = slash + 1;
	}
}

std::vector<CString> ListFiles(const CString& folder, const CString& pattern)
{
	std::vector<CString> files;
	const CString query = JoinPath(folder, pattern);

	WIN32_FIND_DATA findData = {};
	HANDLE handle = ::FindFirstFile(query, &findData);
	if (handle == INVALID_HANDLE_VALUE)
	{
		return files;
	}

	do
	{
		if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			files.push_back(JoinPath(folder, findData.cFileName));
		}
	} while (::FindNextFile(handle, &findData));

	::FindClose(handle);
	std::sort(files.begin(), files.end(), [](const CString& a, const CString& b) {
		return a.CompareNoCase(b) < 0;
	});
	return files;
}

CString FileName(const CString& path)
{
	const int slash = max(path.ReverseFind(_T('\\')), path.ReverseFind(_T('/')));
	return slash >= 0 ? path.Mid(slash + 1) : path;
}

CString RemoveKnownExtension(CString fileName)
{
	CString lower(fileName);
	lower.MakeLower();
	if (lower.Right(7) == _T(".nii.gz"))
	{
		return fileName.Left(fileName.GetLength() - 7);
	}

	const int dot = fileName.ReverseFind(_T('.'));
	return dot >= 0 ? fileName.Left(dot) : fileName;
}

CString NormalizeCovidCaseId(const CString& ctPath)
{
	CString stem = RemoveKnownExtension(FileName(ctPath));

	const CString coronaPrefix = _T("coronacases_org_");
	if (stem.Left(coronaPrefix.GetLength()).CompareNoCase(coronaPrefix) == 0)
	{
		return _T("coronacases_") + stem.Mid(coronaPrefix.GetLength());
	}

	const CString radioPrefix = _T("radiopaedia_org_covid-19-pneumonia-");
	if (stem.Left(radioPrefix.GetLength()).CompareNoCase(radioPrefix) == 0)
	{
		CString body = stem.Mid(radioPrefix.GetLength());
		const CString suffix = _T("-dcm");
		if (body.Right(suffix.GetLength()).CompareNoCase(suffix) == 0)
		{
			body = body.Left(body.GetLength() - suffix.GetLength());
		}
		return _T("radiopaedia_") + body;
	}

	return stem;
}

CString Finding3DMaskPathForImage(const CString& imagePath)
{
	CString name = FileName(imagePath);
	if (name.Left(4).CompareNoCase(_T("IMG_")) == 0)
	{
		name = _T("MASK_") + name.Mid(4);
	}

	const int slash = max(imagePath.ReverseFind(_T('\\')), imagePath.ReverseFind(_T('/')));
	return slash >= 0 ? imagePath.Left(slash + 1) + name : name;
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

bool CDatasetScanner::Scan(const CString& lungDatasetRoot, const CString& covidDatasetRoot, DatasetScanResult& result, CString& errorMessage) const
{
	result = DatasetScanResult();

	if (!lungDatasetRoot.IsEmpty())
	{
		if (DirectoryExists(lungDatasetRoot))
		{
			ScanFindingLungs(lungDatasetRoot, result);
		}
		else
		{
			errorMessage.Format(_T("Finding Lungs 数据集路径不存在：%s"), lungDatasetRoot.GetString());
			return false;
		}
	}

	if (!covidDatasetRoot.IsEmpty())
	{
		if (DirectoryExists(covidDatasetRoot))
		{
			ScanCovid19(covidDatasetRoot, result);
		}
		else
		{
			errorMessage.Format(_T("COVID 数据集路径不存在：%s"), covidDatasetRoot.GetString());
			return false;
		}
	}

	if (result.cases.empty())
	{
		errorMessage = _T("没有扫描到任何数据集病例，请检查 paths.local.ini 中的数据集根目录。");
		return false;
	}

	errorMessage.Empty();
	return true;
}

bool CDatasetScanner::ExportTaskListCsv(const CString& outputPath, const DatasetScanResult& result, CString& errorMessage) const
{
	const int slash = max(outputPath.ReverseFind(_T('\\')), outputPath.ReverseFind(_T('/')));
	if (slash >= 0)
	{
		EnsureDirectory(outputPath.Left(slash));
	}

	std::ofstream file(outputPath.GetString(), std::ios::binary);
	if (!file.is_open())
	{
		errorMessage.Format(_T("无法写入任务清单：%s"), outputPath.GetString());
		return false;
	}

	const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
	file.write(reinterpret_cast<const char*>(bom), sizeof(bom));
	file << "dataset_kind,case_id,image_path,lung_mask_path,infection_mask_path,has_lung_mask,has_infection_mask\n";

	for (const auto& item : result.cases)
	{
		WriteCsvField(file, KindToDisplayName(item.kind));
		file << ",";
		WriteCsvField(file, item.caseId);
		file << ",";
		WriteCsvField(file, item.imagePath);
		file << ",";
		WriteCsvField(file, item.lungMaskPath);
		file << ",";
		WriteCsvField(file, item.infectionMaskPath);
		file << ",";
		file << (item.lungMaskPath.IsEmpty() ? "0" : "1");
		file << ",";
		file << (item.infectionMaskPath.IsEmpty() ? "0" : "1");
		file << "\n";
	}

	errorMessage.Empty();
	return true;
}

CString CDatasetScanner::KindToDisplayName(DatasetKind kind)
{
	switch (kind)
	{
	case DatasetKind::FindingLungs2D:
		return _T("finding-lungs-2d");
	case DatasetKind::FindingLungs3D:
		return _T("finding-lungs-3d");
	case DatasetKind::Covid19:
		return _T("covid-19-ct");
	default:
		return _T("unknown");
	}
}

void CDatasetScanner::ScanFindingLungs(const CString& root, DatasetScanResult& result) const
{
	const CString images2D = JoinPath(root, _T("2d_images"));
	const CString masks2D = JoinPath(root, _T("2d_masks"));
	for (const CString& imagePath : ListFiles(images2D, _T("*.tif")))
	{
		DatasetCase item;
		item.kind = DatasetKind::FindingLungs2D;
		item.caseId = RemoveKnownExtension(FileName(imagePath));
		item.imagePath = imagePath;

		const CString maskPath = JoinPath(masks2D, FileName(imagePath));
		if (FileExists(maskPath))
		{
			item.lungMaskPath = maskPath;
		}
		else
		{
			++result.missingMaskCount;
		}

		result.cases.push_back(item);
		++result.finding2DCount;
	}

	const CString images3D = JoinPath(root, _T("3d_images"));
	for (const CString& imagePath : ListFiles(images3D, _T("IMG_*.nii.gz")))
	{
		DatasetCase item;
		item.kind = DatasetKind::FindingLungs3D;
		item.caseId = RemoveKnownExtension(FileName(imagePath));
		item.imagePath = imagePath;

		const CString maskPath = Finding3DMaskPathForImage(imagePath);
		if (FileExists(maskPath))
		{
			item.lungMaskPath = maskPath;
		}
		else
		{
			++result.missingMaskCount;
		}

		result.cases.push_back(item);
		++result.finding3DCount;
	}
}

void CDatasetScanner::ScanCovid19(const CString& root, DatasetScanResult& result) const
{
	const CString ctDir = JoinPath(root, _T("ct_scans"));
	const CString lungDir = JoinPath(root, _T("lung_mask"));
	const CString infectionDir = JoinPath(root, _T("infection_mask"));

	for (const CString& ctPath : ListFiles(ctDir, _T("*.nii")))
	{
		DatasetCase item;
		item.kind = DatasetKind::Covid19;
		item.caseId = NormalizeCovidCaseId(ctPath);
		item.imagePath = ctPath;

		const CString lungMaskPath = JoinPath(lungDir, item.caseId + _T(".nii"));
		if (FileExists(lungMaskPath))
		{
			item.lungMaskPath = lungMaskPath;
		}
		else
		{
			++result.missingMaskCount;
		}

		const CString infectionMaskPath = JoinPath(infectionDir, item.caseId + _T(".nii"));
		if (FileExists(infectionMaskPath))
		{
			item.infectionMaskPath = infectionMaskPath;
		}

		result.cases.push_back(item);
		++result.covidCount;
	}
}
