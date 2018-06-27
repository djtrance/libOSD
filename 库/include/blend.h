#ifndef _BLEND_H
#define _BLEND_H


#include "typedef.h"
#define FONT_MAX_SIZE 9

#define ZK_WIDTH 			16//字库文件宽 暂时不支持其他
#define ZK_HEIGHT 			16//字库文件高 暂时不支持其他
#define BYTE_SIZE			(ZK_WIDTH*ZK_HEIGHT)

//图像类型
#define  IMAGE_UYVY	     0	//UYVY图像
#define  IMAGE_YUV420P	     1  //YUV420P图像



/*
叠加内容结构
*/
typedef struct _blend_str_mess
{
	unsigned char string[50];//要叠加的字符串
	int iLeft;//左坐标
	int iTop;//上坐标
}blend_str_mess;

/*
叠加信息结构体，里面包含了需要叠加的内容
*/
typedef struct _blend_mess
{
	int time; 	//是否叠加时间
	int iTLeft;	//时间坐标
	int iTTop;
	blend_str_mess strMess[3];
	int font ;	//1黑体 2楷体 3宋体 4仿宋体 5隶体
	int iSize;	//字号 1--9
	int isAddEdge;	//是否描边
	int backGround; //0无背景 1纯色背景 2点状背景 3 线状背景
	 
}blend_mess;
/*
叠加结构，叠加时候使用的结构，起包括了控制，点阵信息等
*/
typedef struct blendInfo{
	blend_mess m_blend_mess; //叠加信息
	char * m_blendData; //点阵数据指针（外部不用操作，算法中自动使用）
	char * m_digitData[15];//数字的点阵数据 用于叠加时间
	int iBitLen[3];//需要叠加的strMess对应的点阵数据的偏移量（外部不用操作）
	int initFlag; // 是否进行了初始化 0否 1 是（外部不用操作）
}blendInfo;
/*
 * 函数名：blend_init_no_vdce
 * 函数功能：
	初始化静态叠加
	第一次进入的时候会给叠加结构体进行分配空间
	之后的进入会根据叠加信息获取点阵数据到叠加结构体中
 * 参数：myblend_info[in/out] 叠加信息结构体
   返回值:
	<=0  失败
	其他 成功
	
 */
int blend_init_no_vdce(blendInfo *myblend_info);

/*
 * 函数名：blend_begin_no_vdce
 * 函数功能：开始叠加
 * 参数： pInBuf[in/out]yuv数据
 * 		width【in】yuv数据的宽
 * 		height【in】yuv数据的高
 *		imageType [in] :0 UYVY 1 yuv420P　参照头文件中的宏定义
		myblend_info[in]:叠加信息结构体
 */
int blend_begin_no_vdce (byte * pInBuf, int width, int height,int imagetype,blendInfo *myblend_info);

/*
函数名称:blend_time
函数功能:设置时间叠加使能
参数:
	blend[in]:0不起用 1 启用
	myblend_info [in/out]:叠加结构体
返回值:
	0
*/
int blend_time(int blend,blendInfo *myblend_info);

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
 * 参数：myblend_info [in]叠加信息结构题
 */
int setblent(blendInfo *myblend_info);
/*
 * 函数名：ExitBlend
 * 函数功能：释放静态叠加申请的内存
 */

void ExitBlend(blendInfo *myblend_info);

#endif
