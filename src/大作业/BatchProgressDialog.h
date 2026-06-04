#pragma once

#include "BatchProcessor.h"

class CBatchProgressDialog : public CDialogEx
{
public:
	explicit CBatchProgressDialog(CWnd* parent = nullptr) noexcept;

	BOOL IsCancelRequested() const noexcept;
	bool UpdateProgress(const BatchProgressInfo& info);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BATCH_PROGRESS };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

private:
	CProgressCtrl m_progress;
	CStatic m_statusText;
	BOOL m_cancelRequested = FALSE;

	bool PumpMessages();
};
