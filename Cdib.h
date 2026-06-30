// ====================================================================
// 文件名: Cdib.h
// 描  述: DIB (设备无关位图) 基础类声明
// 功  能: 封装 BMP 文件的读取、保存、信息查询等基本操作
//   作为整个项目图像处理管线的数据层基类，CProcessDib 继承自它
// ====================================================================

#ifndef __CDIB_H
#define __CDIB_H

class CDib : public CObject
{
public:
    // ===== 图像属性 =====
    RGBQUAD* m_pRGB;               // 调色板/颜色表指针 (NULL 表示无颜色表)
    BYTE *m_pData;                 // 像素数据区指针 (指向实际像素起始位置)
    UINT m_numberOfColors;         // 颜色表中颜色数量
    BOOL m_valid;                  // 文件是否有效 (BMP 格式校验结果)
    BITMAPFILEHEADER bitmapFileHeader;  // BMP 文件头 (14 字节，含文件类型标识 0x4d42)

    // ===== DIB 信息结构 =====
    BITMAPINFOHEADER* m_pBitmapInfoHeader; // BITMAPINFOHEADER 指针 (40 字节，含宽高、位深等)
    BITMAPINFO* m_pBitmapInfo;            // BITMAPINFO 指针 (= pDib，信息头+颜色表+像素)
    BYTE* pDib;                  // DIB 数据块起始指针 (不含 BITMAPFILEHEADER)

    DWORD size;                  // DIB 数据块大小 (文件总长 - BITMAPFILEHEADER 大小)
    int byBitCount;              // 每像素位数 (1/4/8/24)
    DWORD dwWidthBytes;          // 每行字节数 (4 字节对齐)

public:
    CDib();
    ~CDib();

    char m_fileName[256];        // 当前加载的 BMP 文件名

    // ===== 基本查询接口 =====
    char* GetFileName();         // 获取文件名
    BOOL IsValid();              // 检查文件是否为有效 BMP
    DWORD GetSize();             // 获取图像数据大小 (字节)
    UINT GetWidth();             // 获取图像宽度 (像素)
    UINT GetHeight();            // 获取图像高度 (像素)
    UINT GetNumberOfColors();    // 获取颜色表颜色数
    RGBQUAD* GetRGB();           // 获取颜色表指针
    BYTE* GetData();             // 获取像素数据指针
    BITMAPINFO* GetInfo();       // 获取 BITMAPINFO 指针

    // ===== DIB 辅助方法 =====
    WORD PaletteSize(LPBYTE lpDIB);    // 计算调色板大小
    WORD DIBNumColors(LPBYTE lpDIB);   // 根据位深计算颜色数
    void SaveFile(const CString filename);  // 保存 DIB 数据到 BMP 文件

public:
    DWORD GetDibWidthBytes();    // 获取每行字节数 (含 4 字节对齐填充)
    void LoadFile(const char* dibFileName);  // 从文件加载 BMP 图像
};

#endif
