// ====================================================================
// 文件名: DSplitDoc.h
// 描  述: MFC 文档类声明
// 功  能: 作为数据容器，持有两份图像数据:
//         - CDib CDib:     原始 BMP 图像（只读，用于显示）
//         - CProcessDib CDibNew: 图像处理工作副本（所有算法操作的对象）
//   通过 OnFileopen() 响应用户打开文件，同时加载两份图像
// ====================================================================

#if !defined(AFX_DSPLITDOC_H__76E949CD_9CAD_11D1_907F_00A024782894__INCLUDED_)
#define AFX_DSPLITDOC_H__76E949CD_9CAD_11D1_907F_00A024782894__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "CDIB.h"
#include "ProcessDib.h"

class CDSplitDoc : public CDocument
{
protected: // create from serialization only
	CDSplitDoc();
	DECLARE_DYNCREATE(CDSplitDoc)

// Attributes
public:

// Operations
public:
   CDib CDib;              // 原始 BMP 图像数据（基类，只用于加载/显示）
   CProcessDib CDibNew;    // 图像处理工作副本（继承自CDib，所有算法操作对象）
   CString filename;       // 当前打开的 BMP 文件名
   int statedoc;           // 文档状态标记（0=未加载, 1=已加载）

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDSplitDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDSplitDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CDSplitDoc)
	afx_msg void OnFileopen();  // 文件打开命令响应
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSPLITDOC_H__76E949CD_9CAD_11D1_907F_00A024782894__INCLUDED_)
