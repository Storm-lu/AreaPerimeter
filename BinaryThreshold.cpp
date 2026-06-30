// ====================================================================
// 文件名: BinaryThreshold.cpp
// 描  述: 二值化阈值输入对话框实现
//   用户输入 0~255 的灰度阈值，通过 DDX/DDV 机制绑定到 m_gray
//   阈值越大，越少的像素被判定为白色（255），越多的像素被判定为黑色（0）
// ====================================================================

#include "stdafx.h"
#include "DSplit.h"
#include "BinaryThreshold.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CThreshold dialog

CThreshold::CThreshold(CWnd* pParent /*=NULL*/)
	: CDialog(CThreshold::IDD, pParent)
{
	//{{AFX_DATA_INIT(CThreshold)
	m_gray = 0;  // 默认阈值 = 0
	//}}AFX_DATA_INIT
}

// DoDataExchange(): 数据交换 - 将编辑框 IDC_EDITGray 与 m_gray 绑定
//   DDV_MinMaxInt 确保输入值在 [0, 255] 范围内
void CThreshold::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CThreshold)
	DDX_Text(pDX, IDC_EDITGray, m_gray);          // 绑定编辑框到 m_gray
	DDV_MinMaxInt(pDX, m_gray, 0, 255);            // 校验范围 [0, 255]
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CThreshold, CDialog)
	//{{AFX_MSG_MAP(CThreshold)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CThreshold message handlers
