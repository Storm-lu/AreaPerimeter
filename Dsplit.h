// ====================================================================
// 文件名: Dsplit.h
// 项目名: DSplit (AreaPerimeter)
// 描  述: MFC SDI 应用程序主类声明文件
// 功  能: 基于 MFC 框架的图像处理应用 —— 对二值图像进行
//         连通区域分析（面积计算、周长统计、小区域消除）
// 架  构: CDSplitApp -> CMainFrame (动态分割窗口)
//                        ├── CDSplitView (左视图: 显示原图)
//                        └── CDynSplitView2 (右视图: 显示处理结果)
//                                └── CDSplitDoc (文档)
//                                     ├── CDib (原始图像数据)
//                                     └── CProcessDib (图像处理核心)
// ====================================================================

#if !defined(AFX_DSPLIT_H__76E949C7_9CAD_11D1_907F_00A024782894__INCLUDED_)
#define AFX_DSPLIT_H__76E949C7_9CAD_11D1_907F_00A024782894__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CDSplitApp: MFC 应用程序主类
//   负责应用程序的初始化、消息循环和资源管理
//   在 InitInstance() 中注册 SDI 文档模板（文档-框架-视图三件套）
//   详见 DSplit.cpp 中的实现
/////////////////////////////////////////////////////////////////////////////

class CDSplitApp : public CWinApp
{
public:
	CDSplitApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDSplitApp)
	public:
	virtual BOOL InitInstance();  // 应用初始化入口
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDSplitApp)
	afx_msg void OnAppAbout();    // "关于"对话框响应
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSPLIT_H__76E949C7_9CAD_11D1_907F_00A024782894__INCLUDED_)
