// ====================================================================
// 文件名: DynSplitView2.cpp
// 描  述: 右视图实现（分割窗口右列）
//   所有图像处理操作的入口，响应用户菜单命令：
//     CalArea()      - 计算面积: 二值化 + ConnectedRegions (Two-Pass并查集)
//     OnDelSmall()   - 消除小区域: 二值化 + ClearSMALL
//     OnFollowline() - 周长统计: 二值化 + EliminIsoSpots + Borderline
//     OnA()          - 显示标记: 二值化 + ConnectedRegions
//     OnFilesave()   - 保存结果
//   处理流程: clearmem()恢复原图 → 二值化 → 算法处理 → 渲染显示
// ====================================================================

#include "stdafx.h"
#include "DSplit.h"
#include "DynSplitView2.h"
#include "ProcessDib.h"
#include "BinaryThreshold.h"
#include "DELSMALL.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
#define MAX_PATH 260
static char THIS_FILE[MAX_PATH] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDynSplitView2

IMPLEMENT_DYNCREATE(CDynSplitView2, CView)

CDynSplitView2::CDynSplitView2()
{
	state2=0;  // 初始状态：未就绪
}

// ====================================================================
// clearmem(): 重置处理状态
//   从文档中的原始图像 (CDib1) 拷贝数据到处理副本 (CDibNew1)
//   每次处理前调用，确保从干净的原始图像开始
// ====================================================================
void CDynSplitView2::clearmem()
{
	CDSplitDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc ->statedoc=0;        // 重置文档状态
    state2=1;                 // 标记视图已就绪
	CDibNew1=&pDoc->CDibNew;  // 指向处理工作副本
    CDib1=&pDoc->CDib;        // 指向原始图像
    long int  size=CDib1->GetHeight()*CDib1->GetDibWidthBytes();
    memcpy(CDibNew1->m_pData,CDib1->m_pData,size);  // 从原图恢复数据
}

// ====================================================================
// CreateBitmapPalette(): 从 DIB 颜色表创建 Windows 调色板
//   用于正确显示索引色（1/4/8位）BMP 图像的颜色
// ====================================================================
CPalette * CDynSplitView2::CreateBitmapPalette(CProcessDib * pBitmap)
{
		struct
		{
			WORD Version;                // 调色板版本（0x300 = Windows 3.0）
			WORD NumberOfEntries;        // 颜色条目数
			PALETTEENTRY aEntries[256];  // 最多 256 色
		} palette = { 0x300, 256 };
		
		LPRGBQUAD pRGBTable = pBitmap->GetRGB();          // 获取颜色表
		UINT numberOfColors = pBitmap->GetNumberOfColors(); // 获取颜色数
		
		// 将 DIB 的 RGBQUAD 转换为 Windows PALETTEENTRY
		for(UINT x=0; x<numberOfColors; ++x)
		{
			palette.aEntries[x].peRed =
				pRGBTable[x].rgbRed;
			palette.aEntries[x].peGreen =
				pRGBTable[x].rgbGreen;
			palette.aEntries[x].peBlue =
				pRGBTable[x].rgbBlue;
			palette.aEntries[x].peFlags = 0;
		}

		// 释放旧调色板，避免资源泄漏
		if (hPalette.GetSafeHandle() != NULL) {
			hPalette.DeleteObject();
		}

		hPalette.CreatePalette((LPLOGPALETTE)&palette);
		return &hPalette; // 返回成员变量的地址（生命周期由类管理）
}

CDynSplitView2::~CDynSplitView2()
{
}

CDSplitDoc* CDynSplitView2::GetDocument()
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDSplitDoc)));
	return (CDSplitDoc*)m_pDocument;
}

BEGIN_MESSAGE_MAP(CDynSplitView2, CView)
	//{{AFX_MSG_MAP(CDynSplitView2)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_FILESAVE, OnFilesave)       // 保存处理结果
	ON_COMMAND(ID_CALAREA, CalArea)           // 计算连通区域面积
	ON_COMMAND(ID_DELSMALL, OnDelSmall)       // 消除小区域
	ON_COMMAND(ID_FOLLOWLINE, OnFollowline)   // 边界跟踪 + 周长统计
	ON_COMMAND(ID_A, OnA)                     // 显示连通区域标记
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDynSplitView2 drawing

// OnDraw(): 使用 StretchDIBits 渲染处理后的图像
//   若图像有颜色表（索引色），则创建并应用调色板
void CDynSplitView2::OnDraw(CDC* pDC)
{	
	CDSplitDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(!pDoc ->statedoc&&state2==1)  // 文档未标记 + 视图已就绪 → 绘制
	{
 	    int m_scale=1;
        BYTE* pBitmapData = CDibNew1->GetData();
        LPBITMAPINFO pBitmapInfo = CDibNew1->GetInfo();
        int bitmapHeight = CDibNew1->GetHeight();
        int bitmapWidth = CDibNew1->GetWidth();
		int scaledWidth = (int)(bitmapWidth * m_scale);
		int scaledHeight = (int)(bitmapHeight * m_scale);
		if (CDibNew1->GetRGB()) // 有颜色表（索引色图像）→ 需创建调色板
		{
			CPalette * hPalette=CreateBitmapPalette(CDibNew1);
            CPalette * hOldPalette =
                pDC->SelectPalette(hPalette, true);
            pDC->RealizePalette();            // 实现调色板
			::StretchDIBits(pDC->GetSafeHdc(),0, 0, scaledWidth, scaledHeight,
               0, 0, bitmapWidth, bitmapHeight,
				pBitmapData, pBitmapInfo,
				DIB_RGB_COLORS, SRCCOPY);     // 使用 DIB 颜色渲染
            pDC->SelectPalette(hOldPalette, true);
            ::DeleteObject(hPalette);
		}
		else  // 无颜色表（如24位真彩色图）→ 直接渲染
		{
			::StretchDIBits(pDC->GetSafeHdc(),0, 0, scaledWidth, scaledHeight,
				0, 0, bitmapWidth, bitmapHeight,
				pBitmapData, pBitmapInfo,
				DIB_RGB_COLORS, SRCCOPY);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CDynSplitView2 diagnostics

#ifdef _DEBUG
void CDynSplitView2::AssertValid() const
{
	CView::AssertValid();
}

void CDynSplitView2::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDynSplitView2 message handlers

// OnEraseBkgnd(): 自定义背景擦除
BOOL CDynSplitView2::OnEraseBkgnd(CDC* pDC) 
{
	CRect rect;
	GetClientRect(&rect);
	pDC->FillSolidRect(&rect,::GetSysColor(COLOR_WINDOW));  // 使用系统窗口色
	return TRUE;
}

// ====================================================================
// OnFilesave(): 保存处理结果到 BMP 文件
//   弹出保存对话框，将 CDibNew1（处理后的图像）写入文件
// ====================================================================
void CDynSplitView2::OnFilesave() 
{
	CFileDialog dlg(FALSE,_T("BMP"),_T("*.BMP"),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,_T("位图文件(*.BMP)|*.BMP|"));	
    if(IDOK==dlg.DoModal())
	CString  filename;
    filename.Format ("%s",dlg.GetPathName() );    
    CDibNew1->SaveFile(filename);  // 保存处理后的图像
	state2=1;
	Invalidate();  // 刷新视图
}

// ====================================================================
// CalArea(): 计算连通区域面积
//   流程: clearmem() 恢复原图 → 弹出阈值对话框
//         → 24位彩色先备份+转灰度 → 二值化 → ConnectedRegions()
//         → 24位彩色恢复原图叠加显示 → 在图上标注各区域面积
// ====================================================================
void CDynSplitView2::CalArea() 
{
	clearmem();  // 从原图恢复数据
    
    LPBYTE temp=NULL;
	int i,j;
	int wide,height;

	// 弹出二值化阈值对话框
	CThreshold Dlg;
	Dlg.DoModal();
	data_gray=Dlg.m_gray;

	// 24位彩色图预处理：备份原图 + 转灰度
	if(CDibNew1->m_pBitmapInfoHeader->biBitCount==24)
	{
		 wide=CDibNew1->GetDibWidthBytes();
		 height=CDibNew1->GetHeight();
	     temp = new BYTE[wide*height];
	     memset(temp, (BYTE)255, wide * height);
		 CDibNew1->SaveOrigin(temp);   // 备份原图
         CDibNew1->MakeGray();         // 转为灰度
	}

	CDibNew1->BinaryOperation(data_gray);     // 二值化
    CDibNew1->ConnectedRegions();             // Two-Pass并查集连通区域分析 + 面积统计
    
	// 24位彩色图后处理：将备份的原图叠加回来
	if(CDibNew1->m_pBitmapInfoHeader->biBitCount==24)
	{
		LPBYTE  lpSrc,lpDst,temp2;
		lpSrc=CDibNew1->GetData();
		lpDst=temp;
		temp2=lpSrc;
		for(j=0;j<height;j++)
			for(i=0;i<wide;i++)
			{    
				*lpSrc=*lpDst+*lpSrc;      // 叠加原图
				if(*lpSrc>255) *lpSrc=255;  // 防止溢出
				lpSrc++;
				lpDst++;
			}
			lpSrc=temp2;
	}

	delete[] temp;

	// 渲染处理结果
	CClientDC dc(this);   
	CDSplitDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(!pDoc ->statedoc&&state2==1)
	{
 	    int m_scale=1;
        BYTE* pBitmapData = CDibNew1->GetData();
        LPBITMAPINFO pBitmapInfo = CDibNew1->GetInfo();
        int bitmapHeight = CDibNew1->GetHeight();
        int bitmapWidth = CDibNew1->GetWidth();
		int scaledWidth = (int)(bitmapWidth * m_scale);
		int scaledHeight = (int)(bitmapHeight * m_scale);
		if (CDibNew1->GetRGB()) // 有颜色表
		{
			CPalette * hPalette=CreateBitmapPalette(CDibNew1);
            CPalette * hOldPalette =
                dc.SelectPalette(hPalette, true);
            dc.RealizePalette();
			::StretchDIBits(dc.GetSafeHdc(),0, 0, scaledWidth, scaledHeight,
               0, 0, bitmapWidth, bitmapHeight,
				pBitmapData, pBitmapInfo,
				DIB_RGB_COLORS, SRCCOPY);
            dc.SelectPalette(hOldPalette, true);
            ::DeleteObject(hPalette);
		}
		else
		{
			::StretchDIBits(dc.GetSafeHdc(),0, 0, scaledWidth, scaledHeight,
				0, 0, bitmapWidth, bitmapHeight,
				pBitmapData, pBitmapInfo,
				DIB_RGB_COLORS, SRCCOPY);
		}
	}

	// 在图像上标注各连通区域的面积
	dc.SetTextColor(100);
	CString ss_Area[255];
	for( i=0;i<255;i++)
	{
		if(CDibNew1->pppp[i].pp_area!=0)
		{
			ss_Area[i].Format("%d",CDibNew1->pppp[i].pp_area);
			CSize textSize = dc.GetTextExtent(ss_Area[i]);
			int x = CDibNew1->pppp[i].pp_x - textSize.cx / 2;
			int y = CDibNew1->pppp[i].pp_y - textSize.cy / 2;
			if (x < 0) x = 0;
			if (y < 0) y = 0;
			dc.TextOut(x, y, ss_Area[i]);
		}
	}
}

// ====================================================================
// OnDelSmall(): 小区域消除
//   流程: clearmem() → 阈值对话框 → 24位预处理 → 二值化
//         → 弹出面积阈值对话框 → ClearSMALL() 消除小区域 → 刷新视图
// ====================================================================
void CDynSplitView2::OnDelSmall() 
{
	clearmem();  // 从原图恢复数据

	LPBYTE temp;
	int i,j;
	int wide,height;

	// 弹出二值化阈值对话框
	CThreshold Dlg;
	Dlg.DoModal();
	data_gray=Dlg.m_gray;

	// 24位彩色图预处理
	if(CDibNew1->m_pBitmapInfoHeader->biBitCount==24)
	{
		 wide=CDibNew1->GetDibWidthBytes();
		 height=CDibNew1->GetHeight();
	     temp = new BYTE[wide*height];
	     memset(temp, (BYTE)255, wide * height);
		 CDibNew1->SaveOrigin(temp);   // 备份原图
         CDibNew1->MakeGray();         // 转灰度
	}

	CDibNew1->BinaryOperation(data_gray);  // 二值化

	// 24位彩色图后处理：叠加原图
	if(CDibNew1->m_pBitmapInfoHeader->biBitCount==24)
	{
		LPBYTE  lpSrc,lpDst,temp2;
		lpSrc=CDibNew1->GetData();
		lpDst=temp;
		temp2=lpSrc;
		for(j=0;j<height;j++)
			for(i=0;i<wide;i++)
			{    
				*lpSrc=*lpDst+*lpSrc;
				if(*lpSrc>255) *lpSrc=255;
				lpSrc++;
				lpDst++;
			}
			lpSrc=temp2;
	}

	// 弹出面积阈值对话框并执行小区域消除
	int m_value;
	DELSMALL  Dlg1;
	Dlg1.DoModal();
	m_value=Dlg1.m_delsmall;
	CDibNew1->ClearSMALL(m_value);  // 消除面积 < m_value 的区域
 	Invalidate();  // 刷新视图
}

// ====================================================================
// OnFollowline(): 边界跟踪 + 周长统计
//   流程: clearmem() → 阈值对话框 → 24位预处理 → 二值化
//         → EliminIsoSpots() 消除孤立黑点
//         → Borderline() 边界提取 + 周长统计
//         → 渲染结果并在图上标注各区域周长
// ====================================================================
void CDynSplitView2::OnFollowline() 
{
	clearmem();  // 从原图恢复数据
	LPBYTE temp;
	int i,j;
	int wide,height;

	// 弹出二值化阈值对话框
	CThreshold Dlg;
	Dlg.DoModal();
	data_gray=Dlg.m_gray;

	// 24位彩色图预处理
	if(CDibNew1->m_pBitmapInfoHeader->biBitCount==24)
	{
		 wide=CDibNew1->GetDibWidthBytes();
		 height=CDibNew1->GetHeight();
	     temp = new BYTE[wide*height];
	     memset(temp, (BYTE)255, wide * height);
		 CDibNew1->SaveOrigin(temp);   // 备份原图
		 CDibNew1->MakeGray();         // 转灰度
	}

	CDibNew1->BinaryOperation(data_gray);  // 二值化
	CDibNew1->EliminIsoSpots();            // 消除孤立黑点（减少噪声干扰）
	CDibNew1->Borderline();                // 边界提取 + 周长统计（弹出LINEDLG对话框）

	// 渲染处理结果
	CClientDC dc(this);   
	CDSplitDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(!pDoc ->statedoc&&state2==1)
	{
		int m_scale=1;
        BYTE* pBitmapData = CDibNew1->GetData();
        LPBITMAPINFO pBitmapInfo = CDibNew1->GetInfo();
        int bitmapHeight = CDibNew1->GetHeight();
        int bitmapWidth = CDibNew1->GetWidth();
		int scaledWidth = (int)(bitmapWidth * m_scale);
		int scaledHeight = (int)(bitmapHeight * m_scale);
		if (CDibNew1->GetRGB())
		{
			CPalette * hPalette=CreateBitmapPalette(CDibNew1);
            CPalette * hOldPalette =
                dc.SelectPalette(hPalette, true);
            dc.RealizePalette();
			::StretchDIBits(dc.GetSafeHdc(),0, 0, scaledWidth, scaledHeight,
				0, 0, bitmapWidth, bitmapHeight,
				pBitmapData, pBitmapInfo,
				DIB_RGB_COLORS, SRCCOPY);
            dc.SelectPalette(hOldPalette, true);
            ::DeleteObject(hPalette);
		}
		else
		{
			::StretchDIBits(dc.GetSafeHdc(),0, 0, scaledWidth, scaledHeight,
				0, 0, bitmapWidth, bitmapHeight,
				pBitmapData, pBitmapInfo,
				DIB_RGB_COLORS, SRCCOPY);
		}
	}

	// 在图像上标注各连通区域的周长
	dc.SetTextColor(100);
	CString ss_line[255];
	for( i=0;i<255;i++)
	{
		if(CDibNew1->pppp[i].pp_line!=0)
		{
			ss_line[i].Format("%d",CDibNew1->pppp[i].pp_line);
			CSize textSize = dc.GetTextExtent(ss_line[i]);
			int x = CDibNew1->pppp[i].pp_x - textSize.cx / 2;
			int y = CDibNew1->pppp[i].pp_y - textSize.cy / 2;
			if (x < 0) x = 0;
			if (y < 0) y = 0;
			dc.TextOut(x, y, ss_line[i]);
		}
	}
}

// ====================================================================
// OnA(): 显示连通区域标记编号
//   与 CalArea() 流程类似，但在图像上标注的是区域编号而非面积
//   使用 ConnectedRegions() 进行 Two-Pass 并查集标记
// ====================================================================
void CDynSplitView2::OnA() 
{
	clearmem();  // 从原图恢复数据
	LPBYTE temp=NULL;
	int i,j;
	int wide,height;

	// 弹出二值化阈值对话框
	CThreshold Dlg;
	Dlg.DoModal();
	data_gray=Dlg.m_gray;

	// 24位彩色图预处理
	if(CDibNew1->m_pBitmapInfoHeader->biBitCount==24)
	{
         wide=CDibNew1->GetDibWidthBytes();
		 height=CDibNew1->GetHeight();
	     temp = new BYTE[wide*height];
	     memset(temp, (BYTE)255, wide * height);
		 CDibNew1->SaveOrigin(temp);   // 备份原图
	     CDibNew1->MakeGray();         // 转灰度
	}

	CDibNew1->BinaryOperation(data_gray);     // 二值化
    CDibNew1->ConnectedRegions();             // Two-Pass并查集连通区域分析

    // 24位彩色图后处理：叠加原图
    if(CDibNew1->m_pBitmapInfoHeader->biBitCount==24)
	{
		LPBYTE  lpSrc,lpDst,temp2;
		lpSrc=CDibNew1->GetData();
		lpDst=temp;
		temp2=lpSrc;
		for(j=0;j<height;j++)
			for(i=0;i<wide;i++)
			{    
				*lpSrc=*lpDst+*lpSrc;
				if(*lpSrc>255) *lpSrc=255;
				lpSrc++;
				lpDst++;
			}
			lpSrc=temp2;
	}
	delete[] temp;

	// 渲染处理结果
	CClientDC dc(this);   
	CDSplitDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(!pDoc ->statedoc&&state2==1)
	{
 	    int m_scale=1;
        BYTE* pBitmapData = CDibNew1->GetData();
        LPBITMAPINFO pBitmapInfo = CDibNew1->GetInfo();
        int bitmapHeight = CDibNew1->GetHeight();
        int bitmapWidth = CDibNew1->GetWidth();
		int scaledWidth = (int)(bitmapWidth * m_scale);
		int scaledHeight = (int)(bitmapHeight * m_scale);
		if (CDibNew1->GetRGB())
		{
			CPalette * hPalette=CreateBitmapPalette(CDibNew1);
            CPalette * hOldPalette =
                dc.SelectPalette(hPalette, true);
            dc.RealizePalette();
			::StretchDIBits(dc.GetSafeHdc(),0, 0, scaledWidth, scaledHeight,
				0, 0, bitmapWidth, bitmapHeight,
				pBitmapData, pBitmapInfo,
				DIB_RGB_COLORS, SRCCOPY);
            dc.SelectPalette(hOldPalette, true);
            ::DeleteObject(hPalette);
		}
		else
		{
			::StretchDIBits(dc.GetSafeHdc(),0, 0, scaledWidth, scaledHeight,
				0, 0, bitmapWidth, bitmapHeight,
				pBitmapData, pBitmapInfo,
				DIB_RGB_COLORS, SRCCOPY);
		}
	}
	
	// 在图像上标注各连通区域的编号
	dc.SetTextColor(100);
	CString ss_Area[255];
	for(i=0;i<255;i++)
	{
		if(CDibNew1->pppp[i].pp_area!=0)
		{
			ss_Area[i].Format("%d",CDibNew1->pppp[i].pp_number);  // 显示编号
			CSize textSize = dc.GetTextExtent(ss_Area[i]);
			int x = CDibNew1->pppp[i].pp_x - textSize.cx / 2;
			int y = CDibNew1->pppp[i].pp_y - textSize.cy / 2;
			if (x < 0) x = 0;
			if (y < 0) y = 0;
			dc.TextOut(x, y, ss_Area[i]);
		}
	}
}
