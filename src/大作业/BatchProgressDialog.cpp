#include "pch.h"
#include "BatchProgressDialog.h"

#include "resource.h"

BEGIN_MESSAGE_MAP(CBatchProgressDialog, CDialogEx)
END_MESSAGE_MAP()

CBatchProgressDialog::CBatchProgressDialog(CWnd* parent) noexcept
	: CDialogEx(IDD_BATCH_PROGRESS, parent)
{
}

BOOL CBatchProgressDialog::IsCancelRequested() const noexcept
{
	return m_cancelRequested;
}

BOOL CBatchProgressDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_progress.SetRange32(0, 100);
	m_progress.SetPos(0);
	m_statusText.SetWindowText(_T("正在准备批处理..."));
	return TRUE;
}

void CBatchProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_BATCH, m_progress);
	DDX_Control(pDX, IDC_STATIC_PROGRESS_STATUS, m_statusText);
}

void CBatchProgressDialog::OnCancel()
{
	m_cancelRequested = TRUE;
	m_statusText.SetWindowText(_T("正在取消，请等待当前切片处理结束..."));
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
}

bool CBatchProgressDialog::UpdateProgress(const BatchProgressInfo& info)
{
	CString status;
	if (info.total > 0)
	{
		status.Format(
			_T("%s：%d / %d\r\n%s"),
			info.scopeName.GetString(),
			info.current,
			info.total,
			info.itemName.GetString());
		m_progress.SetRange32(0, info.total);
		m_progress.SetPos(info.current);
	}
	else
	{
		status.Format(_T("%s\r\n%s"), info.scopeName.GetString(), info.itemName.GetString());
		m_progress.SetRange32(0, 100);
		m_progress.SetPos(0);
	}

	m_statusText.SetWindowText(status);
	return !m_cancelRequested && PumpMessages();
}

bool CBatchProgressDialog::PumpMessages()
{
	MSG msg;
	while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			::PostQuitMessage(static_cast<int>(msg.wParam));
			m_cancelRequested = TRUE;
			return false;
		}

		if (!IsDialogMessage(&msg))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	return !m_cancelRequested;
}
