#pragma once

#include "AppConfig.h"

class CProcessingOptionsDialog : public CDialogEx
{
public:
	explicit CProcessingOptionsDialog(AppConfig& config, CWnd* parent = nullptr) noexcept;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROCESSING_OPTIONS };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
	void OnOK() override;

private:
	AppConfig& m_config;

	double m_lungWindowLevel = -600.0;
	double m_lungWindowWidth = 1500.0;
	int m_gaussianKernelSize = 5;
	int m_medianKernelSize = 5;
	double m_claheClipLimit = 2.0;
	int m_claheTileGridSize = 8;
	int m_thresholdGaussianKernelSize = 5;
	int m_minComponentArea = 64;
	int m_minComponentAreaDivisor = 1000;
	int m_keepComponentCount = 2;
	int m_openKernelSize = 3;
	int m_closeKernelSize = 7;
	int m_morphologyIterations = 1;
};
