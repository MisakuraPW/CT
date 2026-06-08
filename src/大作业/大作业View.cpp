
// 大作业View.cpp: C大作业View 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "大作业.h"
#endif

#include "大作业Doc.h"
#include "大作业View.h"

#include <opencv2/imgproc.hpp>

#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// C大作业View

IMPLEMENT_DYNCREATE(C大作业View, CView)

BEGIN_MESSAGE_MAP(C大作业View, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &C大作业View::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_HSCROLL, &C大作业View::OnHScrollMsg)
	ON_WM_SIZE()
END_MESSAGE_MAP()

// C大作业View 构造/析构

C大作业View::C大作业View() noexcept
{
	// TODO: 在此处添加构造代码

}

C大作业View::~C大作业View()
{
}

BOOL C大作业View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// C大作业View 绘图

void C大作业View::OnDraw(CDC* pDC)
{
	C大作业Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CRect clientRect;
	GetClientRect(&clientRect);

	// 计算图像显示区域（为滑块预留底部空间）
	CRect imageRect = clientRect;
	if (m_sliderCtrl.GetSafeHwnd() && m_sliderCtrl.IsWindowVisible())
	{
		CRect sliderRect;
		m_sliderCtrl.GetWindowRect(&sliderRect);
		ScreenToClient(&sliderRect);
		imageRect.bottom = sliderRect.top;
	}

	pDC->FillSolidRect(clientRect, RGB(32, 32, 32));

	const cv::Mat& image = pDoc->GetDisplayImage();
	if (image.empty())
	{
		pDC->SetTextColor(RGB(230, 230, 230));
		pDC->SetBkMode(TRANSPARENT);
		pDC->DrawText(_T("请使用“文件 -> 打开”加载一张 CT 图像。"), clientRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		return;
	}

	cv::Mat displayImage;
	if (image.depth() == CV_8U)
	{
		displayImage = image;
	}
	else
	{
		cv::normalize(image, displayImage, 0, 255, cv::NORM_MINMAX, CV_8U);
	}

	cv::Mat bgr;
	if (displayImage.channels() == 1)
	{
		cv::cvtColor(displayImage, bgr, cv::COLOR_GRAY2BGR);
	}
	else if (displayImage.channels() == 3)
	{
		bgr = displayImage;
	}
	else if (displayImage.channels() == 4)
	{
		cv::cvtColor(displayImage, bgr, cv::COLOR_BGRA2BGR);
	}
	else
	{
		return;
	}

	cv::Mat bgra;
	cv::cvtColor(bgr, bgra, cv::COLOR_BGR2BGRA);

	const double scaleX = static_cast<double>(imageRect.Width()) / bgra.cols;
	const double scaleY = static_cast<double>(imageRect.Height()) / bgra.rows;
	const double scale = std::min(scaleX, scaleY);
	const int drawWidth = std::max(1, static_cast<int>(bgra.cols * scale));
	const int drawHeight = std::max(1, static_cast<int>(bgra.rows * scale));
	const int drawX = imageRect.left + (imageRect.Width() - drawWidth) / 2;
	const int drawY = imageRect.top + (imageRect.Height() - drawHeight) / 2;

	BITMAPINFO bitmapInfo = {};
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biWidth = bgra.cols;
	bitmapInfo.bmiHeader.biHeight = -bgra.rows;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	StretchDIBits(
		pDC->GetSafeHdc(),
		drawX,
		drawY,
		drawWidth,
		drawHeight,
		0,
		0,
		bgra.cols,
		bgra.rows,
		bgra.data,
		&bitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY);

	CRect titleRect = imageRect;
	titleRect.top += 8;
	titleRect.left += 12;
	pDC->SetTextColor(RGB(245, 245, 245));
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(pDoc->GetDisplayName(), titleRect, DT_LEFT | DT_TOP | DT_SINGLELINE);
}


// C大作业View 打印


void C大作业View::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL C大作业View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void C大作业View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void C大作业View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void C大作业View::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void C大作业View::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}

BOOL C大作业View::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void C大作业View::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// 创建滑块控件（如果尚未创建）
	if (!m_sliderCtrl.GetSafeHwnd())
	{
		m_sliderCtrl.Create(WS_CHILD | TBS_HORZ | TBS_AUTOTICKS,
			CRect(0, 0, 0, 0), this, 1001);
	}

	// 初始化滑块位置和大小
	OnUpdate(nullptr, 0, nullptr);
}

void C大作业View::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	C大作业Doc* pDoc = GetDocument();
	if (!pDoc)
	{
		CView::OnUpdate(pSender, lHint, pHint);
		return;
	}

	// 如果有3D体数据，显示并配置滑块
	if (pDoc->HasVolume())
	{
		int depth = pDoc->GetVolumeDepth();
		if (depth > 1 && m_sliderCtrl.GetSafeHwnd())
		{
			m_sliderCtrl.SetRange(0, depth - 1, TRUE);
			m_sliderCtrl.SetPos(pDoc->GetCurrentSliceIndex());
			m_sliderCtrl.ShowWindow(SW_SHOW);
		}
	}
	else
	{
		// 单张图像，隐藏滑块
		if (m_sliderCtrl.GetSafeHwnd())
		{
			m_sliderCtrl.ShowWindow(SW_HIDE);
		}
	}

	CView::OnUpdate(pSender, lHint, pHint);
}

LRESULT C大作业View::OnHScrollMsg(WPARAM wParam, LPARAM lParam)
{
	// 检查消息是否来自我们的滑块控件
	if (lParam == 0)
		return 0;

	HWND hwndSlider = (HWND)lParam;
	if (!m_sliderCtrl.GetSafeHwnd() || hwndSlider != m_sliderCtrl.GetSafeHwnd())
		return 0;

	UINT nSBCode = LOWORD(wParam);
	
	// 处理滑块拖动事件
	if (nSBCode == TB_THUMBTRACK || nSBCode == TB_ENDTRACK)
	{
		C大作业Doc* pDoc = GetDocument();
		if (pDoc && pDoc->HasVolume())
		{
			int pos = m_sliderCtrl.GetPos();
			int maxSlice = pDoc->GetVolumeDepth() - 1;
			
			// 确保位置在有效范围内
			if (pos >= 0 && pos <= maxSlice)
			{
				pDoc->SetSliceIndex(pos);
			}
		}
	}
	
	return 0;
}

void C大作业View::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// 调整滑块控件位置
	if (m_sliderCtrl.GetSafeHwnd() && cy > 40 && cx > 0)
	{
		const int sliderHeight = 30;
		const int margin = 10;
		
		// 将滑块放置在窗口底部，左右留有边距
		m_sliderCtrl.SetWindowPos(nullptr, 
			margin, 
			cy - sliderHeight - 5, 
			cx - 2 * margin, 
			sliderHeight, 
			SWP_NOZORDER | SWP_SHOWWINDOW);
	}
	else if (m_sliderCtrl.GetSafeHwnd())
	{
		m_sliderCtrl.ShowWindow(SW_HIDE);
	}
}


// C大作业View 诊断

#ifdef _DEBUG
void C大作业View::AssertValid() const
{
	CView::AssertValid();
}

void C大作业View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

C大作业Doc* C大作业View::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(C大作业Doc)));
	return (C大作业Doc*)m_pDocument;
}
#endif //_DEBUG


// C大作业View 消息处理程序
