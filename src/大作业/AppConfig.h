#pragma once

struct AppConfig
{
	CString lungDatasetRoot;
	CString covidDatasetRoot;
	CString resultRoot;
	bool saveIntermediate = true;
};

class CAppConfigLoader
{
public:
	static bool LoadDefault(AppConfig& config, CString& errorMessage);

private:
	static CString ResolveConfigPath();
	static CString ReadStringValue(const CString& iniPath, const CString& section, const CString& key);
};
