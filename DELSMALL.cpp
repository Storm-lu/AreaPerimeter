// ====================================================================
// 文件名: DELSMALL.cpp
// 描  述: 小区域消除面积阈值输入对话框实现
//   用户输入面积阈值（像素数），通过 DDX 绑定到 m_delsmall
//   ClearSMALL() 将消除所有面积 < m_delsmall 的连通区域
// ====================================================================

#include "stdafx.h"
#include "DSplit.h"
#include "DELSMALL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DELSMALL dialog

DELSMALL::DELSMALL(CWnd* pParent /*=NULL*/)
	: CDialog(DELSMALL::IDD, pParent)
{
	//{{AFX_DATA_INIT(DELSMALL)
	m_delsmall = 0;  // 默认阈值 = 0
	//}}AFX_DATA_INIT
}

// DoDataExchange(): 数据交换 - 将编辑框 IDC_EDITDEL 与 m_delsmall 绑定
void DELSMALL::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DELSMALL)
	DDX_Text(pDX, IDC_EDITDEL, m_delsmall);  // 绑定编辑框到 m_delsmall
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(DELSMALL, CDialog)
	//{{AFX_MSG_MAP(DELSMALL)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DELSMALL message handlers
