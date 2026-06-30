// ====================================================================
// 文件名: DSplitView.cpp
// 描  述: 左视图实现（分割窗口左列）
//   负责显示原始 BMP 图像，使用 LoadImage + BitBlt 方式渲染
//   OnOrigin() 从文档获取文件名，设置 state1=1 触发 OnDraw 重绘
// ====================================================================

#include "stdafx.h"
#include "DSplit.h"

#include "DSplitDoc.h"
#include "DSplitView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
#define MAX_PATH 260 
static char THIS_FILE[MAX_PATH] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDSplitView

IMPLEMENT_DYNCREATE(CDSplitView, CView)

BEGIN_MESSAGE_MAP(CDSplitView, CView)
	//{{AFX_MSG_MAP(CDSplitView)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_YUANTU, OnOrigin)  // "原图"菜单命令绑定
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDSplitView construction/destruction

CDSplitView::CDSplitView()
{
	state1=0;  // 初始状态：未显示图像
}

CDSplitView::~CDSplitView()
{
}

BOOL CDSplitView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CDSplitView drawing

// OnDraw(): 绘制原始 BMP 图像
//   使用 LoadImage 从文件加载 BMP，通过 BitBlt 绘制到视图
void CDSplitView::OnDraw(CDC* pDC)
{      
	if(state1==1)  // 仅当已加载图像时才绘制
	{
		CBitmap m_bitmap;
		// 从文件加载位图（使用 DIB section 以提高性能）
		HBITMAP hBitmap=(HBITMAP)LoadImage(NULL,_T(filename),IMAGE_BITMAP,
			0,0,LR_CREATEDIBSECTION|LR_DEFAULTSIZE|LR_LOADFROMFILE);
        m_bitmap.Attach (hBitmap);
		CDC dcImage;
		if(!dcImage.CreateCompatibleDC (pDC))  // 创建内存 DC
			return;
		BITMAP bm;
		m_bitmap.GetBitmap (&bm);             // 获取位图尺寸
        dcImage.SelectObject (&m_bitmap);      // 选入位图
		pDC->BitBlt (0,0,bm.bmWidth ,bm.bmHeight ,&dcImage,0,0,SRCCOPY);  // 绘制到视图
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDSplitView printing

BOOL CDSplitView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return DoPreparePrinting(pInfo);
}

void CDSplitView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CDSplitView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

/////////////////////////////////////////////////////////////////////////////
// CDSplitView diagnostics

#ifdef _DEBUG
void CDSplitView::AssertValid() const
{
	CView::AssertValid();
}

void CDSplitView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CDSplitDoc* CDSplitView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDSplitDoc)));
	return (CDSplitDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDSplitView message handlers

// OnEraseBkgnd(): 自定义背景擦除 - 使用系统窗口颜色填充
BOOL CDSplitView::OnEraseBkgnd(CDC* pDC) 
{
	CRect rect;
	GetClientRect(&rect);
	pDC->FillSolidRect(&rect,::GetSysColor(COLOR_WINDOW));
	return TRUE;
}

// OnOrigin(): "原图"命令响应 - 从文档获取文件名并触发重绘
void CDSplitView::OnOrigin() 
{
	CDSplitDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
    filename=pDoc->filename;  // 获取文档中的文件路径
	state1=1;                 // 设置状态为已显示
	Invalidate();             // 触发 OnDraw 重绘
}
