// ====================================================================
// 文件名: ProcessDib.h
// 描  述: 图像处理核心类的声明
// 功  能: 继承自 CDib，实现二值图像处理的所有核心算法：
//         - 固定阈值二值化 (BinaryOperation)
//         - 孤立点消除   (EliminIsoSpots / EliminIsoSpotsWhite)
//         - 连通区域标记 (LabelNum: DFS栈式4连通)
//         - 连通区域分析 (ConnectedRegions: Two-Pass并查集8连通 + 面积统计)
//         - 小区域消除   (ClearSMALL: 基于面积阈值过滤)
//         - 边界提取     (Borderline: 边界检测 + 周长统计)
//         - 彩色转灰度   (MakeGray)
// ====================================================================

#if !defined(AFX_JISUANPROCESSDIB_H__6385E9FA_7E01_4785_9F75_56E9F77F4702__INCLUDED_)
#define AFX_JISUANPROCESSDIB_H__6385E9FA_7E01_4785_9F75_56E9F77F4702__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Cdib.h"

// 坐标点结构体（用于边界跟踪）
typedef struct{
	int Height;
	int Width;
}Point;

// 连通区域对象结构体
//   pp_x/pp_y: 区域位置坐标
//   pp_area:   区域面积（像素数）
//   pp_line:   区域周长（边界像素数）
//   pp_number: 区域编号
struct object{
	int pp_x;
	int pp_y;
	int pp_area;
	int pp_line;
	int pp_number;
};


class CProcessDib : public CDib  
{
public:
	// ===== 图像处理核心方法 =====
	void SaveOrigin(LPBYTE temp);            // 备份原始图像数据
	void MakeGray();                         // 24位彩色图转灰度图（加权公式）
	void ConnectedRegions();                 // Two-Pass并查集连通区域分析（8连通）+ 面积统计
	void Borderline();                       // 边界提取 + 周长统计
	void ClearSMALL(int m_value);            // 小区域消除（面积 < m_value 的区域置白）
	void LabelNum();                         // DFS栈式连通区域标记（8连通）
	void EliminIsoSpotsWhite();              // 消除孤立白点
	void EliminIsoSpots();                   // 消除孤立黑点
	void BinaryOperation(int yuzhi_gray);    // 固定阈值二值化
             CProcessDib();
	virtual ~CProcessDib();

	// ===== 图像属性（运行时填充）=====
	BYTE *p_data;     // 原图数据区指针
	int wide,height;  // 原图宽、高

	// ===== 连通区域相关数据 =====
    object pppp[255];  // 连通区域对象数组（最多 250 个区域）
    int x_sign;        // 连通区域总数
	int flag[255];     // 每个连通区域的面积（flag[i] = 第i个区域的像素数）
	int m_temp;        // 临时变量
	int x_temp;        // 临时 X 坐标
	int y_temp;        // 临时 Y 坐标
	BYTE *p_temp;      // 临时标签图（LabelNum/ConnectedRegions 填充）

	int stop;          // 溢出标志（连通区域数 > 250 时置 1）
};

#endif // !defined(AFX_JISUANPROCESSDIB_H__6385E9FA_7E01_4785_9F75_56E9F77F4702__INCLUDED_)
