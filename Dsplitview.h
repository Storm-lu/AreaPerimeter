// ====================================================================
// 文件名: DSplitView.h
// 描  述: 左视图类声明（分割窗口左列）
// 功  能: 位于分割窗口左侧，负责显示原始 BMP 图像
//   通过 OnOrigin() 获取文档中的文件名并触发 OnDraw 重绘
//   OnDraw() 使用 LoadImage + BitBlt 方式渲染原图
// ====================================================================

#if !defined(AFX_DSPLITVIEW_H__76E949CF_9CAD_11D1_907F_00A024782894__INCLUDED_)
#define AFX_DSPLITVIEW_H__76E949CF_9CAD_11D1_907F_00A024782894__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CDSplitView : public CView
{
protected: // create from serialization only
	CDSplitView();
	DECLARE_DYNCREATE(CDSplitView)

// Attributes
public:
	CDSplitDoc* GetDocument();  // 获取关联的文档对象

// Operations
public:
 	CString filename;   // 当前显示的文件路径
    int state1;         // 视图状态标记（0=未显示, 1=已显示）

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDSplitView)
	public:
	virtual void OnDraw(CDC* pDC);  // 绘制原始图像
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDSplitView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CDSplitView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);  // 自定义背景擦除
	afx_msg void OnOrigin();              // 显示原始图像命令
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in DSplitView.cpp
inline CDSplitDoc* CDSplitView::GetDocument()
   { return (CDSplitDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSPLITVIEW_H__76E949CF_9CAD_11D1_907F_00A024782894__INCLUDED_)
