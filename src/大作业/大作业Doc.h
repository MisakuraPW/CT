
// 大作业Doc.h: C大作业Doc 类的接口
//


#pragma once

#include "InfectionAnalyzer.h"
#include "LungSegmenter.h"
#include "MetricsCalculator.h"
#include "NiftiIO.h"

#include <opencv2/core.hpp>

enum class DisplayImageKind
{
	Original,
	Gray,
	LungWindow,
	GaussianBlur,
	MedianBlur,
	Clahe,
	Threshold,
	Connected,
	Morphology,
	FinalMask,
	ManualMask,
	ConnectedColorMap,
	MaskComparisonOverlay,
	InfectionMask,
	InfectionOverlay,
	InfectionSegmentationMask
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
	BOOL HasPreprocessedGray() const;
	BOOL HasLungWindowImage() const;
	BOOL HasGaussianImage() const;
	BOOL HasMedianImage() const;
	BOOL HasClaheImage() const;
	BOOL HasFinalMask() const;
	BOOL HasManualMask() const;
	BOOL HasInfectionMask() const;
	BOOL HasInfectionSegmentationMask() const;
	BOOL HasConnectedColorMap() const;
	BOOL HasMaskComparisonOverlay() const;
	BOOL HasMetrics() const;
	BOOL HasInfectionStats() const;
	BOOL HasInfectionMetrics() const;
	BOOL HasVolume() const;
	BOOL CanMoveToPreviousSlice() const;
	BOOL CanMoveToNextSlice() const;
	BOOL CanBatchProcessCurrentData() const;
	BOOL CanCalculateInfectionMetrics() const;

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
	cv::Mat m_grayImage;
	cv::Mat m_lungWindowImage;
	cv::Mat m_gaussianImage;
	cv::Mat m_medianImage;
	cv::Mat m_claheImage;
	cv::Mat m_manualMask;
	cv::Mat m_infectionMask;
	cv::Mat m_infectionSegmentationMask;
	cv::Mat m_connectedColorMap;
	cv::Mat m_maskComparisonOverlay;
	cv::Mat m_infectionOverlay;
	NiftiVolume m_sourceVolume;
	NiftiVolume m_manualMaskVolume;
	NiftiVolume m_infectionMaskVolume;
	LungSegmentationResult m_segmentationResult;
	DisplayImageKind m_displayKind = DisplayImageKind::Original;
	int m_currentSliceIndex = 0;
	CString m_sourcePath;
	CString m_manualMaskPath;
	CString m_infectionMaskPath;
	SegmentationMetrics m_lastMetrics;
	InfectionStats m_lastInfectionStats;
	SegmentationMetrics m_lastInfectionMetrics;
	BOOL m_hasMetrics = FALSE;
	BOOL m_hasInfectionStats = FALSE;
	BOOL m_hasInfectionMetrics = FALSE;

	BOOL LoadSourceImage(const CString& pathName);
	BOOL LoadManualMask(const CString& pathName);
	BOOL LoadInfectionMask(const CString& pathName);
	cv::Mat SliceOrEmpty(const NiftiVolume& volume, int sliceIndex) const;
	void ApplyCurrentSlice();
	void ClearSliceDerivedResults();
	void ClearImages();
	void SetDisplayKind(DisplayImageKind kind);
	cv::Mat CurrentSaveImage() const;
	CString GetSourceFileName() const;

// 生成的消息映射函数
protected:
	afx_msg void OnOpenManualMask();
	afx_msg void OnOpenInfectionMask();
	afx_msg void OnRunLungSegmentation();
	afx_msg void OnSegmentInfection();
	afx_msg void OnCalculateMetrics();
	afx_msg void OnAnalyzeInfectionBurden();
	afx_msg void OnCalculateInfectionMetrics();
	afx_msg void OnSaveCurrentResult();
	afx_msg void OnExportMetricsCsv();
	afx_msg void OnExportInfectionCsv();
	afx_msg void OnPreprocessGrayNormalize();
	afx_msg void OnPreprocessLungWindow();
	afx_msg void OnPreprocessGaussian();
	afx_msg void OnPreprocessMedian();
	afx_msg void OnPreprocessClahe();
	afx_msg void OnShowOriginal();
	afx_msg void OnShowGray();
	afx_msg void OnShowLungWindow();
	afx_msg void OnShowGaussian();
	afx_msg void OnShowMedian();
	afx_msg void OnShowClahe();
	afx_msg void OnShowThreshold();
	afx_msg void OnShowConnected();
	afx_msg void OnShowMorphology();
	afx_msg void OnShowFinalMask();
	afx_msg void OnShowManualMask();
	afx_msg void OnShowConnectedColorMap();
	afx_msg void OnShowMaskComparisonOverlay();
	afx_msg void OnShowInfectionMask();
	afx_msg void OnShowInfectionOverlay();
	afx_msg void OnShowInfectionSegmentationMask();
	afx_msg void OnPreviousSlice();
	afx_msg void OnNextSlice();
	afx_msg void OnBatchProcessCurrentVolume();
	afx_msg void OnUpdateHasOriginal(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasPreprocessedGray(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasLungWindowImage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasGaussianImage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasMedianImage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasClaheImage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasSegmentation(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasFinalMask(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasFinalAndMask(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasManualMask(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasConnectedColorMap(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasMaskComparisonOverlay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasInfectionMask(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasInfectionSegmentationMask(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasFinalAndInfection(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasMetrics(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasInfectionStats(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHasInfectionMetrics(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCanPreviousSlice(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCanNextSlice(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCanBatchProcessCurrentData(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 用于为搜索处理程序设置搜索内容的 Helper 函数
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
