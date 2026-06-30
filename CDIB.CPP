// ====================================================================
// 文件名: CDIB.CPP
// 描  述: DIB (设备无关位图) 基础类的实现
//   实现 BMP 位图的文件读取、解析和保存功能
//   BMP 文件结构: BITMAPFILEHEADER(14B) + BITMAPINFOHEADER(40B) + 颜色表 + 像素数据
// ====================================================================

#include "stdafx.h"
#include "cdib.h"
#include "windowsx.h"
#include "math.h"
// 宏：计算 DIB 每行字节数（4 字节对齐），如 1位图: ((1+31)/32)*4 = 4
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4) 

CDib::CDib()
{
	size=0;
}

CDib::~CDib()
{
    GlobalFreePtr(m_pBitmapInfo);
}

// ====================================================================
// LoadFile(): 加载 BMP 文件并解析为 DIB 内存结构
//   流程: 读取文件头 -> 校验 BMP 标识(0x4d42) -> 读取 DIB 数据块
//        -> 定位信息头、颜色表、像素数据指针
// ====================================================================
void CDib::LoadFile(const char* dibFileName)
{
	strcpy(m_fileName,dibFileName);
    CFile dibFile(m_fileName, CFile::modeRead);
    dibFile.Read((void*)&bitmapFileHeader,sizeof(BITMAPFILEHEADER));
    if (bitmapFileHeader.bfType == 0x4d42)  // "BM" - 标准 BMP 文件标识
    {
        DWORD fileLength = dibFile.GetLength();    
		size = fileLength -sizeof(BITMAPFILEHEADER);  // DIB 数据大小 = 文件总长 - 文件头
		pDib =(BYTE*)GlobalAllocPtr(GMEM_MOVEABLE, size);  // 分配 DIB 内存
        dibFile.Read((void*)pDib, size);  // 读取全部 DIB 数据
        dibFile.Close();

        // 定位各数据块指针
        m_pBitmapInfo = (BITMAPINFO*) pDib;               // BITMAPINFO = 信息头 + 颜色表
        m_pBitmapInfoHeader = (BITMAPINFOHEADER*) pDib;   // 信息头 = DIB 开头 40 字节
        m_pRGB = (RGBQUAD*)(pDib +
			m_pBitmapInfoHeader->biSize);                  // 颜色表紧随信息头之后

        int m_numberOfColors = GetNumberOfColors();
        if (m_pBitmapInfoHeader->biClrUsed == 0)
            m_pBitmapInfoHeader->biClrUsed =
			m_numberOfColors;
        DWORD colorTableSize = m_numberOfColors *
            sizeof(RGBQUAD);                               // 颜色表占用字节数
        m_pData = pDib + m_pBitmapInfoHeader->biSize
            + colorTableSize;                              // 像素数据 = 信息头 + 颜色表
		m_pBitmapInfoHeader->biSizeImage = GetSize();
		
		if (m_pRGB == (RGBQUAD*)m_pData) // 颜色表指针与像素数据重合 → 无颜色表
			m_pRGB = NULL;
		
		m_valid = TRUE;
    }    
    else
    {
        m_valid = FALSE;
        AfxMessageBox("This isn't a bitmap file!");  // 非 BMP 文件，弹出错误提示
    }
}

BOOL CDib::IsValid()
{
    return m_valid;
}

char* CDib::GetFileName()
{
    return m_fileName;
}

UINT CDib::GetWidth()
{
    return (UINT) m_pBitmapInfoHeader->biWidth;
}

UINT CDib::GetHeight()
{
    return (UINT) m_pBitmapInfoHeader->biHeight;
}

// GetSize(): 获取图像像素数据大小
//   优先使用 biSizeImage（某些格式会预先计算），否则按 宽×高 估算
DWORD CDib::GetSize()
{
    if (m_pBitmapInfoHeader->biSizeImage != 0)
        return m_pBitmapInfoHeader->biSizeImage;
	else
    {
        DWORD height = (DWORD) GetHeight();
        DWORD width = (DWORD) GetWidth();
        return height * width;
    }
}

// GetNumberOfColors(): 获取颜色表中实际颜色数量
//   如果 biClrUsed 为 0 且位深 < 9（索引色），按位深推导：1位→2色, 4位→16色, 8位→256色
UINT CDib::GetNumberOfColors()
{
	int numberOfColors;
	
    if ((m_pBitmapInfoHeader->biClrUsed == 0) &&
		(m_pBitmapInfoHeader->biBitCount < 9))
	{
		switch (m_pBitmapInfoHeader->biBitCount)
		{
		case 1: numberOfColors = 2; break;
		case 4: numberOfColors = 16; break;
		case 8: numberOfColors = 256;
		}
	}
    else
		numberOfColors = (int) m_pBitmapInfoHeader->biClrUsed;
	
    return numberOfColors;
}

BYTE* CDib::GetData()
{
    return m_pData;
}

RGBQUAD* CDib::GetRGB()
{
    return m_pRGB;
}

BITMAPINFO* CDib::GetInfo()
{
    return m_pBitmapInfo;
}

// PaletteSize(): 计算调色板字节大小 = 颜色数 × sizeof(RGBTRIPLE)
WORD CDib::PaletteSize(LPBYTE lpDIB) 
{ 
    return (DIBNumColors(lpDIB) * sizeof(RGBTRIPLE)); 
} 

// DIBNumColors(): 根据 DIB 位深返回颜色表颜色数
//   仅处理 1/4/8 位索引色，24 位真彩色返回 0（无颜色表）
WORD CDib::DIBNumColors(LPBYTE lpDIB) 
{ 
    WORD wBitCount;  // DIB bit count 
    wBitCount = ((LPBITMAPCOREHEADER)lpDIB)->bcBitCount; 
    switch (wBitCount) 
    { 
	case 1: 
		return 2; 
	case 4: 
		return 16; 
	case 8: 
		return 256; 
	default: 
		return 0; 
    } 
} 

// SaveFile(): 将内存中的 DIB 数据写回 BMP 文件
void CDib::SaveFile(const CString filename)
{ 
	strcpy(m_fileName,filename);
    CFile dibFile(m_fileName, CFile::modeCreate|CFile::modeWrite);
    dibFile.Write((void*)&bitmapFileHeader,sizeof(BITMAPFILEHEADER));  // 写入文件头
    dibFile.Write((void*)pDib, size);  // 写入 DIB 数据块
    dibFile.Close();
	
}

// GetDibWidthBytes(): 计算每行像素占用的实际字节数（含 4 字节对齐填充）
//   1位图: (width+7)/8, 4位图: (width+1)/2, 8位图: width, 24位图: 3*width
//   结果需 4 字节对齐
DWORD CDib::GetDibWidthBytes()
{
	byBitCount=m_pBitmapInfoHeader->biBitCount;
	LONG nWidth=m_pBitmapInfoHeader->biWidth;
	
	dwWidthBytes = (DWORD)m_pBitmapInfoHeader->biWidth;	//8-bits 默认
	if(byBitCount == 1) dwWidthBytes = (nWidth + 7) / 8;      // 1 位: 每字节 8 像素
	else if(byBitCount == 4) dwWidthBytes = (nWidth + 1) / 2; // 4 位: 每字节 2 像素
	else if(byBitCount == 24) dwWidthBytes = 3 * nWidth ;     // 24 位: 每像素 3 字节 (BGR)
	
	// 确保 4 字节对齐（DIB 标准要求）
	while((dwWidthBytes & 3) != 0)dwWidthBytes++;
	
	return dwWidthBytes;
}
