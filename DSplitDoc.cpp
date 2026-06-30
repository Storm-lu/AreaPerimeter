// ====================================================================
// 文件名: DSplitDoc.cpp
// 描  述: MFC 文档类实现
//   核心职责:
//     1. OnFileopen() 响应用户打开 BMP 文件
//     2. 同时加载两份图像数据: CDib (原始) 和 CProcessDib (处理副本)
//     3. 设置 statedoc=1 标记文档已加载，触发视图刷新
// ====================================================================

#include "stdafx.h"
#include "DSplit.h"

#include "DSplitDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDSplitDoc

IMPLEMENT_DYNCREATE(CDSplitDoc, CDocument)

BEGIN_MESSAGE_MAP(CDSplitDoc, CDocument)
//{{AFX_MSG_MAP(CDSplitDoc)
ON_COMMAND(ID_FILEOPEN, OnFileopen)  // 文件打开命令绑定
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDSplitDoc construction/destruction

CDSplitDoc::CDSplitDoc()
{
	statedoc=0;  // 初始状态：未加载图像
}

CDSplitDoc::~CDSplitDoc()
{
}

BOOL CDSplitDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	
	// (SDI documents will reuse this document)
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDSplitDoc serialization

void CDSplitDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDSplitDoc diagnostics

#ifdef _DEBUG
void CDSplitDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDSplitDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDSplitDoc commands

// ====================================================================
// OnFileopen(): 文件打开命令响应
//   弹出 BMP 文件选择对话框，同时将文件加载到 CDib（原始）和
//   CProcessDib（处理副本）两个对象中，设置 statedoc=1 触发视图刷新
// ====================================================================
void CDSplitDoc::OnFileopen() 
{
	CFileDialog dlg(TRUE,_T("BMP"),_T("*.BMP"),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,_T("位图文件(*.BMP)|*.BMP|"));	
    if(IDOK==dlg.DoModal ())
		filename.Format ("%s",dlg.GetPathName() );    
	CDib.LoadFile(filename);      // 加载原始图像（用于左视图显示）
    CDibNew.LoadFile(filename);   // 加载处理副本（用于右视图所有算法操作）
	statedoc=1;                   // 标记文档已加载，触发视图 OnDraw 刷新
}
