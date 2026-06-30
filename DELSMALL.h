// ====================================================================
// 文件名: DELSMALL.h
// 描  述: 小区域消除面积阈值输入对话框类声明
// 功  能: 弹出对话框让用户输入面积阈值（像素数）
//   成员 m_delsmall 绑定到编辑框 IDC_EDITDEL
//   连通区域面积 < m_delsmall 的区域将被消除（置为白色）
// ====================================================================

#if !defined(AFX_DELSMALL_H__D42AF56A_17EC_4054_A131_CD4A30604464__INCLUDED_)
#define AFX_DELSMALL_H__D42AF56A_17EC_4054_A131_CD4A30604464__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// DELSMALL dialog - 小区域消除面积阈值设置对话框

class DELSMALL : public CDialog
{
// Construction
public:
	DELSMALL(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DELSMALL)
	enum { IDD = IDD_DELSMALL_DLG };  // 对话框资源 ID
	int		m_delsmall;               // 面积阈值（像素数），绑定到 IDC_EDITDEL 编辑框
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DELSMALL)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DELSMALL)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_DELSMALL_H__D42AF56A_17EC_4054_A131_CD4A30604464__INCLUDED_)
