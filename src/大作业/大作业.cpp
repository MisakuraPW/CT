
// 大作业.cpp: 定义应用程序的类行为。
//

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "大作业.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "大作业Doc.h"
#include "大作业View.h"
#include "AppConfig.h"
#include "DatasetBatchRunner.h"
#include "DatasetScanner.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// C大作业App

BEGIN_MESSAGE_MAP(C大作业App, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &C大作业App::OnAppAbout)
	// 基于文件的标准文档命令
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &C大作业App::OnFileOpen)
	ON_COMMAND(ID_DATASET_SCAN_TASKS, &C大作业App::OnDatasetScanTasks)
	ON_COMMAND(ID_DATASET_PROCESS_CONFIGURED, &C大作业App::OnDatasetProcessConfigured)
	// 标准打印设置命令
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// C大作业App 构造

C大作业App::C大作业App() noexcept
{
	m_bHiColorIcons = TRUE;


	m_nAppLook = 0;
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// 如果应用程序是利用公共语言运行时支持(/clr)构建的，则: 
	//     1) 必须有此附加设置，“重新启动管理器”支持才能正常工作。
	//     2) 在您的项目中，您必须按照生成顺序向 System.Windows.Forms 添加引用。
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: 将以下应用程序 ID 字符串替换为唯一的 ID 字符串；建议的字符串格式
	//为 CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("大作业.AppID.NoVersion"));

	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}

// 唯一的 C大作业App 对象

C大作业App theApp;


// C大作业App 初始化

BOOL C大作业App::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	// 初始化 OLE 库
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction();

	// 使用 RichEdit 控件需要 AfxInitRichEdit2()
	// AfxInitRichEdit2();

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));
	LoadStdProfileSettings(4);  // 加载标准 INI 文件选项(包括 MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// 注册应用程序的文档模板。  文档模板
	// 将用作文档、框架窗口和视图之间的连接
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_MyTYPE,
		RUNTIME_CLASS(C大作业Doc),
		RUNTIME_CLASS(CChildFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(C大作业View));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// 创建主 MDI 框架窗口
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;


	// 分析标准 shell 命令、DDE、打开文件操作的命令行
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// 调度在命令行中指定的命令。  如果
	// 用 /RegServer、/Register、/Unregserver 或 /Unregister 启动应用程序，则返回 FALSE。
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// 主窗口已初始化，因此显示它并对其进行更新
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

int C大作业App::ExitInstance()
{
	//TODO: 处理可能已添加的附加资源
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// C大作业App 消息处理程序


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// 用于运行对话框的应用程序命令
void C大作业App::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void C大作业App::OnFileOpen()
{
	CFileDialog dlg(TRUE, nullptr, nullptr, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		_T("Medical/Image Files (*.nii;*.nii.gz;*.tif;*.tiff;*.png;*.jpg;*.jpeg;*.bmp)|*.nii;*.nii.gz;*.tif;*.tiff;*.png;*.jpg;*.jpeg;*.bmp|NIfTI Files (*.nii;*.nii.gz)|*.nii;*.nii.gz|Image Files (*.tif;*.tiff;*.png;*.jpg;*.jpeg;*.bmp)|*.tif;*.tiff;*.png;*.jpg;*.jpeg;*.bmp|All Files (*.*)|*.*||"));
	if (dlg.DoModal() == IDOK)
	{
		OpenDocumentFile(dlg.GetPathName());
	}
}

void C大作业App::OnDatasetScanTasks()
{
	AppConfig config;
	CString errorMessage;
	if (!CAppConfigLoader::LoadDefault(config, errorMessage))
	{
		AfxMessageBox(errorMessage, MB_ICONWARNING);
		return;
	}

	CDatasetScanner scanner;
	DatasetScanResult result;
	if (!scanner.Scan(config.lungDatasetRoot, config.covidDatasetRoot, result, errorMessage))
	{
		AfxMessageBox(errorMessage, MB_ICONWARNING);
		return;
	}

	CString outputPath = config.resultRoot;
	outputPath.Replace(_T('/'), _T('\\'));
	if (!outputPath.IsEmpty())
	{
		const TCHAR tail = outputPath[outputPath.GetLength() - 1];
		if (tail != _T('\\'))
		{
			outputPath += _T("\\");
		}
	}
	outputPath += _T("dataset_tasks.csv");

	if (!scanner.ExportTaskListCsv(outputPath, result, errorMessage))
	{
		AfxMessageBox(errorMessage, MB_ICONERROR);
		return;
	}

	CString summary;
	summary.Format(
		_T("数据集任务扫描完成。\n\nFinding Lungs 2D: %d\nFinding Lungs 3D: %d\nCOVID-19 CT: %d\n缺失标注: %d\n\n任务清单已写入：\n%s"),
		result.finding2DCount,
		result.finding3DCount,
		result.covidCount,
		result.missingMaskCount,
		outputPath.GetString());
	AfxMessageBox(summary, MB_ICONINFORMATION);
}

void C大作业App::OnDatasetProcessConfigured()
{
	AppConfig config;
	CString errorMessage;
	if (!CAppConfigLoader::LoadDefault(config, errorMessage))
	{
		AfxMessageBox(errorMessage, MB_ICONWARNING);
		return;
	}

	CDatasetScanner scanner;
	DatasetScanResult scanResult;
	if (!scanner.Scan(config.lungDatasetRoot, config.covidDatasetRoot, scanResult, errorMessage))
	{
		AfxMessageBox(errorMessage, MB_ICONWARNING);
		return;
	}

	CString confirm;
	confirm.Format(
		_T("即将处理配置中的全部数据集任务。\n\nFinding Lungs 2D: %d\nFinding Lungs 3D: %d\nCOVID-19 CT: %d\n\n处理过程可能需要较长时间，期间窗口会暂时无响应。是否继续？"),
		scanResult.finding2DCount,
		scanResult.finding3DCount,
		scanResult.covidCount);
	if (AfxMessageBox(confirm, MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;
	}

	CString outputRoot = config.resultRoot;
	outputRoot.Replace(_T('/'), _T('\\'));
	if (!outputRoot.IsEmpty())
	{
		const TCHAR tail = outputRoot[outputRoot.GetLength() - 1];
		if (tail != _T('\\'))
		{
			outputRoot += _T("\\");
		}
	}
	outputRoot += _T("datasets");

	DatasetBatchOptions options;
	options.outputRoot = outputRoot;
	options.saveIntermediate = config.saveIntermediate;
	options.segmentationOptions = config.segmentation;

	DatasetBatchSummary summary;
	CDatasetBatchRunner runner;
	{
		CWaitCursor wait;
		if (!runner.Run(scanResult, options, summary, errorMessage))
		{
			AfxMessageBox(errorMessage, MB_ICONERROR);
			return;
		}
	}

	CString message;
	message.Format(
		_T("数据集批处理完成。\n\n病例总数: %d\n成功: %d\n失败: %d\n切片总数: %d\n\n汇总表：\n%s"),
		summary.totalCases,
		summary.succeededCases,
		summary.failedCases,
		summary.totalSlices,
		summary.summaryCsvPath.GetString());
	AfxMessageBox(message, summary.failedCases > 0 ? MB_ICONWARNING : MB_ICONINFORMATION);
}

// C大作业App 自定义加载/保存方法

void C大作业App::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void C大作业App::LoadCustomState()
{
}

void C大作业App::SaveCustomState()
{
}

// C大作业App 消息处理程序



