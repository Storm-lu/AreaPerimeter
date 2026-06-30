// ====================================================================
// 文件名: MainFrm.h
// 描  述: 主框架窗口类声明
// 功  能: 管理应用程序主窗口，包含:
//         - 工具栏 (CToolBar) 和状态栏 (CStatusBar)
//         - 动态分割窗口 (CDynSplitterWnd): 1行2列布局
//           左列 = CDSplitView (原始图像)
//           右列 = CDynSplitView2 (处理结果)
// ====================================================================

#if !defined(AFX_MAINFRM_H__76E949CB_9CAD_11D1_907F_00A024782894__INCLUDED_)
#define AFX_MAINFRM_H__76E949CB_9CAD_11D1_907F_00A024782894__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "DynSplitterWnd.h"

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
protected:
	CDynSplitterWnd m_wndSplitter;  // 动态分割窗口（1行2列）
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);  // 创建分割窗口
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;  // 底部状态栏
	CToolBar    m_wndToolBar;    // 顶部工具栏

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);     // 窗口创建时初始化工具栏/状态栏
	afx_msg void OnDynSplit();                               // 切换动态分割模式
	afx_msg void OnUpdateDynSplit(CCmdUI* pCmdUI);           // 更新分割模式菜单状态
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__76E949CB_9CAD_11D1_907F_00A024782894__INCLUDED_)
