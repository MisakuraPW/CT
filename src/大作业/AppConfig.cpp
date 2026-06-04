#include "pch.h"
#include "AppConfig.h"

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

bool FileExists(const CString& path)
{
	const DWORD attrs = ::GetFileAttributes(path);
	return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0;
}
}

bool CAppConfigLoader::LoadDefault(AppConfig& config, CString& errorMessage)
{
	const CString iniPath = ResolveConfigPath();
	if (iniPath.IsEmpty())
	{
		errorMessage = _T("未找到 configs\\paths.local.ini。请先复制 configs\\paths.example.ini 并填写本机数据集路径。");
		return false;
	}

	config.lungDatasetRoot = ReadStringValue(iniPath, _T("dataset"), _T("lung_dataset_root"));
	config.covidDatasetRoot = ReadStringValue(iniPath, _T("dataset"), _T("covid_dataset_root"));
	config.resultRoot = ReadStringValue(iniPath, _T("output"), _T("result_root"));

	CString saveIntermediate = ReadStringValue(iniPath, _T("app"), _T("save_intermediate"));
	saveIntermediate.Trim();
	config.saveIntermediate = saveIntermediate.IsEmpty() || saveIntermediate != _T("0");

	config.preprocessing.lungWindowLevel = ReadDoubleValue(iniPath, _T("preprocess"), _T("lung_window_level"), config.preprocessing.lungWindowLevel);
	config.preprocessing.lungWindowWidth = ReadDoubleValue(iniPath, _T("preprocess"), _T("lung_window_width"), config.preprocessing.lungWindowWidth);
	config.preprocessing.gaussianKernelSize = ReadIntValue(iniPath, _T("preprocess"), _T("gaussian_kernel_size"), config.preprocessing.gaussianKernelSize);
	config.preprocessing.medianKernelSize = ReadIntValue(iniPath, _T("preprocess"), _T("median_kernel_size"), config.preprocessing.medianKernelSize);
	config.preprocessing.claheClipLimit = ReadDoubleValue(iniPath, _T("preprocess"), _T("clahe_clip_limit"), config.preprocessing.claheClipLimit);
	config.preprocessing.claheTileGridSize = ReadIntValue(iniPath, _T("preprocess"), _T("clahe_tile_grid_size"), config.preprocessing.claheTileGridSize);

	config.segmentation.thresholdGaussianKernelSize = ReadIntValue(iniPath, _T("segmentation"), _T("threshold_gaussian_kernel_size"), config.segmentation.thresholdGaussianKernelSize);
	config.segmentation.minComponentArea = ReadIntValue(iniPath, _T("segmentation"), _T("min_component_area"), config.segmentation.minComponentArea);
	config.segmentation.minComponentAreaDivisor = ReadIntValue(iniPath, _T("segmentation"), _T("min_component_area_divisor"), config.segmentation.minComponentAreaDivisor);
	config.segmentation.keepComponentCount = ReadIntValue(iniPath, _T("segmentation"), _T("keep_component_count"), config.segmentation.keepComponentCount);
	config.segmentation.openKernelSize = ReadIntValue(iniPath, _T("segmentation"), _T("open_kernel_size"), config.segmentation.openKernelSize);
	config.segmentation.closeKernelSize = ReadIntValue(iniPath, _T("segmentation"), _T("close_kernel_size"), config.segmentation.closeKernelSize);
	config.segmentation.morphologyIterations = ReadIntValue(iniPath, _T("segmentation"), _T("morphology_iterations"), config.segmentation.morphologyIterations);

	if (config.resultRoot.IsEmpty())
	{
		config.resultRoot = _T("results");
	}

	if (config.lungDatasetRoot.IsEmpty() && config.covidDatasetRoot.IsEmpty())
	{
		errorMessage = _T("paths.local.ini 中没有配置 lung_dataset_root 或 covid_dataset_root。");
		return false;
	}

	errorMessage.Empty();
	return true;
}

CString CAppConfigLoader::ResolveConfigPath()
{
	const CString direct = _T("configs\\paths.local.ini");
	if (FileExists(direct))
	{
		return direct;
	}

	TCHAR modulePath[MAX_PATH] = {};
	::GetModuleFileName(nullptr, modulePath, MAX_PATH);
	CString path(modulePath);
	const int slash = path.ReverseFind(_T('\\'));
	if (slash >= 0)
	{
		path = path.Left(slash);
		const CString candidates[] = {
			JoinPath(path, _T("configs\\paths.local.ini")),
			JoinPath(path, _T("..\\configs\\paths.local.ini")),
			JoinPath(path, _T("..\\..\\configs\\paths.local.ini")),
			JoinPath(path, _T("..\\..\\..\\configs\\paths.local.ini")),
			JoinPath(path, _T("..\\..\\..\\..\\configs\\paths.local.ini")),
		};

		for (const CString& candidate : candidates)
		{
			if (FileExists(candidate))
			{
				return candidate;
			}
		}
	}

	return CString();
}

CString CAppConfigLoader::ReadStringValue(const CString& iniPath, const CString& section, const CString& key)
{
	TCHAR buffer[2048] = {};
	::GetPrivateProfileString(section, key, _T(""), buffer, _countof(buffer), iniPath);

	CString value(buffer);
	value.Trim();
	return value;
}

int CAppConfigLoader::ReadIntValue(const CString& iniPath, const CString& section, const CString& key, int defaultValue)
{
	const CString value = ReadStringValue(iniPath, section, key);
	if (value.IsEmpty())
	{
		return defaultValue;
	}

	TCHAR* end = nullptr;
	const long parsed = _tcstol(value, &end, 10);
	return end != value ? static_cast<int>(parsed) : defaultValue;
}

double CAppConfigLoader::ReadDoubleValue(const CString& iniPath, const CString& section, const CString& key, double defaultValue)
{
	const CString value = ReadStringValue(iniPath, section, key);
	if (value.IsEmpty())
	{
		return defaultValue;
	}

	TCHAR* end = nullptr;
	const double parsed = _tcstod(value, &end);
	return end != value ? parsed : defaultValue;
}
