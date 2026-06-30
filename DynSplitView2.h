// ====================================================================
// 文件名: DynSplitView2.h
// 描  述: 右视图类声明（分割窗口右列）
// 功  能: 位于分割窗口右侧，是整个应用的交互核心，负责:
//         - 显示处理后的图像结果 (OnDraw)
//         - 接收用户菜单命令并触发相应图像处理算法
//         - 提供调色板创建、内存清理等辅助功能
//
//   菜单命令映射:
//     ID_CALAREA    -> CalArea()      : 计算连通区域面积
//     ID_DELSMALL   -> OnDelSmall()   : 消除小区域
//     ID_FOLLOWLINE -> OnFollowline() : 边界跟踪 + 周长统计
//     ID_A          -> OnA()          : 显示连通区域标记
//     ID_FILESAVE   -> OnFilesave()   : 保存处理结果
// ====================================================================

#if !defined(AFX_DYNSPLITVIEW2_H__76E949DA_9CAD_11D1_907F_00A024782894__INCLUDED_)
#define AFX_DYNSPLITVIEW2_H__76E949DA_9CAD_11D1_907F_00A024782894__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include"DSplitDoc.h"
#include "ProcessDib.h"
#include "CDIB.h"

class CDynSplitView2 : public CView
{
protected:
	CDynSplitView2();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CDynSplitView2)

// Attributes
public:
	CDSplitDoc* GetDocument();

// Operations
public:
		CPalette *CreateBitmapPalette( CProcessDib* pBitmap);  // 创建位图调色板
 		CString filename;                 // 文件路径
		CProcessDib *CDibNew1;           // 指向文档中 CProcessDib 对象的指针（处理工作副本）
		CDib *CDib1;                     // 指向文档中 CDib 对象的指针（原始图像）
		CPalette hPalette;               // 调色板句柄
		int state2;                      // 视图状态标记（0=未就绪, 1=已就绪）
		void clearmem();                 // 重置处理状态：从原图恢复数据

		int data_gray;                   // 用户输入的灰度阈值

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDynSplitView2)
	protected:
	virtual void OnDraw(CDC* pDC);      // 绘制处理后的图像（使用 StretchDIBits）
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CDynSplitView2();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CDynSplitView2)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);   // 自定义背景擦除
	afx_msg void OnFilesave();             // 保存处理结果到 BMP 文件
	afx_msg void CalArea();                // 计算连通区域面积（二值化 + ConnectedRegions）
	afx_msg void OnDelSmall();             // 小区域消除（二值化 + ClearSMALL）
	afx_msg void OnFollowline();           // 边界跟踪 + 周长统计（二值化 + EliminIsoSpots + Borderline）
	afx_msg void OnA();                    // 显示连通区域标记编号
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DYNSPLITVIEW2_H__76E949DA_9CAD_11D1_907F_00A024782894__INCLUDED_)
