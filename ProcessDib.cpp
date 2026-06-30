// ====================================================================
// 文件名: ProcessDib.cpp
// 描  述: 图像处理核心算法实现
//   本文件包含所有图像处理算法的具体实现，是整个项目的核心模块。
//
// 
//     BinaryOperation()    - 固定阈值二值化 (灰度/24位彩色)
//     EliminIsoSpots()     - 8邻域孤立黑点消除
//     EliminIsoSpotsWhite()- 8邻域孤立白点消除
//     LabelNum()           - DFS栈式连通区域标记 (4连通)
//     ConnectedRegions()   - Two-Pass并查集连通区域标记 (8连通) + 面积统计
//     ClearSMALL()         - 基于面积阈值的小区域消除
//     Borderline()         - 边界提取 + 周长统计
//     MakeGray()           - 24位彩色转灰度 (加权公式)
//     SaveOrigin()         - 备份原始图像数据
// ====================================================================

#include "stdafx.h"
#include "DSplit.h"
#include "ProcessDib.h"
#include "SquareDlg.h"
#include "LINEDLG.h"
#include <vector>
#ifdef _DEBUG
#undef THIS_FILE
#define MAX_PATH 260
static char THIS_FILE[MAX_PATH]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProcessDib::CProcessDib()
{
	x_sign=0;   // 连通区域计数清零
	m_temp = 0;   // 临时变量初始化
	x_temp=0;
	y_temp=0;
    p_temp=0;   // 临时标签图指针初始化
	stop=0;     // 溢出标志清零
}

CProcessDib::~CProcessDib()
{

}

namespace
{
static void ResolveLabelAnchor(const BYTE* labelMap, int width, int height, int label, int preferredX, int preferredY, int& outX, int& outY)
{
	outX = preferredX;
	outY = preferredY;

	if (labelMap == NULL || width <= 0 || height <= 0)
	{
		return;
	}

	if (preferredX < 0) preferredX = 0;
	if (preferredX >= width) preferredX = width - 1;
	if (preferredY < 0) preferredY = 0;
	if (preferredY >= height) preferredY = height - 1;

	int bestX = -1;
	int bestY = -1;
	int bestDist = 0x7fffffff;

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			if (labelMap[y * width + x] != (BYTE)label)
			{
				continue;
			}

			int dx = x - preferredX;
			int dy = y - preferredY;
			int dist = dx * dx + dy * dy;
			if (dist < bestDist)
			{
				bestDist = dist;
				bestX = x;
				bestY = y;
			}
		}
	}

	if (bestX >= 0)
	{
		outX = bestX;
		outY = bestY;
	}
}

static void FillClosedRegionsForArea(BYTE* data, int width, int height, int dibWidth, bool is8bit)
{
	if (data == NULL || width <= 0 || height <= 0)
	{
		return;
	}

	std::vector<BYTE> exterior(width * height, 0);
	std::vector<int> stack;
	stack.reserve(width * height / 2 + 1);

	auto isWhite = [&](int x, int y) -> bool
	{
		if (x < 0 || x >= width || y < 0 || y >= height)
		{
			return false;
		}

		int idx = y * dibWidth + x * (is8bit ? 1 : 3);
		if (is8bit)
		{
			return data[idx] == 255;
		}
		return data[idx] == 255 && data[idx + 1] == 255 && data[idx + 2] == 255;
	};

	auto pushIfWhite = [&](int x, int y)
	{
		if (x < 0 || x >= width || y < 0 || y >= height)
		{
			return;
		}

		int idx = y * width + x;
		if (exterior[idx] == 0 && isWhite(x, y))
		{
			exterior[idx] = 1;
			stack.push_back(idx);
		}
	};

	for (int x = 0; x < width; ++x)
	{
		pushIfWhite(x, 0);
		if (height > 1)
		{
			pushIfWhite(x, height - 1);
		}
	}
	for (int y = 1; y < height - 1; ++y)
	{
		pushIfWhite(0, y);
		if (width > 1)
		{
			pushIfWhite(width - 1, y);
		}
	}

	while (!stack.empty())
	{
		int idx = stack.back();
		stack.pop_back();
		int x = idx % width;
		int y = idx / width;

		pushIfWhite(x - 1, y);
		pushIfWhite(x + 1, y);
		pushIfWhite(x, y - 1);
		pushIfWhite(x, y + 1);
	}

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			int mapIdx = y * width + x;
			if (exterior[mapIdx] != 0)
			{
				continue;
			}

			int idx = y * dibWidth + x * (is8bit ? 1 : 3);
			if (is8bit)
			{
				if (data[idx] == 255) data[idx] = 0;
			}
			else
			{
				if (data[idx] == 255 && data[idx + 1] == 255 && data[idx + 2] == 255)
				{
					data[idx] = data[idx + 1] = data[idx + 2] = 0;
				}
			}
		}
	}
}
}

// ====================================================================
// BinaryOperation(): 固定阈值二值化
//   灰度图: 逐像素与阈值比较，>=阈值→白色(255)，<阈值→黑色(0)
//   24位彩色: 先用加权公式 gray=(30R+59G+11B)/100 转为灰度，再二值化
//   加权系数基于人眼对绿光敏感、对蓝光不敏感的原理
// ====================================================================
void CProcessDib::BinaryOperation(int threshold)
{
	p_data=this->GetData ();        // 取得原图的数据区指针
    wide=this->GetWidth ();         // 取得原图的数据区宽度
    height=this->GetHeight ();      // 取得原图的数据区高度
	int DibWidth = this->GetDibWidthBytes();  // 取得原图的每行字节数（含对齐）

    if(m_pBitmapInfoHeader->biBitCount<9)	// 灰度/索引色图像 (1/4/8位)
	{
		for (int j = 0; j < height; j++) {
			for (int i = 0; i < wide; i++) {
				BYTE* pbyPixel = p_data + j * DibWidth + i;  // 定位当前像素
				*pbyPixel = (*pbyPixel >= threshold) ? 255 : 0;  // 阈值比较：>=阈值→白，否则→黑
			}
		}
	}
	else	// 24位彩色图像 (BGR 顺序，每像素3字节)
	{
		for (int j = 0; j < height; j++) {
			BYTE* row = p_data + j * DibWidth;  // 当前行首地址
			for (int i = 0; i < wide; i++)
			{
				BYTE* pixel = row + i * 3;
				BYTE b = pixel[0];  // 蓝色分量
				BYTE g = pixel[1];  // 绿色分量
				BYTE r = pixel[2];  // 红色分量
				// 加权灰度转换: gray = 0.30R + 0.59G + 0.11B 
				int gray = (30 * r + 59 * g + 11 * b) / 100;
				BYTE bw = (gray >= threshold) ? 255 : 0;  // 二值化
				pixel[0] = pixel[1] = pixel[2] = bw;  // 三通道统一设为黑白值
			}
		}
	}
}

// ====================================================================
// EliminIsoSpots(): 消除孤立黑点
//   算法: 遍历每个像素，检查其 8 邻域中的黑点数量
//   若当前像素为黑色(0)且 8 邻域黑点数 <= 1，则视为孤立噪点，置为白色(255)
//   使用临时缓冲区避免边处理边读取导致的干扰
// ====================================================================
void CProcessDib::EliminIsoSpots()
{
	p_data=this->GetData ();
    wide=this->GetWidth ();
    height=this->GetHeight ();
    int DibWidth = this->GetDibWidthBytes();

	if(m_pBitmapInfoHeader->biBitCount<9)	// 灰度图像
	{
		BYTE* temp = new BYTE[DibWidth * height];     // 开辟临时缓冲区（避免原地修改干扰邻域判断）
		memcpy(temp, p_data, DibWidth * height);      // 拷贝原数据到临时区
		for(int j=1;j<height-1;j++)	// 跳过边界（避免越界）
		{
			for(int i=1;i<wide-1;i++)
			{
				int count=0;
				// 统计当前像素 8 邻域中的黑点数量
				for (int dj = -1; dj <= 1; dj++) {
					for (int di = -1; di <= 1; di++) {
						if(temp[(j + dj)*DibWidth + (i + di)] == 0){
							count++;
						}
					}
				}
				// 若当前为黑点且邻域黑点数 <=1（即仅自身或孤立），则消除
				if (temp[(j)*DibWidth + (i)] == 0 && count <= 1) {
					p_data[(j)*DibWidth + (i)] = 255;  // 置为白色
				}
			}
		}
		delete [] temp;
	}
	else	// 24位彩色（三通道都为0视为黑色）
	{
		BYTE* temp = new BYTE[DibWidth * height];
		memcpy(temp, p_data, DibWidth * height);
		for(int j=1;j<height-1;j++)
		{
			for(int i=1;i<wide-1;i++)
			{
				int count=0;
				for (int dj = -1; dj <= 1; dj++) {
					for (int di = -1; di <= 1; di++) {
					   // 24位像素偏移 = y*DibWidth + x*3
					   int index = (j + dj) * DibWidth + (i + di) * 3;
					   if (temp[index] == 0) {
						   count++;  // 统计黑点个数（仅检查蓝色通道即可，因为二值化后三通道相同）
					   }
					}
				}
				int index0 = j * DibWidth + i * 3;
				if (temp[index0] == 0 && count <= 1) {
					p_data[index0] = p_data[index0 + 1] = p_data[index0 + 2] = 255;  // 三通道置白
				}
			}
		}
		delete [] temp;
	}
}

// ====================================================================
// EliminIsoSpotsWhite(): 消除孤立白点
//   与 EliminIsoSpots() 逻辑对称，针对白色(255)孤立噪点
//   若当前像素为白色且 8 邻域白点数 <= 1，则置为黑色(0)
// ====================================================================
void CProcessDib::EliminIsoSpotsWhite()
{
	p_data=this->GetData ();
	wide=this->GetWidth ();
	height=this->GetHeight ();
	int DibWidth = this->GetDibWidthBytes();

	if(m_pBitmapInfoHeader->biBitCount<9)	// 灰度图像
	{
		BYTE* temp = new BYTE[DibWidth * height];       // 临时缓冲区
		memcpy(temp, p_data, DibWidth * height);       // 拷贝原数据
		for(int j=1;j<height-1;j++)
		{
			for(int i=1;i<wide-1;i++)
			{
				int count=0;  // 白点计数器
				for (int dj = -1; dj <= 1; dj++) {
					for (int di = -1; di <= 1; di++) {
						if(temp[(j + dj)*DibWidth + (i + di)] == 255){  // 检查8邻域是否为白点
							count++;
						}
					}
				}
				if (temp[(j)*DibWidth + (i)] == 255 && count <= 1) {  // 当前为白点且邻域白点数<=1
					p_data[(j)*DibWidth + (i)] = 0;  // 消除孤立白点，置为黑色
				}
			}
		}
		delete [] temp;
	}
	else	// 24位彩色
	{
		BYTE* temp = new BYTE[DibWidth * height];
		memcpy(temp, p_data, DibWidth * height);
		for(int j=1;j<height-1;j++)
		{
			for(int i=1;i<wide-1;i++)
			{
				int count=0;
				for (int dj = -1; dj <= 1; dj++) {
					for (int di = -1; di <= 1; di++) {
						int index = (j + dj) * DibWidth + (i + di) * 3;
						if (temp[index] == 255) {
							count++;
						}
					}
				}
				int index0 = j * DibWidth + i * 3;
				if (temp[index0] == 255 && count <= 1) {
					p_data[index0] = p_data[index0 + 1] = p_data[index0 + 2] = 0;  // 三通道置黑
				}
			}
		}
		delete [] temp;
	}
}

// ====================================================================
// LabelNum(): DFS栈式连通区域标记（8连通）
//   算法: 逐像素扫描，发现未标记的前景像素(黑色=0)时，
//        使用 vector<CPoint> 作为栈进行深度优先搜索(DFS)，
//        将所有 4 连通的前景像素标记为同一编号 (1, 2, 3...)
//   结果: p_temp 存储标签图 (255=背景, 1~x_sign=各区域编号)
//         flag[1..x_sign] 存储各区域面积
//   限制: 最多支持 250 个连通区域 (x_sign > 250 时报错退出)
// ====================================================================
void CProcessDib::LabelNum()
{
	x_sign=0;            // 连通区域计数清零
	m_temp=0;
	x_temp=0;
	y_temp=0;
	stop=0;              // 溢出标志清零
	memset(flag,0,sizeof(flag));  // 面积数组清零
	p_data=this->GetData ();
    wide=this->GetWidth ();
    height=this->GetHeight ();
    int DibWidth = this->GetDibWidthBytes();

    if(m_pBitmapInfoHeader->biBitCount<9)	// 灰度图像
	{
		// 释放旧标签图（避免内存泄漏）
		if (p_temp) { delete[] p_temp; p_temp = nullptr; }
		p_temp=new BYTE[wide*height];             // 分配标签图内存
		memset(p_temp,255,wide*height);            // 初始化为 255（背景/未标记）

		for(int j=0;j<height;j++)	// 逐行扫描（含边界）
		{
			if(stop==1) break;       // 连通区溢出则停止
			for(int i=0;i<wide;i++)	// 逐列扫描（含边界）
			{
				if(x_sign>250)        // 最多支持 250 个区域
				{
					AfxMessageBox("connected region is too much, please increase the number you input ");
					stop=1;
					break;
				}
				int idx0 = j * DibWidth + i;
				// 发现未标记的前景像素（黑色=0且标签图未标记=255）
				if (p_data[idx0] == 0 && p_temp[j * wide + i] == 255)
				{
					x_sign++;  // 新连通区域，编号+1
					if (x_sign > 250) { AfxMessageBox(_T("connected region is too much, please increase the number you input ")); stop=1; return; }
					std::vector<CPoint> stk;     // DFS 栈
					stk.emplace_back(i, j);       // 种子点入栈

					while (!stk.empty())          // DFS 主循环
					{
						CPoint pt = stk.back(); stk.pop_back();  // 弹出栈顶
						int x0 = pt.x, y0 = pt.y; int idx = y0 * wide + x0;
						if (p_temp[idx] != 255) continue;  // 已标记则跳过

						p_temp[idx] = (BYTE)x_sign;  // 标记为当前区域编号
						flag[x_sign]++;               // 该区域面积+1

						// 检查 8 邻域 (上下左右 + 4个对角)，将未标记前景点入栈
						bool l = (x0 > 0), r = (x0 < wide - 1), u = (y0 > 0), d = (y0 < height - 1);
						// 左
						if (l) {
							int nx = x0 - 1, ny = y0;
							if (p_data[ny * DibWidth + nx] == 0 && p_temp[ny * wide + nx] == 255) stk.emplace_back(nx, ny);
						}
						// 右
						if (r) {
							int nx = x0 + 1, ny = y0;
							if (p_data[ny * DibWidth + nx] == 0 && p_temp[ny * wide + nx] == 255) stk.emplace_back(nx, ny);
						}
						// 上
						if (u) {
							int nx = x0, ny = y0 - 1;
							if (p_data[ny * DibWidth + nx] == 0 && p_temp[ny * wide + nx] == 255) stk.emplace_back(nx, ny);
						}
						// 下
						if (d) {
							int nx = x0, ny = y0 + 1;
							if (p_data[ny * DibWidth + nx] == 0 && p_temp[ny * wide + nx] == 255) stk.emplace_back(nx, ny);
						}
						// 左上（对角线）
						if (u && l) {
							int nx = x0 - 1, ny = y0 - 1;
							if (p_data[ny * DibWidth + nx] == 0 && p_temp[ny * wide + nx] == 255) stk.emplace_back(nx, ny);
						}
						// 右上（对角线）
						if (u && r) {
							int nx = x0 + 1, ny = y0 - 1;
							if (p_data[ny * DibWidth + nx] == 0 && p_temp[ny * wide + nx] == 255) stk.emplace_back(nx, ny);
						}
						// 左下（对角线）
						if (d && l) {
							int nx = x0 - 1, ny = y0 + 1;
							if (p_data[ny * DibWidth + nx] == 0 && p_temp[ny * wide + nx] == 255) stk.emplace_back(nx, ny);
						}
						// 右下（对角线）
						if (d && r) {
							int nx = x0 + 1, ny = y0 + 1;
							if (p_data[ny * DibWidth + nx] == 0 && p_temp[ny * wide + nx] == 255) stk.emplace_back(nx, ny);
						}
					}
				}
			}
		}
	}
	else	// 24位彩色（三通道都为0视为前景黑色）
	{
		// 释放旧标签图（避免内存泄漏）
		if (p_temp) { delete[] p_temp; p_temp = nullptr; }
		p_temp = new BYTE[wide*height];
		memset(p_temp,255,wide*height);

		for(int j=0;j<height;j++)	// 逐行扫描（含边界）
		{
			if(stop==1) break;
			for(int i=0;i<wide;i++)	// 逐列扫描（含边界）
			{
				if(x_sign>250)
				{
					AfxMessageBox("connecteed region is too much, please increase the number you input ");
					stop=1;
					break;
				}
				// 24位: 检查三通道是否全为0（黑色）且未标记
				// 像素(x,y)在p_data中的字节偏移 = y*DibWidth + x*3（DibWidth已含每行字节数含对齐）
				int idxColor = j * DibWidth + i * 3;
				if (p_data[idxColor] == 0 && p_data[idxColor+1] == 0 && p_data[idxColor+2] == 0 && p_temp[j * wide + i] == 255)
				{
					x_sign++;
					if (x_sign > 250) { AfxMessageBox(_T("connected region is too much, please increase the number you input ")); stop=1; return; }
					std::vector<CPoint> stk;
					stk.emplace_back(i, j);
					while (!stk.empty())
					{
						CPoint pt = stk.back(); stk.pop_back();
						int x0 = pt.x, y0 = pt.y; int idx = y0 * wide + x0;
						if (p_temp[idx] != 255) continue;
						p_temp[idx] = (BYTE)x_sign;
						flag[x_sign]++;

						// 8邻域扩展（24位需检查3通道都为0）
						bool l = (x0 > 0), r = (x0 < wide - 1), u = (y0 > 0), d = (y0 < height - 1);
						// 左
						if (l) {
							int nx = x0 - 1, ny = y0; int nidx = ny * DibWidth + nx * 3;
							if (p_data[nidx]==0 && p_data[nidx+1]==0 && p_data[nidx+2]==0 && p_temp[ny * wide + nx]==255) stk.emplace_back(nx, ny);
						}
						// 右
						if (r) {
							int nx = x0 + 1, ny = y0; int nidx = ny * DibWidth + nx * 3;
							if (p_data[nidx]==0 && p_data[nidx+1]==0 && p_data[nidx+2]==0 && p_temp[ny * wide + nx]==255) stk.emplace_back(nx, ny);
						}
						// 上
						if (u) {
							int nx = x0, ny = y0 - 1; int nidx = ny * DibWidth + nx * 3;
							if (p_data[nidx]==0 && p_data[nidx+1]==0 && p_data[nidx+2]==0 && p_temp[ny * wide + nx]==255) stk.emplace_back(nx, ny);
						}
						// 下
						if (d) {
							int nx = x0, ny = y0 + 1; int nidx = ny * DibWidth + nx * 3;
							if (p_data[nidx]==0 && p_data[nidx+1]==0 && p_data[nidx+2]==0 && p_temp[ny * wide + nx]==255) stk.emplace_back(nx, ny);
						}
						// 左上（对角线）
						if (u && l) {
							int nx = x0 - 1, ny = y0 - 1; int nidx = ny * DibWidth + nx * 3;
							if (p_data[nidx]==0 && p_data[nidx+1]==0 && p_data[nidx+2]==0 && p_temp[ny * wide + nx]==255) stk.emplace_back(nx, ny);
						}
						// 右上（对角线）
						if (u && r) {
							int nx = x0 + 1, ny = y0 - 1; int nidx = ny * DibWidth + nx * 3;
							if (p_data[nidx]==0 && p_data[nidx+1]==0 && p_data[nidx+2]==0 && p_temp[ny * wide + nx]==255) stk.emplace_back(nx, ny);
						}
						// 左下（对角线）
						if (d && l) {
							int nx = x0 - 1, ny = y0 + 1; int nidx = ny * DibWidth + nx * 3;
							if (p_data[nidx]==0 && p_data[nidx+1]==0 && p_data[nidx+2]==0 && p_temp[ny * wide + nx]==255) stk.emplace_back(nx, ny);
						}
						// 右下（对角线）
						if (d && r) {
							int nx = x0 + 1, ny = y0 + 1; int nidx = ny * DibWidth + nx * 3;
							if (p_data[nidx]==0 && p_data[nidx+1]==0 && p_data[nidx+2]==0 && p_temp[ny * wide + nx]==255) stk.emplace_back(nx, ny);
						}
					}
				}
			}
		}
	}
}

// ====================================================================
// ConnectedRegions(): Two-Pass 并查集连通区域标记 (8连通) + 面积统计
//
//   算法流程:
//     First Pass  (行384-438):
//       - 逐像素扫描，遇到前景点(黑色=0)时检查4个已扫描邻居(上、上左、上右、左)
//       - 若邻居均无标签 → 分配新标签
//       - 若邻居有标签 → 取最小标签，并通过并查集合并不同标签
//     Second Pass (行440-497):
//       - 扁平化标签（将并查集根映射为连续编号1,2,3...）
//       - 统计每个区域的面积
//       - 生成 p_temp 标签图并弹出 SquareDlg 显示结果
//
//   与 LabelNum() 的区别:
//     - LabelNum():     DFS 栈式 4连通标记 (递归深度大时栈可能较大)
//     - ConnectedRegions(): Two-Pass 并查集 8连通标记 (效率更高，无栈溢出风险)
// ====================================================================
void CProcessDib::ConnectedRegions()
{
	// Two-pass connected-component labeling with union-find (8-connectivity).
	// 支持 8-bit (灰度/索引) 和 24-bit 彩色图（颜色为0视为前景）。
	// ===== 初始化图像参数 =====
	p_data = this->GetData();          // 取得原图数据区指针
	wide = this->GetWidth();           // 取得原图数据区宽度
	height = this->GetHeight();        // 取得原图数据区高度
	int DibWidth = this->GetDibWidthBytes();  // 取得 DIB 数据宽度字节数

	FillClosedRegionsForArea(p_data, wide, height, DibWidth, (m_pBitmapInfoHeader->biBitCount < 9));

	// 清理旧标记/面积
	memset(flag, 0, sizeof(flag));
	x_sign = 0;
	stop = 0;

	// 临时标签数组（int 每像素，0 = 未标记/背景）
	int pixCount = wide * height;
	int *labels = new int[pixCount];
	for (int i = 0; i < pixCount; ++i) labels[i] = 0;

	// 并查集 parent 数组: parent[i] = i 表示根节点
	int *parent = new int[pixCount + 1];
	for (int i = 0; i <= pixCount; ++i) parent[i] = i;

	// 并查集 find: 查找根节点（带路径压缩）
	auto findp = [&](int x) {
		int r = x;
		while (parent[r] != r) r = parent[r];
		// 路径压缩：将路径上所有节点直接指向根
		int t;
		while (parent[x] != x) { t = parent[x]; parent[x] = r; x = t; }
		return r;
	};

	// 并查集 unite: 合并两个集合
	auto unite = [&](int a, int b) {
		if (a == 0 || b == 0) return;   // 忽略背景标签 0
		int pa = findp(a), pb = findp(b);
		if (pa != pb) parent[pb] = pa;   // 将 pb 的根指向 pa 的根
	};

	int nextLabel = 1;  // 标签从 1 开始
	bool is8bit = (m_pBitmapInfoHeader->biBitCount < 9);

	
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < wide; ++x)
		{
			// 判断当前像素是否为前景（黑色）
			bool isForeground = false;
			if (is8bit)
			{
				BYTE pix = p_data[y * DibWidth + x];
				isForeground = (pix == 0);  // 黑色为前景
			}
			else
			{
				int idx = y * DibWidth + x * 3;
				isForeground = (p_data[idx] == 0 && p_data[idx + 1] == 0 && p_data[idx + 2] == 0);
			}
			if (!isForeground) continue;  // 非前景跳过

			// 收集已扫描邻居标签（上、上左、上右、左）——8 连通
			int minLabel = 0;
			int neighbors[4] = {0,0,0,0};
			// 上 (y-1, x)
			if (y > 0) neighbors[0] = labels[(y - 1) * wide + x];
			// 上左 (y-1, x-1)
			if (y > 0 && x > 0) neighbors[1] = labels[(y - 1) * wide + (x - 1)];
			// 上右 (y-1, x+1)
			if (y > 0 && x < wide - 1) neighbors[2] = labels[(y - 1) * wide + (x + 1)];
			// 左 (y, x-1)
			if (x > 0) neighbors[3] = labels[y * wide + (x - 1)];

			// 找最小邻居标签
			for (int k = 0; k < 4; ++k)
			{
				if (neighbors[k] != 0)
				{
					if (minLabel == 0 || neighbors[k] < minLabel) minLabel = neighbors[k];
				}
			}

			if (minLabel == 0)
			{
				// 邻居无标签 → 分配新标签
				minLabel = nextLabel++;
				labels[y * wide + x] = minLabel;
			}
			else
			{
				labels[y * wide + x] = minLabel;
			}

			// 合并所有不同的邻居标签（解决标签等价问题）
			for (int k = 0; k < 4; ++k)
			{
				if (neighbors[k] != 0 && neighbors[k] != minLabel)
					unite(minLabel, neighbors[k]);
			}
		}
	}

	
	// mapLabel: 将并查集根映射为连续编号
	int *mapLabel = new int[pixCount + 1];
	for (int i = 0; i <= pixCount; ++i) mapLabel[i] = 0;
	int mapped = 0;

	for (int i = 0; i < pixCount; ++i)
	{
		if (labels[i] == 0) continue;
		int root = findp(labels[i]);     // 找到该像素标签的并查集根
		if (mapLabel[root] == 0)
		{
			++mapped;
			if (mapped > 250)            // 限制标号数以兼容原代码 flag[1..250]
			{
				AfxMessageBox(_T("connected region is too much"));
				stop = 1;
				break;
			}
			mapLabel[root] = mapped;     // 分配连续编号
		}
		labels[i] = mapLabel[root];      // 扁平化标签
		if (!stop) flag[labels[i]]++;    // 统计该区域面积
	}

	if (!stop)
	{
		x_sign = mapped;  // 实际连通区域数

		// 生成 p_temp 标签图（保存标签，未标记位置设为255=背景）
		if (p_temp) { delete[] p_temp; p_temp = nullptr; }
		p_temp = new BYTE[wide * height];
		for (int j = 0; j < height; ++j)
			for (int i = 0; i < wide; ++i)
			{
				int idx = j * wide + i;
				p_temp[idx] = (labels[idx] == 0) ? (BYTE)255 : (BYTE)labels[idx];
			}

		// 复制面积到局部数组 fg 并计算总面积
		int fg[255] = {0};
		for (int i = 1; i <= x_sign; ++i) fg[i - 1] = flag[i];
		int y_sign = x_sign;
		int m_Area = 0;
		for (int i = 0; i < y_sign; ++i) m_Area += fg[i];  // 累加总面积

		// Fill pppp[] for DynSplitView2 overlay: area, index, centroid
		memset(pppp, 0, sizeof(pppp));
		int* sumX = new int[x_sign + 1];
		int* sumY = new int[x_sign + 1];
		memset(sumX, 0, (x_sign + 1) * sizeof(int));
		memset(sumY, 0, (x_sign + 1) * sizeof(int));
		for (int lab = 1; lab <= x_sign; lab++)
		{
			pppp[lab - 1].pp_area = flag[lab];
			pppp[lab - 1].pp_number = lab;
		}
		for (int j = 0; j < height; j++)
		{
			for (int i = 0; i < wide; i++)
			{
				int lab = p_temp[j * wide + i];
				if (lab != 255)
				{
					sumX[lab] += i;
					sumY[lab] += j;
				}
			}
		}
		for (int lab = 1; lab <= x_sign; lab++)
		{
			int area = flag[lab];
			if (area > 0)
			{
				int anchorX = sumX[lab] / area;
				int anchorY = sumY[lab] / area;
				ResolveLabelAnchor(p_temp, wide, height, lab, anchorX, anchorY, pppp[lab - 1].pp_x, pppp[lab - 1].pp_y);
			}
		}
		delete[] sumX;
		delete[] sumY;

		// 弹出面积统计对话框 SquareDlg
		SquareDlg dlg;
		dlg.m_number = y_sign;           // 连通区域个数
		dlg.m_squareALL = m_Area;        // 总面积
		CString ss[20];
		for (int i = 0; i < y_sign && i < 20; ++i)
		{
			ss[i].Format(_T("connected region: %3d  The region's area: %10.0d\r\n"), i + 1, fg[i]);
			dlg.m_ShuChu += ss[i];
		}
		dlg.DoModal();
	}

	// 释放临时资源
	delete[] labels;
	delete[] parent;
	delete[] mapLabel;
}

// ====================================================================
// ClearSMALL(): 小区域消除
//   参数: m_value - 面积阈值（像素数），面积小于此值的区域将被消除
//   流程: 1) 调用 LabelNum() 标记连通区域
//         2) 遍历所有连通区，面积 flag[i] < m_value 的像素全部置为白色(255)
//   应用场景: 去除二值图像中的噪声小区域
// ====================================================================
void CProcessDib::ClearSMALL(int m_value)
{
	LabelNum();  // 调用标记函数（使用 DFS 4连通标记）

    if(m_pBitmapInfoHeader->biBitCount<9)	// 灰度图像
	{
		if(stop!=1)  // 连通区未溢出（<=250个）
		{
			int DibWidth = this->GetDibWidthBytes();
			for(int i=1;i<=x_sign;i++)
			{
				if(flag[i]<m_value)  // 判断连通区的面积是否小于阈值
				{
					// 遍历全图，将该区域所有像素置为白色
                    for(int j=1;j<height;j++)
					{
						for (int k = 1; k < wide; k++) {
							if (p_temp[j * wide + k] == (BYTE)i) {  // 匹配区域编号
                                p_data[j * DibWidth + k] = 255;      // 置白
							}
						}
					}
				}
			}
		}
	}
	else	// 24位彩色
	{
		if(stop!=1)
		{
			for(int i=1;i<=x_sign;i++)
			{
				if(flag[i]<m_value)
				{
					int DibWidth = this->GetDibWidthBytes();
					for(int j=1;j<height-1;j++)
					{
						for (int k = 1; k < wide-1; k++) {
							if (p_temp[j * wide + k] == (BYTE)i) {
								int idx = j * DibWidth + k * 3;
								p_data[idx] = p_data[idx+1] = p_data[idx+2] = 255;  // 三通道置白
							}
						}
					}
				}
				}
			}
		}
}

/***************************************************************/
/*函数名称：Borderline()                                       */
/*函数类型：void                                               */
/*功能：对每个连通区进行边界跟踪，提取边界，输出周长。         */
/***************************************************************/
void CProcessDib::Borderline()
{
	LabelNum();  //调用标记函数
	LPBYTE	lpSrc;  // 指向源图像的指针
	LPBYTE	lpDst;	// 指向缓存图像的指针
	LPBYTE	temp;  // 指向缓存DIB图像的指针
	int pixel;	//像素值
	bool bFindStartPoint;	//是否找到起始点及回到起始点
	bool bFindPoint;	//是否扫描到一个边界点
	Point StartPoint,CurrentPoint;	//起始边界点与当前边界点
	//八个方向和起始扫描方向
	int Direction[8][2]={{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1},{-1,0}};
	int BeginDirect;
	int DibWidth = this->GetDibWidthBytes();   //取得原图的每行字节数
	if(m_pBitmapInfoHeader->biBitCount<9)	//灰度图像
	{
		temp = new BYTE[DibWidth * height]; 	// 暂时分配内存，以保存新图像
		lpDst = temp;	// 初始化新分配的内存，设定初始值为255
		memset(lpDst, (BYTE)255, DibWidth * height);
		if(stop!=1)//判断连通区是否太多
		{
			// ===== 边界检测：8邻域法 =====
			// 遍历每个前景像素(黑色=0)，若其8邻域中存在非前景像素 → 该像素为边界点
			for (int y = 1; y < height - 1; y++) {
				for (int x = 1; x < wide - 1; x++) {
					if (p_data[y * DibWidth + x] == 0) {     // 当前为前景黑色像素
						bool bFindPoint = false;
						for (int dy = -1; dy <= 1 && !bFindPoint; dy++) {
							for (int dx = -1; dx <= 1; dx++) {
								if (p_data[(y + dy) * DibWidth + x + dx] != 0) {
									bFindPoint = true;       // 找到非前景邻居 → 边界点
									break;
								}
							}
						}
						if (bFindPoint) {
							lpDst[y * DibWidth + x] = 0;    // 边界点标记为黑色

						}
					}
				}
			}

			memcpy(p_data, temp, DibWidth * height);// 复制边界结果到原数据区
			delete[] temp; 	// 释放临时缓冲区
			/////////////////////////////////////////////////
			int x_line=0;
			
			int fn[255]={0};// 周长数组：fn[i] = 第i个连通区域的边界像素数
			memset(fn,0,sizeof(fn));
			int y_line=0;   // 有效连通区域计数
			int m_line=0;   // 总周长（所有区域边界像素之和）
			// ===== 周长统计 =====
			// 基于标签图 p_temp 统计每个连通区域的边界像素数（周长）
			// p_temp 由 LabelNum() 填充：255=背景, 1~x_sign=区域编号
			// p_data 中值为0的像素即为边界点（上一步已提取）
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < wide; x++)
				{
					BYTE lab = p_temp[y * wide + x];     // 获取该像素所属区域编号
					if (lab != 255)                       // 属于某个连通区域（非背景）
					{
						int idx = y * DibWidth + x;
						if (p_data[idx] == 0)             // 该像素为边界点
						{
							fn[lab - 1]++;                // 该区域周长+1（标签从1开始，数组从0开始）
							m_line++;                     // 总周长+1
						}
					}
				}
			}
			// 统计有效区域数（fn[i] > 0 表示该区域有边界像素）
			for (int i = 0; i < x_sign; i++)
			{
				if (fn[i] > 0) y_line++;
			}
			LINEDLG  dlg;//输出对话框
			dlg.m_shumu=y_line;//输出连通区域个数
			dlg.m_zongshu=m_line;//输出连通区域的总积
			CString ss[20];
			//在对话框里输出每个连通区的周长（边界像素个数）
			int outCount = 0;
			for (int i = 0; i < x_sign && outCount < 20; i++)
			{
				if (fn[i] > 0)
				{
					ss[outCount].Format("connected regin is %3d  The circle length is :%10.0d\r\n", outCount + 1, fn[i]);//输出
					dlg.m_line += ss[outCount];
					outCount++;//输出计数器
				}
			}
			dlg.DoModal();
			/////////////////////////////////////////////////////////////////
			// Add your code here
			// Fill pppp[] with perimeter and centroid for overlay labels
			memset(pppp, 0, sizeof(pppp));
			int* sumX = new int[x_sign + 1];
			int* sumY = new int[x_sign + 1];
			memset(sumX, 0, (x_sign + 1) * sizeof(int));
			memset(sumY, 0, (x_sign + 1) * sizeof(int));
			for (int lab = 1; lab <= x_sign; lab++)
			{
				pppp[lab - 1].pp_line = fn[lab - 1];
				pppp[lab - 1].pp_number = lab;
				pppp[lab - 1].pp_area = flag[lab];
			}
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < wide; x++)
				{
					BYTE lab = p_temp[y * wide + x];
					if (lab != 255)
					{
						sumX[lab] += x;
						sumY[lab] += y;
					}
				}
			}
			for (int lab = 1; lab <= x_sign; lab++)
			{
				int area = flag[lab];
				if (area > 0)
				{
					int anchorX = sumX[lab] / area;
					int anchorY = sumY[lab] / area;
					ResolveLabelAnchor(p_temp, wide, height, lab, anchorX, anchorY, pppp[lab - 1].pp_x, pppp[lab - 1].pp_y);
				}
			}
			delete[] sumX;
			delete[] sumY;
		}//end if(stop!=1)
	}
	else	//24位彩色
	{
		p_data=this->GetData();
		temp = new BYTE[wide*height*3]; 	// 暂时分配内存，以保存新图像
		lpDst = temp;	// 初始化新分配的内存，设定初始值为255
		memset(lpDst, (BYTE)255, wide * height*3);

		if(stop!=1)//判断连通区是否太多
		{


			// ===== 边界检测：8邻域法（24位版本）=====
			// 三通道都为0=前景黑色，检查8邻域是否有非全黑的像素
			for (int y = 1; y < height - 1; y++) {
				for (int x = 1; x < wide - 1; x++) {
				int idx = y * DibWidth + x * 3;
				if (p_data[idx] == 0 && p_data[idx + 1] == 0 && p_data[idx + 2] == 0) {
					bool bFindPoint = false;
					for (int dy = -1; dy <= 1 && !bFindPoint; dy++) {
						for (int dx = -1; dx <= 1; dx++) {
							int nidx = (y + dy) * DibWidth + (x + dx) * 3;
							if (!(p_data[nidx] == 0 && p_data[nidx + 1] == 0 && p_data[nidx + 2] == 0)) {
								bFindPoint = true;
								break;
							}
						}
					}
					if (bFindPoint) {
						lpDst[idx] = lpDst[idx + 1] = lpDst[idx + 2] = 0;  // 边界点三通道置黑
					}
				}
			}
		}


		memcpy(p_data, temp, wide * height*3);// 复制边界结果到原数据区
			delete[] temp; 	// 释放临时缓冲区
			/////////////////////////////////////////////////
			int x_line=0;
			
			int fn[255]={0};// 周长数组：fn[i] = 第i个连通区域的边界像素数
			memset(fn,0,sizeof(fn));
			int y_line=0;   // 有效连通区域计数
			int m_line=0;   // 总周长
			// ===== 周长统计（24位版本）=====
			// 基于标签图 p_temp 统计每个连通区域的边界像素数（周长）
			// 24位彩色图像：p_data 中三通道都为0的像素即为边界点（上一步已提取）
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < wide; x++)
				{
				BYTE lab = p_temp[y * wide + x];           // 获取该像素所属区域编号
				if (lab != 255)                             // 属于某个连通区域（非背景）
				{
					int idx = y * DibWidth + x * 3;
					// 检查三通道是否都为0（即该像素为边界点）
					if (p_data[idx] == 0 && p_data[idx + 1] == 0 && p_data[idx + 2] == 0)
					{
						fn[lab - 1]++;                      // 该区域周长+1
						m_line++;                           // 总周长+1
					}
				}
				}
			}
			// 统计有效区域数（fn[i] > 0 表示该区域有边界像素）
			for (int i = 0; i < x_sign; i++)
			{
				if (fn[i] > 0) y_line++;
			}
			LINEDLG  dlg;//输出对话框
			dlg.m_shumu=y_line;//输出连通区域个数
			dlg.m_zongshu=m_line;//输出连通区域的总积
			CString ss[20];
			//在对话框里输出每个连通区的周长（边界像素个数）
			int outCount = 0;
			for (int i = 0; i < x_sign && outCount < 20; i++)
			{
				if (fn[i] > 0)
				{
					ss[outCount].Format("connected region: %3d  The region's circle length: %10.0d\r\n", outCount + 1, fn[i]);
					dlg.m_line += ss[outCount];
					outCount++;
				}
			}
			dlg.DoModal();
			/////////////////////////////////////////////////////////////////
			//Add your code here
			// Fill pppp[] with perimeter and centroid for overlay labels
			memset(pppp, 0, sizeof(pppp));
			int* sumX = new int[x_sign + 1];
			int* sumY = new int[x_sign + 1];
			memset(sumX, 0, (x_sign + 1) * sizeof(int));
			memset(sumY, 0, (x_sign + 1) * sizeof(int));
			for (int lab = 1; lab <= x_sign; lab++)
			{
				pppp[lab - 1].pp_line = fn[lab - 1];
				pppp[lab - 1].pp_number = lab;
				pppp[lab - 1].pp_area = flag[lab];
			}
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < wide; x++)
				{
					BYTE lab = p_temp[y * wide + x];
					if (lab != 255)
					{
						sumX[lab] += x;
						sumY[lab] += y;
					}
				}
			}
			for (int lab = 1; lab <= x_sign; lab++)
			{
				int area = flag[lab];
				if (area > 0)
				{
					int anchorX = sumX[lab] / area;
					int anchorY = sumY[lab] / area;
					ResolveLabelAnchor(p_temp, wide, height, lab, anchorX, anchorY, pppp[lab - 1].pp_x, pppp[lab - 1].pp_y);
				}
			}
			delete[] sumX;
			delete[] sumY;

		}//end if(stop!=1)
	}
}

// ====================================================================
// (注释掉的旧版 Borderline()，代码结构更紧凑但逻辑等价)
//   与新版区别：新版灰度分支 p_data 索引统一使用 DibWidth 而非 wide
//   以下为旧版代码保留供参考
// ====================================================================
/*void CProcessDib::Borderline()
{
	// 新实现：统计连通区边界并显示周长对话框
	LabelNum();                     // 先标记所有连通区域
	int DibW = this->GetDibWidthBytes();

	if (m_pBitmapInfoHeader->biBitCount < 9) // 8-bit 灰度图像
	{
		BYTE* temp = new BYTE[wide * height];
		memset(temp, 255, wide * height);    // 初始化为全白

		// 边界检测: 前景点(0)且8邻域存在非前景点 → 边界
		for (int y = 1; y < height - 1; ++y) {
			for (int x = 1; x < wide - 1; ++x) {
				int idx = y * DibW + x;
				if (p_data[idx] == 0) {       // 当前为前景
					bool bFind = false;
					for (int dy = -1; dy <= 1 && !bFind; ++dy) {
						for (int dx = -1; dx <= 1; ++dx) {
							int nidx = (y + dy) * DibW + (x + dx);
							if (p_data[nidx] != 0) { bFind = true; break; }  // 找到非前景邻居
						}
					}
					if (bFind) temp[idx] = 0;  // 标记为边界点
				}
			}
		}

		// 将边界结果复制回原数据区
		memcpy(p_data, temp, wide * height);
		delete[] temp;

		// 基于标签图 p_temp 统计每个区域的周长（边界像素数）
		int fn[255] = {0}; int total = 0;
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < wide; ++x) {
				BYTE lab = p_temp[y * wide + x];  // 获取区域编号
				if (lab != 255) {                  // 属于某个连通区
					int idx = y * DibW + x;
					if (p_data[idx] == 0) { fn[lab - 1]++; total++; }  // 该区域周长+1
				}
			}
		}

		// 统计有效区域数并弹出周长对话框
		int cnt = 0; for (int i = 0; i < 255; ++i) if (fn[i] > 0) cnt++;
		LINEDLG dlg; dlg.m_shumu = cnt; dlg.m_zongshu = total;  // 区域数、总周长
		CString ss; ss.Empty();
		int out=0;
		for (int i = 0; i < 255 && out < 20; ++i) {
			if (fn[i] > 0) { CString s; s.Format(_T("connect regin is %3d  The circle length is :%10.0d\r\n"), out+1, fn[i]); ss += s; ++out; }
		}
		dlg.m_line = ss; dlg.DoModal();
	}
	else // 24-bit 彩色图像
	{
		p_data = this->GetData();
		BYTE* temp = new BYTE[wide * height * 3];
		memset(temp, 255, wide * height * 3);
		int DibWidth = DibW;
		for (int y = 1; y < height - 1; ++y) {
			for (int x = 1; x < wide - 1; ++x) {
			int idxc = y * DibWidth + x * 3;
			if (p_data[idxc] == 0 && p_data[idxc+1] == 0 && p_data[idxc+2] == 0) {
				bool bFind = false;
				for (int dy = -1; dy <= 1 && !bFind; ++dy) {
					for (int dx = -1; dx <= 1; ++dx) {
						int nidx = (y + dy) * DibWidth + (x + dx) * 3;
							if (!(p_data[nidx] == 0 && p_data[nidx+1] == 0 && p_data[nidx+2] == 0)) { bFind = true; break; }
						}
					}
					if (bFind) {
						temp[idxc] = temp[idxc+1] = temp[idxc+2] = 0;
					}
				}
			}
		}
		memcpy(p_data, temp, wide * height * 3); delete[] temp;
		// 统计各区域周长
		int fn[255] = {0}; int total = 0;
		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < wide; ++x) {
				BYTE lab = p_temp[y * wide + x];
				if (lab != 255) {
			int idxc = y * DibWidth + x * 3;
			if (p_data[idxc] == 0 || p_data[idxc+1] == 0 || p_data[idxc+2] == 0) { fn[lab-1]++; total++; }
				}
			}
		}
		int cnt = 0; for (int i = 0; i < 255; ++i) if (fn[i] > 0) cnt++;
		LINEDLG dlg; dlg.m_shumu = cnt; dlg.m_zongshu = total;
		CString ss; ss.Empty(); int out=0;
		for (int i = 0; i < 255 && out < 20; ++i) { if (fn[i] > 0) { CString s; s.Format(_T("connected region: %3d  The region's circle length: %10.0d\r\n"), out+1, fn[i]); ss += s; ++out; } }
		dlg.m_line = ss; dlg.DoModal();
	}
}
*/
// ====================================================================
// MakeGray(): 24位彩色图像转灰度图
//   使用加权公式: gray = (30R + 59G + 11B) / 100
//   将 RGB 三通道设置为相同的灰度值，实现彩色转灰度效果
// ====================================================================
void CProcessDib::MakeGray()
{
	BYTE *p_data;     //原图数据区指针
	int wide,height,DibWidth;    //原图长、宽、字节宽
	p_data=this->GetData ();   //取得原图的数据区指针
	wide=this->GetWidth ();   //取得原图的数据区宽度
	height=this->GetHeight ();   //取得原图的数据区高度
	DibWidth=this->GetDibWidthBytes();   //取得原图的每行字节数
	for(int j=0;j<height;j++)	// 每行
	{
		BYTE* row = p_data + j * DibWidth;
		for(int i=0;i<wide;i++)	// 每列
		{
			BYTE* pixel = row + i * 3;
			BYTE b = pixel[0];
			BYTE g = pixel[1];
			BYTE r = pixel[2];
			int gray=0;
			gray=(30*r+59*g+11*b)/100;
			pixel[0] = gray;        // 三通道统一设为灰度值
			pixel[1] = gray;
			pixel[2] = gray;
		}
	}
}

// ====================================================================
// SaveOrigin(): 备份原始图像数据
//   将当前 p_data 拷贝到 temp 缓冲区中，用于后续恢复
//   在 24 位彩色图处理流程中，先备份原图，转灰度后再叠加显示
// ====================================================================
void CProcessDib::SaveOrigin(LPBYTE temp)
{
	// 指向DIB象素指针
	LPBYTE p_data;
	// 指向缓存图像的指针
	LPBYTE	lpDst;
	// 找到DIB图像象素起始位置
	p_data= GetData();
	// DIB的宽度
	LONG wide = GetDibWidthBytes();
	// DIB的高度
	LONG height = GetHeight();
	// 初始化新分配的内存，设定初始值为255（全白）
	lpDst = (LPBYTE)temp;
	memset(lpDst, (BYTE)255, wide * height);
	memcpy(lpDst,p_data,wide*height);  // 拷贝原图数据到缓冲区
}
