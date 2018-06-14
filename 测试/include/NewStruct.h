/*
 * auth:ywj
 *
 */
#ifndef _NESSTRUCT_H_
#define _NESSTRUCT_H_

#include "typedef.h"


#define MAX_OVERMESSNUM     15           //最大动态叠加信息个数
#define MAX_OVERCHARNUM     50          //最大动态叠加字符个数

typedef struct _dynamic_over_mess{
	int iLeftDis;//左边距 	范围：0 到 图像宽-1
	int iTopDis;//上边距	 范围：0 到 图像高-1
	int iFont;//字体	范围：1黑体 2楷体 3宋体 4仿宋体 5隶体
	int iFontSize;//字号	范围：1 到 9
	unsigned char szText[MAX_OVERCHARNUM];//叠加文本	字符串，长度小于50，支持数字、字母、汉字
}s_dynamic_over_mess;//动态叠加信息结构


typedef struct _dynamic_VideoOverInfo{
	int VideoOverFlag;//视频叠加标志 0-不叠加 1-叠加
}
s_dynamic_VideoOverInfo;

typedef struct _dynamic_over_info
{
	int m_iDynamicOverMessNum;//叠加信息个数
	s_dynamic_over_mess m_pOver_mess[MAX_OVERMESSNUM];
}s_dynamic_over_info;

#endif
