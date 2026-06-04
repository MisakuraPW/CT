#include "pch.h"
#include "ProcessingOptionsDialog.h"

#include "resource.h"

CProcessingOptionsDialog::CProcessingOptionsDialog(AppConfig& config, CWnd* parent) noexcept
	: CDialogEx(IDD_PROCESSING_OPTIONS, parent)
	, m_config(config)
{
	m_lungWindowLevel = m_config.preprocessing.lungWindowLevel;
	m_lungWindowWidth = m_config.preprocessing.lungWindowWidth;
	m_gaussianKernelSize = m_config.preprocessing.gaussianKernelSize;
	m_medianKernelSize = m_config.preprocessing.medianKernelSize;
	m_claheClipLimit = m_config.preprocessing.claheClipLimit;
	m_claheTileGridSize = m_config.preprocessing.claheTileGridSize;
	m_thresholdGaussianKernelSize = m_config.segmentation.thresholdGaussianKernelSize;
	m_minComponentArea = m_config.segmentation.minComponentArea;
	m_minComponentAreaDivisor = m_config.segmentation.minComponentAreaDivisor;
	m_keepComponentCount = m_config.segmentation.keepComponentCount;
	m_openKernelSize = m_config.segmentation.openKernelSize;
	m_closeKernelSize = m_config.segmentation.closeKernelSize;
	m_morphologyIterations = m_config.segmentation.morphologyIterations;
}

BOOL CProcessingOptionsDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	return TRUE;
}

void CProcessingOptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_LUNG_WL, m_lungWindowLevel);
	DDX_Text(pDX, IDC_EDIT_LUNG_WW, m_lungWindowWidth);
	DDX_Text(pDX, IDC_EDIT_GAUSSIAN_KERNEL, m_gaussianKernelSize);
	DDX_Text(pDX, IDC_EDIT_MEDIAN_KERNEL, m_medianKernelSize);
	DDX_Text(pDX, IDC_EDIT_CLAHE_CLIP, m_claheClipLimit);
	DDX_Text(pDX, IDC_EDIT_CLAHE_TILE, m_claheTileGridSize);
	DDX_Text(pDX, IDC_EDIT_SEG_GAUSSIAN, m_thresholdGaussianKernelSize);
	DDX_Text(pDX, IDC_EDIT_MIN_COMPONENT_AREA, m_minComponentArea);
	DDX_Text(pDX, IDC_EDIT_MIN_AREA_DIVISOR, m_minComponentAreaDivisor);
	DDX_Text(pDX, IDC_EDIT_KEEP_COMPONENTS, m_keepComponentCount);
	DDX_Text(pDX, IDC_EDIT_OPEN_KERNEL, m_openKernelSize);
	DDX_Text(pDX, IDC_EDIT_CLOSE_KERNEL, m_closeKernelSize);
	DDX_Text(pDX, IDC_EDIT_MORPH_ITERATIONS, m_morphologyIterations);
}

void CProcessingOptionsDialog::OnOK()
{
	if (!UpdateData(TRUE))
	{
		return;
	}

	if (m_lungWindowWidth <= 0.0)
	{
		AfxMessageBox(_T("肺窗 WW 必须大于 0。"), MB_ICONWARNING);
		return;
	}

	m_config.preprocessing.lungWindowLevel = m_lungWindowLevel;
	m_config.preprocessing.lungWindowWidth = m_lungWindowWidth;
	m_config.preprocessing.gaussianKernelSize = m_gaussianKernelSize < 3 ? 3 : m_gaussianKernelSize;
	m_config.preprocessing.medianKernelSize = m_medianKernelSize < 3 ? 3 : m_medianKernelSize;
	m_config.preprocessing.claheClipLimit = m_claheClipLimit < 0.1 ? 0.1 : m_claheClipLimit;
	m_config.preprocessing.claheTileGridSize = m_claheTileGridSize < 1 ? 1 : m_claheTileGridSize;
	m_config.segmentation.thresholdGaussianKernelSize = m_thresholdGaussianKernelSize < 3 ? 3 : m_thresholdGaussianKernelSize;
	m_config.segmentation.minComponentArea = m_minComponentArea < 1 ? 1 : m_minComponentArea;
	m_config.segmentation.minComponentAreaDivisor = m_minComponentAreaDivisor < 1 ? 1 : m_minComponentAreaDivisor;
	m_config.segmentation.keepComponentCount = m_keepComponentCount < 1 ? 1 : m_keepComponentCount;
	m_config.segmentation.openKernelSize = m_openKernelSize < 3 ? 3 : m_openKernelSize;
	m_config.segmentation.closeKernelSize = m_closeKernelSize < 3 ? 3 : m_closeKernelSize;
	m_config.segmentation.morphologyIterations = m_morphologyIterations < 1 ? 1 : m_morphologyIterations;

	CDialogEx::OnOK();
}
