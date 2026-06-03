
// 大作业Doc.h: C大作业Doc 类的接口
//


#pragma once

#include "LungSegmenter.h"
#include "MetricsCalculator.h"

#include <opencv2/core.hpp>

enum class DisplayImageKind
{
	Original,
	Gray,
	Threshold,
	Connected,
	Morphology,
	FinalMask,
	ManualMask
};

class C大作业Doc : public CDocument
{
protected: // 仅从序列化创建
	C大作业Doc() noexcept;
	DECLARE_DYNCREATE(C大作业Doc)

// 特性
public:

// 操作
public:
	const cv::Mat& GetDisplayImage() const;
	CString GetDisplayName() const;
	BOOL HasOriginalImage() const;
	BOOL HasFinalMask() const;
	BOOL HasManualMask() const;

// 重写
public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// 实现
public:
	virtual ~C大作业Doc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	cv::Mat m_originalImage;
	cv::Mat m_manualMask;
	LungSegmentationResult m_segmentationResult;
	DisplayImageKind m_displayKind = DisplayImageKind::Original;
	CString m_sourcePath;
	CString m_manualMaskPath;
	SegmentationMetrics m_lastMetrics;

	BOOL LoadSourceImage(const CString& pathName);
	BOOL LoadManualMask(const CString& pathName);
	void ClearImages();
	void SetDisplayKind(DisplayImageKind kind);
	cv::Mat CurrentSaveImage() const;

// 生成的消息映射函数
protected:
	afx_msg void OnOpenManualMask();
	afx_msg void OnRunLungSegmentation();
	afx_msg void OnCalculateMetrics();
	afx_msg void OnSaveCurrentResult();
	afx_msg void OnShowOriginal();
	afx_msg void OnShowGray();
	afx_msg void OnShowThreshold();
	afx_msg void OnShowConnected();
	afx_msg void OnShowMorphology();
	afx_msg void OnShowFinalMask();
	afx_msg void OnShowManualMask();
	afx_msg void OnUpdateHasOriginal(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasSegmentation(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasFinalAndMask(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasManualMask(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 用于为搜索处理程序设置搜索内容的 Helper 函数
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
