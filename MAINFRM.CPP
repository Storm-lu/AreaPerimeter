// ====================================================================
// 文件名: MainFrm.cpp
// 描  述: 主框架窗口实现
//   核心功能: 创建静态分割窗口（1行2列）
//     左列 (0,0): 使用默认视图类 CDSplitView，显示原始 BMP 图像
//     右列 (0,1): 使用 CDynSplitView2，显示处理后的结果
//   并管理工具栏和状态栏的创建与停靠
// ====================================================================

#include "stdafx.h"
#include "DSplit.h"

#include "MainFrm.h"
#include "DynSplitView2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_DYNSPLIT, OnDynSplit)                     // 动态分割模式切换
	ON_UPDATE_COMMAND_UI(ID_DYNSPLIT, OnUpdateDynSplit)     // 更新菜单复选状态
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// 状态栏指示器配置: 通用信息栏 + Caps/Num/Scroll 锁状态
static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,      // 大写锁定
	ID_INDICATOR_NUM,       // 数字锁定
	ID_INDICATOR_SCRL,      // 滚动锁定
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

// OnCreate(): 窗口创建时初始化工具栏和状态栏
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// 创建工具栏
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	// 创建状态栏
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// 设置工具栏样式: 工具提示 + 浮动提示 + 可调大小
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// 启用工具栏停靠功能
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

// ====================================================================
// OnCreateClient(): 创建分割窗口
//   创建静态分割窗口（1行2列），左侧显示原图，右侧显示处理结果
//   左窗格使用文档模板的默认视图类 (CDSplitView)
//   右窗格使用 CDynSplitView2 类，负责所有图像处理操作
// ====================================================================
BOOL CMainFrame::OnCreateClient( LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	// 创建静态分割窗口: 1 行, 2 列
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
	{
		TRACE0("Failed to Splitter window\n");
		return FALSE;
	}

	// 左窗格 (0,0): 默认视图 - 显示原始图像
	if (!m_wndSplitter.CreateView(0, 0,
		pContext->m_pNewViewClass, CSize(350, 150), pContext))
	{
		TRACE0("Failed to create first pane\n");
		return FALSE;
	}

	// 右窗格 (0,1): CDynSplitView2 - 显示处理结果，接收用户操作
	if (!m_wndSplitter.CreateView(0, 1,
			RUNTIME_CLASS(CDynSplitView2), CSize(0, 0), pContext))
	{
		TRACE0("Failed to create second pane\n");
		return FALSE;
	}

	// 激活右视图（处理视图）
	SetActiveView((CView*)m_wndSplitter.GetPane(0,1));

	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	return CFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

// OnDynSplit(): 切换分割窗口的动态/静态模式
void CMainFrame::OnDynSplit() 
{
	m_wndSplitter.SetDynamic(!m_wndSplitter.IsDynamic());
}

// OnUpdateDynSplit(): 根据当前模式更新菜单项的复选状态
void CMainFrame::OnUpdateDynSplit(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_wndSplitter.IsDynamic());
}
