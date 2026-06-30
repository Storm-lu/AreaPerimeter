// ====================================================================
// 文件名: DSplit.cpp
// 描  述: MFC 应用程序主类的实现
//   定义应用程序的初始化流程，包括：
//     1. 注册 SDI 单文档模板 (CDSplitDoc + CMainFrame + CDSplitView)
//     2. 解析命令行参数并处理文件打开
//     3. 显示主窗口
// ====================================================================

#include "stdafx.h"
#include "DSplit.h"

#include "MainFrm.h"
#include "DSplitDoc.h"
#include "DSplitView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDSplitApp - 消息映射
//   将菜单命令 ID_APP_ABOUT 绑定到 OnAppAbout 处理函数

BEGIN_MESSAGE_MAP(CDSplitApp, CWinApp)
	//{{AFX_MSG_MAP(CDSplitApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDSplitApp construction

CDSplitApp::CDSplitApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDSplitApp object - 全局唯一应用实例

CDSplitApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDSplitApp initialization
//   InitInstance(): 应用程序初始化主流程
//   1) 启用 3D 控件样式
//   2) 设置注册表键用于存储应用设置
//   3) 注册 SDI 文档模板 (文档-框架-视图绑定)
//   4) 解析命令行并显示主窗口

BOOL CDSplitApp::InitInstance()
{
#ifdef _AFXDLL
	Enable3dControls();			// 使用 MFC 共享 DLL 时调用
#else
	Enable3dControlsStatic();	// 静态链接 MFC 时调用
#endif

	// 修改注册表键名（用于保存 MRU 等设置）
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // 加载标准 INI 文件选项 (包括 MRU 最近文件列表)

	// 注册 SDI 单文档模板，绑定文档-框架-视图三个核心类
	//   CDSplitDoc   -> 文档类（存储原始图像和处理图像）
	//   CMainFrame   -> 主框架窗口（包含动态分割窗口）
	//   CDSplitView  -> 左视图（显示原始图像）
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CDSplitDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CDSplitView));
	AddDocTemplate(pDocTemplate);

	// 解析命令行参数（如打开文件、打印等）
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// 执行命令行指定的命令
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// 显示并更新主窗口
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg - "关于"对话框
//   显示应用程序版本、版权等基本信息

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// "关于"菜单命令响应：显示模态对话框
void CDSplitApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CDSplitApp commands
