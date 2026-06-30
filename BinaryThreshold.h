// ====================================================================
// 文件名: BinaryThreshold.h
// 描  述: 二值化阈值输入对话框类声明
// 功  能: 弹出对话框让用户输入灰度阈值 (0-255)
//   成员 m_gray 绑定到编辑框 IDC_EDITGray，取值范围 [0, 255]
//   在所有图像处理操作前都会先弹出此对话框
// ====================================================================

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CThreshold dialog - 二值化阈值设置对话框

class CThreshold : public CDialog
{
// Construction
public:
	CThreshold(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CThreshold)
	enum { IDD = IDD_DIALOG2 };   // 对话框资源 ID
	int		m_gray;               // 灰度阈值 (0~255)，绑定到 IDC_EDITGray 编辑框
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CThreshold)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CThreshold)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
