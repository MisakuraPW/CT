
// 大作业Doc.cpp: C大作业Doc 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "大作业.h"
#endif

#include "大作业Doc.h"

#include "AppConfig.h"
#include "BatchProcessor.h"
#include "CsvExporter.h"
#include "ImageIO.h"
#include "InfectionAnalyzer.h"
#include "InfectionSegmenter.h"
#include "LungSegmenter.h"
#include "MetricsCalculator.h"
#include "NiftiIO.h"
#include "OverlayVisualizer.h"
#include "Preprocessor.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <propkey.h>

namespace
{
	AppConfig LoadConfigOrDefaults()
	{
		AppConfig config;
		CString ignoredError;
		CAppConfigLoader::LoadDefault(config, ignoredError);
		return config;
	}

	cv::Mat NormalizeBinaryMaskForDoc(const cv::Mat& mask)
	{
		if (mask.empty())
		{
			return {};
		}

		cv::Mat gray;
		if (mask.channels() == 1)
		{
			gray = mask;
		}
		else if (mask.channels() == 3)
		{
			cv::cvtColor(mask, gray, cv::COLOR_BGR2GRAY);
		}
		else if (mask.channels() == 4)
		{
			cv::cvtColor(mask, gray, cv::COLOR_BGRA2GRAY);
		}

		cv::Mat gray8;
		if (gray.depth() == CV_8U)
		{
			gray8 = gray;
		}
		else
		{
			cv::normalize(gray, gray8, 0, 255, cv::NORM_MINMAX, CV_8U);
		}

		cv::Mat binary;
		cv::threshold(gray8, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
		return binary;
	}
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// C大作业Doc

IMPLEMENT_DYNCREATE(C大作业Doc, CDocument)

BEGIN_MESSAGE_MAP(C大作业Doc, CDocument)
	ON_COMMAND(ID_IMAGE_OPEN_MASK, &C大作业Doc::OnOpenManualMask)
	ON_COMMAND(ID_IMAGE_OPEN_INFECTION_MASK, &C大作业Doc::OnOpenInfectionMask)
	ON_COMMAND(ID_PROCESS_RUN_SEGMENTATION, &C大作业Doc::OnRunLungSegmentation)
	ON_COMMAND(ID_PROCESS_SEGMENT_INFECTION, &C大作业Doc::OnSegmentInfection)
	ON_COMMAND(ID_ANALYSIS_CALCULATE_METRICS, &C大作业Doc::OnCalculateMetrics)
	ON_COMMAND(ID_ANALYSIS_INFECTION_BURDEN, &C大作业Doc::OnAnalyzeInfectionBurden)
	ON_COMMAND(ID_ANALYSIS_CALCULATE_INFECTION_METRICS, &C大作业Doc::OnCalculateInfectionMetrics)
	ON_COMMAND(ID_RESULT_SAVE_CURRENT, &C大作业Doc::OnSaveCurrentResult)
	ON_COMMAND(ID_RESULT_EXPORT_METRICS_CSV, &C大作业Doc::OnExportMetricsCsv)
	ON_COMMAND(ID_RESULT_EXPORT_INFECTION_CSV, &C大作业Doc::OnExportInfectionCsv)
	ON_COMMAND(ID_PREPROCESS_GRAY_NORMALIZE, &C大作业Doc::OnPreprocessGrayNormalize)
	ON_COMMAND(ID_PREPROCESS_LUNG_WINDOW, &C大作业Doc::OnPreprocessLungWindow)
	ON_COMMAND(ID_PREPROCESS_GAUSSIAN, &C大作业Doc::OnPreprocessGaussian)
	ON_COMMAND(ID_PREPROCESS_MEDIAN, &C大作业Doc::OnPreprocessMedian)
	ON_COMMAND(ID_PREPROCESS_CLAHE, &C大作业Doc::OnPreprocessClahe)
	ON_COMMAND(ID_VIEW_SHOW_ORIGINAL, &C大作业Doc::OnShowOriginal)
	ON_COMMAND(ID_VIEW_SHOW_GRAY, &C大作业Doc::OnShowGray)
	ON_COMMAND(ID_VIEW_SHOW_LUNG_WINDOW, &C大作业Doc::OnShowLungWindow)
	ON_COMMAND(ID_VIEW_SHOW_GAUSSIAN, &C大作业Doc::OnShowGaussian)
	ON_COMMAND(ID_VIEW_SHOW_MEDIAN, &C大作业Doc::OnShowMedian)
	ON_COMMAND(ID_VIEW_SHOW_CLAHE, &C大作业Doc::OnShowClahe)
	ON_COMMAND(ID_VIEW_SHOW_THRESHOLD, &C大作业Doc::OnShowThreshold)
	ON_COMMAND(ID_VIEW_SHOW_CONNECTED, &C大作业Doc::OnShowConnected)
	ON_COMMAND(ID_VIEW_SHOW_MORPHOLOGY, &C大作业Doc::OnShowMorphology)
	ON_COMMAND(ID_VIEW_SHOW_FINAL_MASK, &C大作业Doc::OnShowFinalMask)
	ON_COMMAND(ID_VIEW_SHOW_MANUAL_MASK, &C大作业Doc::OnShowManualMask)
	ON_COMMAND(ID_VIEW_SHOW_CONNECTED_COLORMAP, &C大作业Doc::OnShowConnectedColorMap)
	ON_COMMAND(ID_VIEW_SHOW_MASK_COMPARISON, &C大作业Doc::OnShowMaskComparisonOverlay)
	ON_COMMAND(ID_VIEW_SHOW_INFECTION_MASK, &C大作业Doc::OnShowInfectionMask)
	ON_COMMAND(ID_VIEW_SHOW_INFECTION_OVERLAY, &C大作业Doc::OnShowInfectionOverlay)
	ON_COMMAND(ID_VIEW_SHOW_INFECTION_SEGMENTATION_MASK, &C大作业Doc::OnShowInfectionSegmentationMask)
	ON_COMMAND(ID_VOLUME_PREVIOUS_SLICE, &C大作业Doc::OnPreviousSlice)
	ON_COMMAND(ID_VOLUME_NEXT_SLICE, &C大作业Doc::OnNextSlice)
	ON_COMMAND(ID_BATCH_PROCESS_CURRENT_VOLUME, &C大作业Doc::OnBatchProcessCurrentVolume)
	ON_UPDATE_COMMAND_UI(ID_IMAGE_OPEN_MASK, &C大作业Doc::OnUpdateHasOriginal)
	ON_UPDATE_COMMAND_UI(ID_IMAGE_OPEN_INFECTION_MASK, &C大作业Doc::OnUpdateHasOriginal)
	ON_UPDATE_COMMAND_UI(ID_PROCESS_RUN_SEGMENTATION, &C大作业Doc::OnUpdateHasOriginal)
	ON_UPDATE_COMMAND_UI(ID_PROCESS_SEGMENT_INFECTION, &C大作业Doc::OnUpdateHasFinalMask)
	ON_UPDATE_COMMAND_UI(ID_ANALYSIS_CALCULATE_METRICS, &C大作业Doc::OnUpdateHasFinalAndMask)
	ON_UPDATE_COMMAND_UI(ID_ANALYSIS_INFECTION_BURDEN, &C大作业Doc::OnUpdateHasFinalAndInfection)
	ON_UPDATE_COMMAND_UI(ID_ANALYSIS_CALCULATE_INFECTION_METRICS, &C大作业Doc::OnUpdateHasInfectionMetrics)
	ON_UPDATE_COMMAND_UI(ID_RESULT_SAVE_CURRENT, &C大作业Doc::OnUpdateHasOriginal)
	ON_UPDATE_COMMAND_UI(ID_RESULT_EXPORT_METRICS_CSV, &C大作业Doc::OnUpdateHasMetrics)
	ON_UPDATE_COMMAND_UI(ID_RESULT_EXPORT_INFECTION_CSV, &C大作业Doc::OnUpdateHasInfectionStats)
	ON_UPDATE_COMMAND_UI(ID_PREPROCESS_GRAY_NORMALIZE, &C大作业Doc::OnUpdateHasOriginal)
	ON_UPDATE_COMMAND_UI(ID_PREPROCESS_LUNG_WINDOW, &C大作业Doc::OnUpdateHasOriginal)
	ON_UPDATE_COMMAND_UI(ID_PREPROCESS_GAUSSIAN, &C大作业Doc::OnUpdateHasOriginal)
	ON_UPDATE_COMMAND_UI(ID_PREPROCESS_MEDIAN, &C大作业Doc::OnUpdateHasOriginal)
	ON_UPDATE_COMMAND_UI(ID_PREPROCESS_CLAHE, &C大作业Doc::OnUpdateHasOriginal)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_ORIGINAL, &C大作业Doc::OnUpdateHasOriginal)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_GRAY, &C大作业Doc::OnUpdateHasPreprocessedGray)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_LUNG_WINDOW, &C大作业Doc::OnUpdateHasLungWindowImage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_GAUSSIAN, &C大作业Doc::OnUpdateHasGaussianImage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_MEDIAN, &C大作业Doc::OnUpdateHasMedianImage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_CLAHE, &C大作业Doc::OnUpdateHasClaheImage)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_THRESHOLD, &C大作业Doc::OnUpdateHasSegmentation)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_CONNECTED, &C大作业Doc::OnUpdateHasSegmentation)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_MORPHOLOGY, &C大作业Doc::OnUpdateHasSegmentation)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_FINAL_MASK, &C大作业Doc::OnUpdateHasSegmentation)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_MANUAL_MASK, &C大作业Doc::OnUpdateHasManualMask)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_CONNECTED_COLORMAP, &C大作业Doc::OnUpdateHasConnectedColorMap)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_MASK_COMPARISON, &C大作业Doc::OnUpdateHasMaskComparisonOverlay)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_INFECTION_MASK, &C大作业Doc::OnUpdateHasInfectionMask)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_INFECTION_SEGMENTATION_MASK, &C大作业Doc::OnUpdateHasInfectionSegmentationMask)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_INFECTION_OVERLAY, &C大作业Doc::OnUpdateHasInfectionStats)
	ON_UPDATE_COMMAND_UI(ID_VOLUME_PREVIOUS_SLICE, &C大作业Doc::OnUpdateCanPreviousSlice)
	ON_UPDATE_COMMAND_UI(ID_VOLUME_NEXT_SLICE, &C大作业Doc::OnUpdateCanNextSlice)
	ON_UPDATE_COMMAND_UI(ID_BATCH_PROCESS_CURRENT_VOLUME, &C大作业Doc::OnUpdateCanBatchProcessCurrentData)
END_MESSAGE_MAP()


// C大作业Doc 构造/析构

C大作业Doc::C大作业Doc() noexcept
{
	// TODO: 在此添加一次性构造代码

}

C大作业Doc::~C大作业Doc()
{
}

BOOL C大作业Doc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	ClearImages();

	return TRUE;
}

BOOL C大作业Doc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
	{
		return FALSE;
	}

	if (!LoadSourceImage(lpszPathName))
	{
		AfxMessageBox(_T("图像读取失败。请确认文件格式是 PNG/JPG/BMP/TIFF，且 OpenCV DLL 可以被程序找到。"));
		return FALSE;
	}

	return TRUE;
}


// C大作业Doc 序列化

void C大作业Doc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}

#ifdef SHARED_HANDLERS

// 缩略图的支持
void C大作业Doc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 修改此代码以绘制文档数据
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 搜索处理程序的支持
void C大作业Doc::InitializeSearchContent()
{
	CString strSearchContent;
	// 从文档数据设置搜索内容。
	// 内容部分应由“;”分隔

	// 例如:     strSearchContent = _T("point;rectangle;circle;ole object;")；
	SetSearchContent(strSearchContent);
}

void C大作业Doc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// C大作业Doc 诊断

#ifdef _DEBUG
void C大作业Doc::AssertValid() const
{
	CDocument::AssertValid();
}

void C大作业Doc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// C大作业Doc 命令

const cv::Mat& C大作业Doc::GetDisplayImage() const
{
	switch (m_displayKind)
	{
	case DisplayImageKind::Gray:
		if (!m_grayImage.empty())
		{
			return m_grayImage;
		}
		return m_segmentationResult.gray.empty() ? m_originalImage : m_segmentationResult.gray;
	case DisplayImageKind::LungWindow:
		return m_lungWindowImage.empty() ? m_originalImage : m_lungWindowImage;
	case DisplayImageKind::GaussianBlur:
		return m_gaussianImage.empty() ? m_originalImage : m_gaussianImage;
	case DisplayImageKind::MedianBlur:
		return m_medianImage.empty() ? m_originalImage : m_medianImage;
	case DisplayImageKind::Clahe:
		return m_claheImage.empty() ? m_originalImage : m_claheImage;
	case DisplayImageKind::Threshold:
		return m_segmentationResult.thresholdMask.empty() ? m_originalImage : m_segmentationResult.thresholdMask;
	case DisplayImageKind::Connected:
		return m_segmentationResult.connectedMask.empty() ? m_originalImage : m_segmentationResult.connectedMask;
	case DisplayImageKind::Morphology:
		return m_segmentationResult.morphologyMask.empty() ? m_originalImage : m_segmentationResult.morphologyMask;
	case DisplayImageKind::FinalMask:
		return m_segmentationResult.finalMask.empty() ? m_originalImage : m_segmentationResult.finalMask;
	case DisplayImageKind::ManualMask:
		return m_manualMask.empty() ? m_originalImage : m_manualMask;
	case DisplayImageKind::ConnectedColorMap:
		return m_connectedColorMap.empty() ? m_originalImage : m_connectedColorMap;
	case DisplayImageKind::MaskComparisonOverlay:
		return m_maskComparisonOverlay.empty() ? m_originalImage : m_maskComparisonOverlay;
	case DisplayImageKind::InfectionMask:
		return m_infectionMask.empty() ? m_originalImage : m_infectionMask;
	case DisplayImageKind::InfectionOverlay:
		return m_infectionOverlay.empty() ? m_originalImage : m_infectionOverlay;
	case DisplayImageKind::InfectionSegmentationMask:
		return m_infectionSegmentationMask.empty() ? m_originalImage : m_infectionSegmentationMask;
	case DisplayImageKind::Original:
	default:
		return m_originalImage;
	}
}

CString C大作业Doc::GetDisplayName() const
{
	CString prefix;
	if (HasVolume())
	{
		prefix.Format(_T("切片 %d / %d - "), m_currentSliceIndex + 1, m_sourceVolume.depth);
	}

	switch (m_displayKind)
	{
	case DisplayImageKind::Gray:
		return prefix + _T("灰度/归一化图像");
	case DisplayImageKind::LungWindow:
		return prefix + _T("肺窗显示 WL=-600 WW=1500");
	case DisplayImageKind::GaussianBlur:
		return prefix + _T("高斯滤波结果");
	case DisplayImageKind::MedianBlur:
		return prefix + _T("中值滤波结果");
	case DisplayImageKind::Clahe:
		return prefix + _T("CLAHE 对比度增强结果");
	case DisplayImageKind::Threshold:
		return prefix + _T("Otsu 阈值分割结果");
	case DisplayImageKind::Connected:
		return prefix + _T("连通域筛选结果");
	case DisplayImageKind::Morphology:
		return prefix + _T("形态学修复结果");
	case DisplayImageKind::FinalMask:
		return prefix + _T("最终肺部 mask");
	case DisplayImageKind::ManualMask:
		return prefix + _T("人工 mask");
	case DisplayImageKind::ConnectedColorMap:
		return prefix + _T("连通域伪彩色图");
	case DisplayImageKind::MaskComparisonOverlay:
		return prefix + _T("预测/人工 mask 对比叠加图");
	case DisplayImageKind::InfectionMask:
		return prefix + _T("感染 mask");
	case DisplayImageKind::InfectionOverlay:
		return prefix + _T("感染区域叠加图");
	case DisplayImageKind::InfectionSegmentationMask:
		return prefix + _T("感染分割 mask");
	case DisplayImageKind::Original:
	default:
		return prefix + _T("原始图像");
	}
}

BOOL C大作业Doc::HasOriginalImage() const
{
	return !m_originalImage.empty();
}

BOOL C大作业Doc::HasPreprocessedGray() const
{
	return !m_grayImage.empty() || !m_segmentationResult.gray.empty();
}

BOOL C大作业Doc::HasLungWindowImage() const
{
	return !m_lungWindowImage.empty();
}

BOOL C大作业Doc::HasGaussianImage() const
{
	return !m_gaussianImage.empty();
}

BOOL C大作业Doc::HasMedianImage() const
{
	return !m_medianImage.empty();
}

BOOL C大作业Doc::HasClaheImage() const
{
	return !m_claheImage.empty();
}

BOOL C大作业Doc::HasFinalMask() const
{
	return !m_segmentationResult.finalMask.empty();
}

BOOL C大作业Doc::HasManualMask() const
{
	return !m_manualMask.empty();
}

BOOL C大作业Doc::HasInfectionMask() const
{
	return !m_infectionMask.empty();
}

BOOL C大作业Doc::HasInfectionSegmentationMask() const
{
	return !m_infectionSegmentationMask.empty();
}

BOOL C大作业Doc::HasConnectedColorMap() const
{
	return !m_connectedColorMap.empty();
}

BOOL C大作业Doc::HasMaskComparisonOverlay() const
{
	return !m_maskComparisonOverlay.empty();
}

BOOL C大作业Doc::HasMetrics() const
{
	return m_hasMetrics;
}

BOOL C大作业Doc::HasInfectionStats() const
{
	return m_hasInfectionStats;
}

BOOL C大作业Doc::HasInfectionMetrics() const
{
	return m_hasInfectionMetrics;
}

BOOL C大作业Doc::HasVolume() const
{
	return m_sourceVolume.depth > 1;
}

BOOL C大作业Doc::CanMoveToPreviousSlice() const
{
	return HasVolume() && m_currentSliceIndex > 0;
}

BOOL C大作业Doc::CanMoveToNextSlice() const
{
	return HasVolume() && m_currentSliceIndex + 1 < m_sourceVolume.depth;
}

BOOL C大作业Doc::CanBatchProcessCurrentData() const
{
	return HasOriginalImage();
}

BOOL C大作业Doc::CanCalculateInfectionMetrics() const
{
	return HasInfectionSegmentationMask() && HasInfectionMask();
}

BOOL C大作业Doc::LoadSourceImage(const CString& pathName)
{
	ClearImages();

	if (NiftiIO::IsNiftiPath(pathName.GetString()))
	{
		CString error;
		if (!NiftiIO::LoadVolume(pathName.GetString(), m_sourceVolume, error))
		{
			AfxMessageBox(error);
			return FALSE;
		}

		m_currentSliceIndex = std::max(0, m_sourceVolume.depth / 2);
		m_originalImage = SliceOrEmpty(m_sourceVolume, m_currentSliceIndex);
	}
	else
	{
		m_originalImage = ImageIO::LoadImage(pathName.GetString(), cv::IMREAD_UNCHANGED);
	}

	if (m_originalImage.empty())
	{
		return FALSE;
	}

	m_sourcePath = pathName;
	m_displayKind = DisplayImageKind::Original;
	SetTitle(pathName.Mid(pathName.ReverseFind(_T('\\')) + 1));
	UpdateAllViews(nullptr);
	return TRUE;
}

BOOL C大作业Doc::LoadManualMask(const CString& pathName)
{
	m_manualMaskVolume = NiftiVolume{};
	if (NiftiIO::IsNiftiPath(pathName.GetString()))
	{
		CString error;
		if (!NiftiIO::LoadVolume(pathName.GetString(), m_manualMaskVolume, error))
		{
			AfxMessageBox(error);
			return FALSE;
		}
		m_manualMask = SliceOrEmpty(m_manualMaskVolume, m_currentSliceIndex);
	}
	else
	{
		m_manualMask = ImageIO::LoadImage(pathName.GetString(), cv::IMREAD_GRAYSCALE);
	}

	if (m_manualMask.empty())
	{
		return FALSE;
	}

	m_manualMask = NormalizeBinaryMaskForDoc(m_manualMask);
	m_manualMaskPath = pathName;
	m_hasMetrics = FALSE;
	m_maskComparisonOverlay.release();
	SetDisplayKind(DisplayImageKind::ManualMask);
	return TRUE;
}

BOOL C大作业Doc::LoadInfectionMask(const CString& pathName)
{
	m_infectionMaskVolume = NiftiVolume{};
	if (NiftiIO::IsNiftiPath(pathName.GetString()))
	{
		CString error;
		if (!NiftiIO::LoadVolume(pathName.GetString(), m_infectionMaskVolume, error))
		{
			AfxMessageBox(error);
			return FALSE;
		}
		m_infectionMask = SliceOrEmpty(m_infectionMaskVolume, m_currentSliceIndex);
	}
	else
	{
		m_infectionMask = ImageIO::LoadImage(pathName.GetString(), cv::IMREAD_GRAYSCALE);
	}

	if (m_infectionMask.empty())
	{
		return FALSE;
	}

	m_infectionMask = NormalizeBinaryMaskForDoc(m_infectionMask);
	m_infectionMaskPath = pathName;
	m_hasInfectionStats = FALSE;
	m_infectionOverlay.release();
	SetDisplayKind(DisplayImageKind::InfectionMask);
	return TRUE;
}

cv::Mat C大作业Doc::SliceOrEmpty(const NiftiVolume& volume, int sliceIndex) const
{
	if (sliceIndex < 0 || sliceIndex >= static_cast<int>(volume.slices.size()))
	{
		return {};
	}

	return volume.slices[sliceIndex].clone();
}

void C大作业Doc::ApplyCurrentSlice()
{
	if (!m_sourceVolume.slices.empty())
	{
		m_originalImage = SliceOrEmpty(m_sourceVolume, m_currentSliceIndex);
	}

	if (!m_manualMaskVolume.slices.empty())
	{
		m_manualMask = SliceOrEmpty(m_manualMaskVolume, m_currentSliceIndex);
		if (!m_manualMask.empty())
		{
			m_manualMask = NormalizeBinaryMaskForDoc(m_manualMask);
		}
	}

	if (!m_infectionMaskVolume.slices.empty())
	{
		m_infectionMask = SliceOrEmpty(m_infectionMaskVolume, m_currentSliceIndex);
		if (!m_infectionMask.empty())
		{
			m_infectionMask = NormalizeBinaryMaskForDoc(m_infectionMask);
		}
	}

	ClearSliceDerivedResults();
	SetDisplayKind(DisplayImageKind::Original);
}

void C大作业Doc::ClearSliceDerivedResults()
{
	m_grayImage.release();
	m_lungWindowImage.release();
	m_gaussianImage.release();
	m_medianImage.release();
	m_claheImage.release();
	m_segmentationResult = LungSegmentationResult{};
	m_connectedColorMap.release();
	m_maskComparisonOverlay.release();
	m_infectionOverlay.release();
	m_infectionSegmentationMask.release();
	m_lastMetrics = SegmentationMetrics{};
	m_lastInfectionStats = InfectionStats{};
	m_lastInfectionMetrics = SegmentationMetrics{};
	m_hasMetrics = FALSE;
	m_hasInfectionStats = FALSE;
	m_hasInfectionMetrics = FALSE;
}

void C大作业Doc::ClearImages()
{
	m_originalImage.release();
	m_grayImage.release();
	m_lungWindowImage.release();
	m_gaussianImage.release();
	m_medianImage.release();
	m_claheImage.release();
	m_manualMask.release();
	m_infectionMask.release();
	m_connectedColorMap.release();
	m_maskComparisonOverlay.release();
	m_infectionOverlay.release();
	m_infectionSegmentationMask.release();
	m_sourceVolume = NiftiVolume{};
	m_manualMaskVolume = NiftiVolume{};
	m_infectionMaskVolume = NiftiVolume{};
	m_segmentationResult = LungSegmentationResult{};
	m_displayKind = DisplayImageKind::Original;
	m_currentSliceIndex = 0;
	m_sourcePath.Empty();
	m_manualMaskPath.Empty();
	m_infectionMaskPath.Empty();
	m_lastMetrics = SegmentationMetrics{};
	m_lastInfectionStats = InfectionStats{};
	m_lastInfectionMetrics = SegmentationMetrics{};
	m_hasMetrics = FALSE;
	m_hasInfectionStats = FALSE;
	m_hasInfectionMetrics = FALSE;
}

void C大作业Doc::SetDisplayKind(DisplayImageKind kind)
{
	m_displayKind = kind;
	UpdateAllViews(nullptr);
}

cv::Mat C大作业Doc::CurrentSaveImage() const
{
	const cv::Mat& current = GetDisplayImage();
	if (current.empty())
	{
		return {};
	}

	if (current.depth() == CV_8U)
	{
		return current;
	}

	cv::Mat normalized;
	cv::normalize(current, normalized, 0, 255, cv::NORM_MINMAX, CV_8U);
	return normalized;
}

CString C大作业Doc::GetSourceFileName() const
{
	if (m_sourcePath.IsEmpty())
	{
		return _T("current_image");
	}

	const int slash = m_sourcePath.ReverseFind(_T('\\'));
	return slash >= 0 ? m_sourcePath.Mid(slash + 1) : m_sourcePath;
}

void C大作业Doc::OnOpenManualMask()
{
	CFileDialog dlg(TRUE, nullptr, nullptr, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		_T("Medical/Image Files (*.nii;*.nii.gz;*.png;*.jpg;*.jpeg;*.bmp;*.tif;*.tiff)|*.nii;*.nii.gz;*.png;*.jpg;*.jpeg;*.bmp;*.tif;*.tiff|All Files (*.*)|*.*||"));
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	if (!LoadManualMask(dlg.GetPathName()))
	{
		AfxMessageBox(_T("人工 mask 读取失败。"));
	}
}

void C大作业Doc::OnOpenInfectionMask()
{
	CFileDialog dlg(TRUE, nullptr, nullptr, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		_T("Medical/Image Files (*.nii;*.nii.gz;*.png;*.jpg;*.jpeg;*.bmp;*.tif;*.tiff)|*.nii;*.nii.gz;*.png;*.jpg;*.jpeg;*.bmp;*.tif;*.tiff|All Files (*.*)|*.*||"));
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	if (!LoadInfectionMask(dlg.GetPathName()))
	{
		AfxMessageBox(_T("感染 mask 读取失败。"));
	}
}

void C大作业Doc::OnRunLungSegmentation()
{
	if (m_originalImage.empty())
	{
		AfxMessageBox(_T("请先打开一张 CT 图像。"));
		return;
	}

	CWaitCursor wait;
	const AppConfig config = LoadConfigOrDefaults();
	CLungSegmenter segmenter;
	m_segmentationResult = segmenter.Segment(m_originalImage, config.segmentation);
	if (m_segmentationResult.finalMask.empty())
	{
		AfxMessageBox(_T("肺部分割失败。"));
		return;
	}

	COverlayVisualizer visualizer;
	m_grayImage = m_segmentationResult.gray.clone();
	m_connectedColorMap = visualizer.MakeConnectedComponentColorMap(m_segmentationResult.thresholdMask);
	m_maskComparisonOverlay.release();
	m_hasMetrics = FALSE;
	m_hasInfectionStats = FALSE;
	m_infectionOverlay.release();
	SetDisplayKind(DisplayImageKind::FinalMask);
	AfxMessageBox(_T("肺部分割完成。可在“视图”菜单切换查看中间结果。"));
}

void C大作业Doc::OnSegmentInfection()
{
	if (m_segmentationResult.finalMask.empty())
	{
		AfxMessageBox(_T("请先执行“一键肺部分割”。"));
		return;
	}

	CWaitCursor wait;
	CInfectionSegmenter segmenter;
	InfectionSegmentationResult result = segmenter.Segment(
		m_originalImage, m_segmentationResult.finalMask);

	if (result.infectionMask.empty())
	{
		AfxMessageBox(_T("感染区域分割失败。"));
		return;
	}

	m_infectionSegmentationMask = result.infectionMask.clone();
	m_hasInfectionStats = FALSE;
	m_hasInfectionMetrics = FALSE;
	m_infectionOverlay.release();
	SetDisplayKind(DisplayImageKind::InfectionSegmentationMask);
	AfxMessageBox(_T("感染区域分割完成。可在“视图”菜单查看感染 mask 和叠加图，\n也可在“分析”菜单执行感染负荷分析。"));
}

void C大作业Doc::OnCalculateMetrics()
{
	if (m_segmentationResult.finalMask.empty() || m_manualMask.empty())
	{
		AfxMessageBox(_T("请先完成肺部分割，并打开人工 mask。"));
		return;
	}

	CMetricsCalculator calculator;
	m_lastMetrics = calculator.Calculate(m_segmentationResult.finalMask, m_manualMask);
	COverlayVisualizer visualizer;
	m_maskComparisonOverlay = visualizer.MakeComparisonOverlay(m_originalImage, m_segmentationResult.finalMask, m_manualMask);
	m_hasMetrics = TRUE;

	CString message;
	message.Format(_T("Dice: %.4f\r\nIoU: %.4f\r\nPrecision: %.4f\r\nRecall: %.4f\r\nArea Error: %.4f\r\n\r\nTP: %lld\r\nFP: %lld\r\nTN: %lld\r\nFN: %lld"),
		m_lastMetrics.dice,
		m_lastMetrics.iou,
		m_lastMetrics.precision,
		m_lastMetrics.recall,
		m_lastMetrics.areaError,
		m_lastMetrics.tp,
		m_lastMetrics.fp,
		m_lastMetrics.tn,
		m_lastMetrics.fn);
	AfxMessageBox(message);
}

void C大作业Doc::OnAnalyzeInfectionBurden()
{
	if (m_segmentationResult.finalMask.empty() || m_infectionMask.empty())
	{
		AfxMessageBox(_T("请先完成肺部分割，并打开感染 mask。"));
		return;
	}

	CInfectionAnalyzer analyzer;
	m_lastInfectionStats = analyzer.Analyze(m_segmentationResult.finalMask, m_infectionMask);
	m_infectionOverlay = analyzer.MakeInfectionOverlay(m_originalImage, m_segmentationResult.finalMask, m_infectionMask);
	m_hasInfectionStats = TRUE;

	CString message;
	message.Format(_T("整体感染负荷: %.4f\r\n肺部面积: %lld\r\n感染面积: %lld\r\n\r\n图像左侧肺感染比例: %.4f\r\n左侧肺面积: %lld\r\n左侧感染面积: %lld\r\n\r\n图像右侧肺感染比例: %.4f\r\n右侧肺面积: %lld\r\n右侧感染面积: %lld"),
		m_lastInfectionStats.infectionRatio,
		m_lastInfectionStats.lungArea,
		m_lastInfectionStats.infectionArea,
		m_lastInfectionStats.leftRatio,
		m_lastInfectionStats.leftLungArea,
		m_lastInfectionStats.leftInfectionArea,
		m_lastInfectionStats.rightRatio,
		m_lastInfectionStats.rightLungArea,
		m_lastInfectionStats.rightInfectionArea);

	SetDisplayKind(DisplayImageKind::InfectionOverlay);
	AfxMessageBox(message);
}

void C大作业Doc::OnCalculateInfectionMetrics()
{
	if (!HasInfectionSegmentationMask() || !HasInfectionMask())
	{
		AfxMessageBox(_T("请先执行感染区域分割，并打开感染 mask。"));
		return;
	}

	CMetricsCalculator calculator;
	m_lastInfectionMetrics = calculator.Calculate(m_infectionSegmentationMask, m_infectionMask);
	m_hasInfectionMetrics = TRUE;

	CString message;
	message.Format(_T("感染区域 Dice / IoU:\r\n\r\nDice: %.4f\r\nIoU: %.4f\r\nPrecision: %.4f\r\nRecall: %.4f\r\nArea Error: %.4f\r\n\r\nTP: %lld\r\nFP: %lld\r\nTN: %lld\r\nFN: %lld"),
		m_lastInfectionMetrics.dice,
		m_lastInfectionMetrics.iou,
		m_lastInfectionMetrics.precision,
		m_lastInfectionMetrics.recall,
		m_lastInfectionMetrics.areaError,
		m_lastInfectionMetrics.tp,
		m_lastInfectionMetrics.fp,
		m_lastInfectionMetrics.tn,
		m_lastInfectionMetrics.fn);
	AfxMessageBox(message);
}

void C大作业Doc::OnSaveCurrentResult()
{
	const cv::Mat image = CurrentSaveImage();
	if (image.empty())
	{
		AfxMessageBox(_T("当前没有可保存的图像。"));
		return;
	}

	CFileDialog dlg(FALSE, _T("png"), _T("result.png"), OFN_OVERWRITEPROMPT,
		_T("PNG Image (*.png)|*.png|JPEG Image (*.jpg)|*.jpg|Bitmap Image (*.bmp)|*.bmp|TIFF Image (*.tif)|*.tif|All Files (*.*)|*.*||"));
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	if (!ImageIO::SaveImage(dlg.GetPathName().GetString(), image))
	{
		AfxMessageBox(_T("结果图像保存失败。"));
		return;
	}

	AfxMessageBox(_T("结果图像已保存。"));
}

void C大作业Doc::OnExportMetricsCsv()
{
	if (!m_hasMetrics)
	{
		AfxMessageBox(_T("请先计算 Dice / IoU。"));
		return;
	}

	CFileDialog dlg(FALSE, _T("csv"), _T("metrics.csv"), OFN_OVERWRITEPROMPT,
		_T("CSV Files (*.csv)|*.csv|All Files (*.*)|*.*||"));
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	if (!CsvExporter::ExportMetrics(dlg.GetPathName().GetString(), GetSourceFileName(), m_lastMetrics))
	{
		AfxMessageBox(_T("指标 CSV 导出失败。"));
		return;
	}

	AfxMessageBox(_T("指标 CSV 已导出。"));
}

void C大作业Doc::OnExportInfectionCsv()
{
	if (!m_hasInfectionStats)
	{
		AfxMessageBox(_T("请先完成感染负荷分析。"));
		return;
	}

	CFileDialog dlg(FALSE, _T("csv"), _T("infection_stats.csv"), OFN_OVERWRITEPROMPT,
		_T("CSV Files (*.csv)|*.csv|All Files (*.*)|*.*||"));
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	if (!CsvExporter::ExportInfectionStats(dlg.GetPathName().GetString(), GetSourceFileName(), m_lastInfectionStats))
	{
		AfxMessageBox(_T("感染负荷 CSV 导出失败。"));
		return;
	}

	AfxMessageBox(_T("感染负荷 CSV 已导出。"));
}

void C大作业Doc::OnPreprocessGrayNormalize()
{
	CPreprocessor preprocessor;
	m_grayImage = preprocessor.MakeGray8(m_originalImage);
	if (m_grayImage.empty())
	{
		AfxMessageBox(_T("灰度/归一化处理失败。"));
		return;
	}
	SetDisplayKind(DisplayImageKind::Gray);
}

void C大作业Doc::OnPreprocessLungWindow()
{
	const AppConfig config = LoadConfigOrDefaults();
	CPreprocessor preprocessor;
	m_lungWindowImage = preprocessor.ApplyWindow(m_originalImage, config.preprocessing.lungWindowLevel, config.preprocessing.lungWindowWidth);
	if (m_lungWindowImage.empty())
	{
		AfxMessageBox(_T("肺窗处理失败。"));
		return;
	}
	SetDisplayKind(DisplayImageKind::LungWindow);
}

void C大作业Doc::OnPreprocessGaussian()
{
	const AppConfig config = LoadConfigOrDefaults();
	CPreprocessor preprocessor;
	m_gaussianImage = preprocessor.ApplyGaussianBlur(m_originalImage, config.preprocessing.gaussianKernelSize);
	if (m_gaussianImage.empty())
	{
		AfxMessageBox(_T("高斯滤波失败。"));
		return;
	}
	SetDisplayKind(DisplayImageKind::GaussianBlur);
}

void C大作业Doc::OnPreprocessMedian()
{
	const AppConfig config = LoadConfigOrDefaults();
	CPreprocessor preprocessor;
	m_medianImage = preprocessor.ApplyMedianBlur(m_originalImage, config.preprocessing.medianKernelSize);
	if (m_medianImage.empty())
	{
		AfxMessageBox(_T("中值滤波失败。"));
		return;
	}
	SetDisplayKind(DisplayImageKind::MedianBlur);
}

void C大作业Doc::OnPreprocessClahe()
{
	const AppConfig config = LoadConfigOrDefaults();
	CPreprocessor preprocessor;
	m_claheImage = preprocessor.ApplyClahe(
		m_originalImage,
		config.preprocessing.claheClipLimit,
		cv::Size(config.preprocessing.claheTileGridSize, config.preprocessing.claheTileGridSize));
	if (m_claheImage.empty())
	{
		AfxMessageBox(_T("CLAHE 对比度增强失败。"));
		return;
	}
	SetDisplayKind(DisplayImageKind::Clahe);
}

void C大作业Doc::OnShowOriginal()
{
	SetDisplayKind(DisplayImageKind::Original);
}

void C大作业Doc::OnShowGray()
{
	SetDisplayKind(DisplayImageKind::Gray);
}

void C大作业Doc::OnShowLungWindow()
{
	SetDisplayKind(DisplayImageKind::LungWindow);
}

void C大作业Doc::OnShowGaussian()
{
	SetDisplayKind(DisplayImageKind::GaussianBlur);
}

void C大作业Doc::OnShowMedian()
{
	SetDisplayKind(DisplayImageKind::MedianBlur);
}

void C大作业Doc::OnShowClahe()
{
	SetDisplayKind(DisplayImageKind::Clahe);
}

void C大作业Doc::OnShowThreshold()
{
	SetDisplayKind(DisplayImageKind::Threshold);
}

void C大作业Doc::OnShowConnected()
{
	SetDisplayKind(DisplayImageKind::Connected);
}

void C大作业Doc::OnShowMorphology()
{
	SetDisplayKind(DisplayImageKind::Morphology);
}

void C大作业Doc::OnShowFinalMask()
{
	SetDisplayKind(DisplayImageKind::FinalMask);
}

void C大作业Doc::OnShowManualMask()
{
	SetDisplayKind(DisplayImageKind::ManualMask);
}

void C大作业Doc::OnShowConnectedColorMap()
{
	SetDisplayKind(DisplayImageKind::ConnectedColorMap);
}

void C大作业Doc::OnShowMaskComparisonOverlay()
{
	SetDisplayKind(DisplayImageKind::MaskComparisonOverlay);
}

void C大作业Doc::OnShowInfectionMask()
{
	SetDisplayKind(DisplayImageKind::InfectionMask);
}

void C大作业Doc::OnShowInfectionOverlay()
{
	SetDisplayKind(DisplayImageKind::InfectionOverlay);
}

void C大作业Doc::OnShowInfectionSegmentationMask()
{
	SetDisplayKind(DisplayImageKind::InfectionSegmentationMask);
}

void C大作业Doc::OnPreviousSlice()
{
	if (!CanMoveToPreviousSlice())
	{
		return;
	}

	--m_currentSliceIndex;
	ApplyCurrentSlice();
}

void C大作业Doc::OnNextSlice()
{
	if (!CanMoveToNextSlice())
	{
		return;
	}

	++m_currentSliceIndex;
	ApplyCurrentSlice();
}

void C大作业Doc::OnBatchProcessCurrentVolume()
{
	if (!CanBatchProcessCurrentData())
	{
		AfxMessageBox(_T("请先打开 CT 图像或 3D 体数据。"));
		return;
	}

	CFolderPickerDialog dlg(nullptr, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST);
	dlg.m_ofn.lpstrTitle = _T("选择批量处理输出目录");
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	std::vector<cv::Mat> sourceSlices;
	std::vector<cv::Mat> gtMaskSlices;
	std::vector<cv::Mat> infectionMaskSlices;

	if (!m_sourceVolume.slices.empty())
	{
		sourceSlices = m_sourceVolume.slices;
	}
	else if (!m_originalImage.empty())
	{
		sourceSlices.push_back(m_originalImage);
	}

	if (!m_manualMaskVolume.slices.empty())
	{
		gtMaskSlices = m_manualMaskVolume.slices;
	}
	else if (!m_manualMask.empty())
	{
		gtMaskSlices.push_back(m_manualMask);
	}

	if (!m_infectionMaskVolume.slices.empty())
	{
		infectionMaskSlices = m_infectionMaskVolume.slices;
	}
	else if (!m_infectionMask.empty())
	{
		infectionMaskSlices.push_back(m_infectionMask);
	}

	BatchOptions options;
	options.outputRoot = dlg.GetPathName().GetString();
	options.caseName = GetSourceFileName();
	const AppConfig config = LoadConfigOrDefaults();
	options.saveIntermediate = config.saveIntermediate;
	options.segmentationOptions = config.segmentation;

	CWaitCursor wait;
	CBatchProcessor processor;
	BatchProcessResult result;
	CString error;
	if (!processor.ProcessSlices(sourceSlices, gtMaskSlices, infectionMaskSlices, options, result, error))
	{
		AfxMessageBox(error);
		return;
	}

	CString message;
	message.Format(_T("批量处理完成。\r\n总切片数: %d\r\n成功处理: %d\r\n计算指标切片: %d\r\n感染统计切片: %d\r\n\r\n汇总 CSV:\r\n%s\r\n\r\n运行参数:\r\n%s"),
		result.totalSlices,
		result.processedSlices,
		result.metricSlices,
		result.infectionSlices,
		result.summaryCsvPath.GetString(),
		result.manifestPath.GetString());
	AfxMessageBox(message);
}

void C大作业Doc::OnUpdateHasOriginal(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasOriginalImage());
}

void C大作业Doc::OnUpdateHasPreprocessedGray(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasPreprocessedGray());
}

void C大作业Doc::OnUpdateHasLungWindowImage(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasLungWindowImage());
}

void C大作业Doc::OnUpdateHasGaussianImage(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasGaussianImage());
}

void C大作业Doc::OnUpdateHasMedianImage(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasMedianImage());
}

void C大作业Doc::OnUpdateHasClaheImage(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasClaheImage());
}

void C大作业Doc::OnUpdateHasSegmentation(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasFinalMask());
}

void C大作业Doc::OnUpdateHasFinalMask(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasFinalMask());
}

void C大作业Doc::OnUpdateHasFinalAndMask(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasFinalMask() && HasManualMask());
}

void C大作业Doc::OnUpdateHasManualMask(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasManualMask());
}

void C大作业Doc::OnUpdateHasConnectedColorMap(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasConnectedColorMap());
}

void C大作业Doc::OnUpdateHasMaskComparisonOverlay(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasMaskComparisonOverlay());
}

void C大作业Doc::OnUpdateHasInfectionMask(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasInfectionMask());
}

void C大作业Doc::OnUpdateHasInfectionSegmentationMask(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasInfectionSegmentationMask());
}

void C大作业Doc::OnUpdateHasFinalAndInfection(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasFinalMask() && HasInfectionMask());
}

void C大作业Doc::OnUpdateHasMetrics(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasMetrics());
}

void C大作业Doc::OnUpdateHasInfectionStats(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasInfectionStats());
}

void C大作业Doc::OnUpdateHasInfectionMetrics(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CanCalculateInfectionMetrics());
}

void C大作业Doc::OnUpdateCanPreviousSlice(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CanMoveToPreviousSlice());
}

void C大作业Doc::OnUpdateCanNextSlice(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CanMoveToNextSlice());
}

void C大作业Doc::OnUpdateCanBatchProcessCurrentData(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CanBatchProcessCurrentData());
}
