#ifndef _BLEND_H
#define _BLEND_H


#include "NewStruct.h"
#include "typedef.h"
#define FONT_MAX_SIZE 9

#define ZK_WIDTH 			16//字库文件宽 暂时不支持其他
#define ZK_HEIGHT 			16//字库文件高 暂时不支持其他
#define BYTE_SIZE			(ZK_WIDTH*ZK_HEIGHT)

//图像类型
#define  IMAGE_UYVY	     0	//UYVY图像
#define  IMAGE_YUV420P	     1  //YUV420P图像

typedef struct _blend_str_mess
{
	unsigned char string[50];
	int iLeft;
	int iTop;
}blend_str_mess;


typedef struct _blend_mess
{
	int time; 	//是否叠加时间
	int iTLeft;	//时间坐标
	int iTTop;
	blend_str_mess strMess[3];
	int font ;	//1黑体 2楷体 3宋体 4仿宋体 5隶体
	int iSize;
	 
}blend_mess;

typedef struct blendInfo{
	blend_mess m_blend_mess;
	char m_last_time[25];
	char * m_blendData;
	int iBitLen[3];

}blendInfo;
/*
 * 函数名：blend_init_no_vdce
 * 函数功能：初始化静态叠加
 * 参数：index【in】初始化标志。当为0的时候完全初始化，包括内存申请 ！=0的时候只是初始化其中的值 *
 */

int blend_init_no_vdce(int index);
/*
 * 函数名：blend_begin_no_vdce
 * 函数功能：开始静态叠加
 * 参数： pInBuf[in/out]yuv数据
 * 		width【in】yuv数据的宽
 * 		height【in】yuv数据的高
 * 		index【in】暂时无用
 * 		iType【in】暂时无用
 *		imageType [in] :0 UYVY 1 yuv420P　参照头文件中的宏定义
 */
int blend_begin_no_vdce(byte * pInBuf, int width, int height,int imageType);


void set_BlendResetVal(int val);
//静态叠加时间开关
//0关 1开
int blend_time(int enable);

/*
 * 函数名： setFondColor
 * 函数功能：设置叠加字体的颜色
 * 参数：R[in]红色
 * 		G[in]绿
 * 		B[in]蓝
 */
void setFondColor(int R,int G,int B);
/*
 * 函数名： setBordColor
 * 函数功能：设置叠加底色/边框的颜色（未使用）
 * 参数：R[in]红色
 * 		G[in]绿
 * 		B[in]蓝
 */
void setBordColor(int R,int G,int B);
/*
 * 函数名：setblent
 * 函数功能：设置静态叠加信息
 * 参数：newblendinfo[in]静态叠加信息结构提
 */
int  setblent(blend_mess blendinfo);
/*
 * 函数名：ExitBlend
 * 函数功能：释放静态叠加申请的内存
 */
void ExitBlend();


#endif
