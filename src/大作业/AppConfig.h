#pragma once

#include "ProcessingOptions.h"

struct AppConfig
{
	CString lungDatasetRoot;
	CString covidDatasetRoot;
	CString resultRoot;
	bool saveIntermediate = true;
	PreprocessingOptions preprocessing;
	LungSegmentationOptions segmentation;
};

class CAppConfigLoader
{
public:
	static bool LoadDefault(AppConfig& config, CString& errorMessage);

private:
	static CString ResolveConfigPath();
	static CString ReadStringValue(const CString& iniPath, const CString& section, const CString& key);
	static int ReadIntValue(const CString& iniPath, const CString& section, const CString& key, int defaultValue);
	static double ReadDoubleValue(const CString& iniPath, const CString& section, const CString& key, double defaultValue);
};
