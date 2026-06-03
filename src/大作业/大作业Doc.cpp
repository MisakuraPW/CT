
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

#include "ImageIO.h"
#include "LungSegmenter.h"
#include "MetricsCalculator.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// C大作业Doc

IMPLEMENT_DYNCREATE(C大作业Doc, CDocument)

BEGIN_MESSAGE_MAP(C大作业Doc, CDocument)
	ON_COMMAND(ID_IMAGE_OPEN_MASK, &C大作业Doc::OnOpenManualMask)
	ON_COMMAND(ID_PROCESS_RUN_SEGMENTATION, &C大作业Doc::OnRunLungSegmentation)
	ON_COMMAND(ID_ANALYSIS_CALCULATE_METRICS, &C大作业Doc::OnCalculateMetrics)
	ON_COMMAND(ID_RESULT_SAVE_CURRENT, &C大作业Doc::OnSaveCurrentResult)
	ON_COMMAND(ID_VIEW_SHOW_ORIGINAL, &C大作业Doc::OnShowOriginal)
	ON_COMMAND(ID_VIEW_SHOW_GRAY, &C大作业Doc::OnShowGray)
	ON_COMMAND(ID_VIEW_SHOW_THRESHOLD, &C大作业Doc::OnShowThreshold)
	ON_COMMAND(ID_VIEW_SHOW_CONNECTED, &C大作业Doc::OnShowConnected)
	ON_COMMAND(ID_VIEW_SHOW_MORPHOLOGY, &C大作业Doc::OnShowMorphology)
	ON_COMMAND(ID_VIEW_SHOW_FINAL_MASK, &C大作业Doc::OnShowFinalMask)
	ON_COMMAND(ID_VIEW_SHOW_MANUAL_MASK, &C大作业Doc::OnShowManualMask)
	ON_UPDATE_COMMAND_UI(ID_IMAGE_OPEN_MASK, &C大作业Doc::OnUpdateHasOriginal)
	ON_UPDATE_COMMAND_UI(ID_PROCESS_RUN_SEGMENTATION, &C大作业Doc::OnUpdateHasOriginal)
	ON_UPDATE_COMMAND_UI(ID_ANALYSIS_CALCULATE_METRICS, &C大作业Doc::OnUpdateHasFinalAndMask)
	ON_UPDATE_COMMAND_UI(ID_RESULT_SAVE_CURRENT, &C大作业Doc::OnUpdateHasOriginal)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_ORIGINAL, &C大作业Doc::OnUpdateHasOriginal)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_GRAY, &C大作业Doc::OnUpdateHasSegmentation)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_THRESHOLD, &C大作业Doc::OnUpdateHasSegmentation)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_CONNECTED, &C大作业Doc::OnUpdateHasSegmentation)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_MORPHOLOGY, &C大作业Doc::OnUpdateHasSegmentation)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_FINAL_MASK, &C大作业Doc::OnUpdateHasSegmentation)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_MANUAL_MASK, &C大作业Doc::OnUpdateHasManualMask)
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
		return m_segmentationResult.gray.empty() ? m_originalImage : m_segmentationResult.gray;
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
	case DisplayImageKind::Original:
	default:
		return m_originalImage;
	}
}

CString C大作业Doc::GetDisplayName() const
{
	switch (m_displayKind)
	{
	case DisplayImageKind::Gray:
		return _T("灰度/归一化图像");
	case DisplayImageKind::Threshold:
		return _T("Otsu 阈值分割结果");
	case DisplayImageKind::Connected:
		return _T("连通域筛选结果");
	case DisplayImageKind::Morphology:
		return _T("形态学修复结果");
	case DisplayImageKind::FinalMask:
		return _T("最终肺部 mask");
	case DisplayImageKind::ManualMask:
		return _T("人工 mask");
	case DisplayImageKind::Original:
	default:
		return _T("原始图像");
	}
}

BOOL C大作业Doc::HasOriginalImage() const
{
	return !m_originalImage.empty();
}

BOOL C大作业Doc::HasFinalMask() const
{
	return !m_segmentationResult.finalMask.empty();
}

BOOL C大作业Doc::HasManualMask() const
{
	return !m_manualMask.empty();
}

BOOL C大作业Doc::LoadSourceImage(const CString& pathName)
{
	ClearImages();

	m_originalImage = ImageIO::LoadImage(pathName.GetString(), cv::IMREAD_UNCHANGED);
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
	m_manualMask = ImageIO::LoadImage(pathName.GetString(), cv::IMREAD_GRAYSCALE);
	if (m_manualMask.empty())
	{
		return FALSE;
	}

	cv::threshold(m_manualMask, m_manualMask, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
	m_manualMaskPath = pathName;
	SetDisplayKind(DisplayImageKind::ManualMask);
	return TRUE;
}

void C大作业Doc::ClearImages()
{
	m_originalImage.release();
	m_manualMask.release();
	m_segmentationResult = LungSegmentationResult{};
	m_displayKind = DisplayImageKind::Original;
	m_sourcePath.Empty();
	m_manualMaskPath.Empty();
	m_lastMetrics = SegmentationMetrics{};
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

void C大作业Doc::OnOpenManualMask()
{
	CFileDialog dlg(TRUE, nullptr, nullptr, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		_T("Image Files (*.png;*.jpg;*.jpeg;*.bmp;*.tif;*.tiff)|*.png;*.jpg;*.jpeg;*.bmp;*.tif;*.tiff|All Files (*.*)|*.*||"));
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	if (!LoadManualMask(dlg.GetPathName()))
	{
		AfxMessageBox(_T("人工 mask 读取失败。"));
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
	CLungSegmenter segmenter;
	m_segmentationResult = segmenter.Segment(m_originalImage);
	if (m_segmentationResult.finalMask.empty())
	{
		AfxMessageBox(_T("肺部分割失败。"));
		return;
	}

	SetDisplayKind(DisplayImageKind::FinalMask);
	AfxMessageBox(_T("肺部分割完成。可在“视图”菜单切换查看中间结果。"));
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

void C大作业Doc::OnShowOriginal()
{
	SetDisplayKind(DisplayImageKind::Original);
}

void C大作业Doc::OnShowGray()
{
	SetDisplayKind(DisplayImageKind::Gray);
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

void C大作业Doc::OnUpdateHasOriginal(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasOriginalImage());
}

void C大作业Doc::OnUpdateHasSegmentation(CCmdUI* pCmdUI)
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
