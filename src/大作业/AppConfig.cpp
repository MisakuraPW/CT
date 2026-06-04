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
