#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include "blend.h"

/*
   颜色结构体
 */
typedef struct _Scolor
{
	int Y;
	int U;
	int V;
}Scolor;
//字体颜色
Scolor m_FontColor = {0xff,0x80,0x80};
//边框颜色或者底色
Scolor m_BordColor ={0x00,0x80,0x80};

//静态叠加初始化结构
blend_mess m_blend_mess_channelx =
{
	.font = 3,//宋体
	.strMess =
	{
	{"", 0, 0},
	{"", 0, 0},
	{"", 0, 0},
	},
	.time = 1,//启用时间叠加
	.iTLeft = 10,//时间的左坐标
	.iTTop = 10,//时间的上坐标
	.iSize = 3,//字体大小默认为3号
	.isAddEdge = 0,//不描边
	.backGround = 2//使用点状底纹
};

/*
   用于检出时间字符串点阵字库
 */
char m_timeStr[15]="0123456789-:";
//位字节点阵字库存储区
static unsigned char ascFontData[1024*4] = {0};//ASC 字库
static unsigned char hzkFontData[1024*300] = {0};//汉字库
//字节字库存储区
static unsigned char ascFontDataByte[1024*4*8] = {0};//ASC||字库
static unsigned char hzkFontDataByte[1024*300*8] = {0};


//获取当前时间保存为字符串
int GetTimeString(char * str)
{
	time_t timep;
	struct tm * p = NULL/*, timeSc*/;
	time(&timep);

	p=localtime(&timep);
	sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d",  p -> tm_year + 1900, 1 + p -> tm_mon, p -> tm_mday,\
							    p->tm_hour, p -> tm_min, p -> tm_sec);
	
	return 0;
}

/*
函数名称:blend_time
函数功能:设置时间叠加使能
参数:
	blend[in]:0不起用 1 启用
	myblend_info [in/out]:叠加结构体
返回值:
	0
*/
int blend_time(int blend,blendInfo *myblend_info)
{
	if(blend == 1)
		myblend_info->m_blend_mess.time = 1;
	else
		myblend_info->m_blend_mess.time = 0;
	return 0;
}

static int m_iReset = 0;
//重启静态叠加开关 0 不重启 1重启
void set_BlendResetVal(int val)
{
	m_iReset = val;
}

/*
 * 函数名：BitmapByte
 * 函数功能： 将位数据保存为字节数据，把bit的每一位转换成为一个字节
 * 参数： 
	buff [out]转换后的数据 
	bit  [in] 需要转换的数据
 * 返回值: null
 */
static  inline void BitmapByte(byte * buff, char bit)
{
	int32 j;
	byte vl;
	byte mask[2] = {0x00,0x01};
	byte maskbit[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};  //取设置字节位用的掩码

	for(j = 0; j < 8; j++)
	{
		vl = (bit & maskbit[7-j]) ? 1 : 0;
		*buff = *buff | mask[vl];
		buff++;
	}

}

/*
函数功能，从字库中读取数据并转换成字节码
argument:
	width[in]: The wedth of font
	height[in]:The height of font 
	hzk[in]: The type of font
return:
	1:success
     	-1 failed
*/
int GetFontData(int width,int height,int hzk)
{
	FILE *hzkfp = NULL, *ascfp = NULL;//The file point of font	
	char filenamezk[200];
	char filenameasc[200];
	int i = 0;
	int asclen = 0;
	int haklen = 0;
	//int iOffset = 0;
	switch(hzk)
	{
	case 1://黑体
		sprintf(filenamezk, "./font/ht%d%d.zk", width, height);
		break;
	case 2://楷体
		sprintf(filenamezk, "./font/kt%d%d.zk", width, height);
		break;
	case 3://宋体
		sprintf(filenamezk, "./font/st%d%d.zk", width, height);
		break;
	case 4://仿宋体
		sprintf(filenamezk, "./font/ft%d%d.zk" , width, height);
		break;
	case 5://隶体
		sprintf(filenamezk, "./font/lt%d%d.zk", width, height);
		break;
	default://缺省为宋体
		sprintf(filenamezk, "./font/st%d%d.zk", width, height);
		break;
	}
	/*
	   open font  library

	 */
	if ((hzkfp = fopen( filenamezk, "rb")) == NULL)  //无该字库文件返回
	{
		return -1;
	}
	sprintf(filenameasc, "./font/asc%d%d.zk", width, height);
	if ((ascfp = fopen( filenameasc, "rb" )) == NULL)	//无该字库文件返回
	{
		fclose(hzkfp);
		return -1;
	}
	asclen = fread(ascFontData,1, 1024*4, ascfp);
	haklen = fread(hzkFontData,1, 1024*300, hzkfp);
	if(hzkfp != NULL)
	{
		fclose(hzkfp);
	}
	fclose(ascfp);	
	//iOffset = 0;
	//bit font to byte font
	for(i = 0;i < asclen;i++)
	{
		BitmapByte(ascFontDataByte+ i * 8,ascFontData[i]);
	}
	for(i = 0;i < haklen;i++)
	{
		BitmapByte(hzkFontDataByte+ i * 8,hzkFontData[i]);
	}
	return 1;
}
//===================================================================================
//  函 数 名：   GetLattice
//	功能：根据要叠加的内容取出字节码的点阵
//	入口：width,height字体大小：16、24、32、48
//	data: 保存生成的点阵（由调用者提供足够的缓冲区）
//	num:要转换的数据
//	返回：0-没有生成  其它-生成的点阵大小
//===================================================================================

int GetLattice(byte *data,byte *num,int width,int height, int hzk)
{
	unsigned long offset;
	unsigned char *pdata;
	short hzSize = (width * height) ;	//汉字模大小（字节）
	short ascSize = (width* height)/2;	//ASC码字模大小（字节）宽度为汉字的一半

 	pdata = data;
	if (*num < 0x80)   //ASCII处理
	{
		offset = *num * ascSize;			//width*height ASCII
		memcpy(pdata,ascFontDataByte+offset,ascSize);
	}

	if ( *num >= 0xA1 )	//汉字处理
	{
		//：(94*(区号-1)+位号-1)*一个汉字字模占用字节数
		offset = (( *num - 0xA1) * 94 + *(num +1) - 0xA1) * hzSize;
		memcpy(pdata,hzkFontDataByte+offset,hzSize);
	}

	return 0;
}
/*
 * 函数名“：ZoomUp
 * 函数功能：对点阵进行放大
 * 参数：
	in [in]需要放大的点阵 
	out[out]放大后的点阵 
	iWidth[in]点阵的宽 
	iHeight[out]点阵的高 
	iSize 字体大小
   返回值:
	0
	
 */
int32 ZoomUp(int8 * in, int8 * out, int32 iWidth, int32 iHeight, int32 iSize)
{
	int32 i, j, m, n;
	int8 * ptr = in;

	if(iSize == 1)
		memcpy(out,in,iWidth*iHeight);
	
	for(i = 0; i < iHeight; i ++)
	{
		for(j = 0; j < iWidth; j ++)
		{

			/*example
			 * 1 2==>isize = 2 				1 1 2 2
			 * 3 4						1 1 2 2
			 * 						3 3 4 4
			 * 						3 3 4 4
			 *
			 *		==>isize = 3			1 1 1 2 2 2
			 *						1 1 1 2 2 2
			 *						1 1 1 2 2 2
			 *						3 3 3 4 4 4
			 *						3 3 3 4 4 4
			 *						3 3 3 4 4 4
			 */
			if(*ptr != 0)
			{
				for(m = 0; m < iSize; m ++)
				{
					for(n = 0; n < iSize; n ++)
					{
						*(out + i * iWidth * iSize * iSize + j * iSize + m * iWidth * iSize + n) = *ptr;
					}
				}
			}
			ptr ++;
		}
	}
	return 0;
}

/*
函数名称: AddBlackBoard
函数功能:给点阵数据添加黑边
参数:
	Data[in/out]:点阵数据
	iWidth[in]:点阵数据的宽
	iHeight[in]:点阵数据的高
	isAddEdge[in]:是否添加黑边 0 不添加 1 添加
返回值:
	0
	
*/
int32 AddBlackBoard(int8 * Data, int32 iWidth, int32 iHeight,int isAddEdge)
{

	int32 i,j;
	int8 tempData[160 * 160] = {0};
	int8 * temp = tempData;
	int8 * ptemp1 = Data;
	//不进行操作直接返回
	if(isAddEdge == 0)
		return 0;
	//拷贝点阵数据，一会进行操作的时候使用
	memcpy(tempData, Data, iWidth * iHeight);
	//给每个需要填充的像素的上下左右添加描边
	for(i = 0; i < iHeight; i ++)
	{
		for(j = 0; j < iWidth; j ++)
		{
			if(*ptemp1 == 1)
			{
				if(j != 0)
				{
					*(temp - 1) = 2;
					//*(temp - 1 - iWidth) = 2;
				}

				*(temp - iWidth) = 2;
				//*(temp + 1 - iWidth) = 2;
				*(temp + 1) = 2;
				//*(temp + 1 + iWidth) = 2;
				*(temp + iWidth) = 2;
				//*(temp -1 + iWidth) = 2;
			}
			temp ++;
			ptemp1 ++;
		}
	}
	//修改点阵数据
	for(i = 0; i < iWidth * iHeight; i ++)
	{
		 if((tempData[i] == 2) && (Data[i] == 0))
			 Data[i] = 2;
	}
	return 0;

}
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
int blend_init_no_vdce(blendInfo *myblend_info)
{
	int i, j;
	int strLen;
	int iOffset = 0;
	//static int blendInit[4] = {0,0,0,0};
	byte tempData[BYTE_SIZE];
	strLen = 110;
	int ret = 0;
	//static time_t timestrLastblend = 0; //last time of blend time str 
	//time_t nowtime = 0;//now time
	if (myblend_info->initFlag == 0)
	{
		
		//timestrLastblend = time(NULL);
		if(0 > GetFontData(ZK_WIDTH,ZK_HEIGHT,myblend_info->m_blend_mess.font))
			return -1;
			
		myblend_info->m_blend_mess = m_blend_mess_channelx;//初始化叠加信息结构
		myblend_info->m_blendData = (char *) malloc(strLen * BYTE_SIZE * FONT_MAX_SIZE * FONT_MAX_SIZE);//叠加数据申请
		if (myblend_info->m_blendData == NULL)
		{
			printf("memory failed blend 1 \n");
			return -1;
		}

		for(i = 0; i < strlen(m_timeStr); i ++)//初始化点阵存储空间
		{
			myblend_info->m_digitData[i] = malloc(BYTE_SIZE/2  * FONT_MAX_SIZE * FONT_MAX_SIZE);
			if (myblend_info->m_digitData[i] == NULL)
			{
				printf("memory failed blend 2 \n");
				return -1;
			}
		}
		myblend_info->initFlag = 1;
	}
	memset(myblend_info->m_blendData, 0, strLen * BYTE_SIZE * FONT_MAX_SIZE * FONT_MAX_SIZE);
	//对时间数字点阵进行初始化
	//nowtime = time(NULL);
	//once  per second
	//if(nowtime != timestrLastblend)
	//{
		//timestrLastblend = nowtime;
		for(i = 0; i < strlen(m_timeStr); i ++)
		{
			memset(myblend_info->m_digitData[i], 0, BYTE_SIZE/2 * FONT_MAX_SIZE * FONT_MAX_SIZE);
			memset(tempData, 0, BYTE_SIZE);
			//获取点阵
			ret = GetLattice(tempData,(byte*)&(m_timeStr[i]),ZK_WIDTH,ZK_HEIGHT, myblend_info->m_blend_mess.font);
			//放大
			ZoomUp((int8 *)tempData, myblend_info->m_digitData[i], ZK_WIDTH/2, ZK_HEIGHT, myblend_info->m_blend_mess.iSize); 
			AddBlackBoard(myblend_info->m_digitData[i],ZK_WIDTH/2 * myblend_info->m_blend_mess.iSize,ZK_HEIGHT*myblend_info->m_blend_mess.iSize,myblend_info->m_blend_mess.isAddEdge);
		}
//	}

	//对文字进行初始化
	for(j = 0; j < 3; j ++)
	{
		myblend_info->iBitLen[j] = iOffset;
		for(i = 0; i < strlen((char *)myblend_info->m_blend_mess.strMess[j].string); i++)
		{
			memset(tempData, 0, BYTE_SIZE);
			//汉字初始化
			if(myblend_info->m_blend_mess.strMess[j].string[i] >= 0xA1)
			{
				ret = GetLattice(tempData,&(myblend_info->m_blend_mess.strMess[j].string[i]),ZK_WIDTH,ZK_HEIGHT, myblend_info->m_blend_mess.font);
				ZoomUp((int8 *)tempData, myblend_info->m_blendData + iOffset, ZK_WIDTH, ZK_HEIGHT, myblend_info->m_blend_mess.iSize);
				AddBlackBoard(myblend_info->m_blendData + iOffset,ZK_WIDTH*myblend_info->m_blend_mess.iSize,ZK_HEIGHT*myblend_info->m_blend_mess.iSize,myblend_info->m_blend_mess.isAddEdge);
				i ++;
				iOffset += BYTE_SIZE * myblend_info->m_blend_mess.iSize*myblend_info->m_blend_mess.iSize;
			}
			else
			{//字母初始化
				ret = GetLattice(tempData,&(myblend_info->m_blend_mess.strMess[j].string[i]),ZK_WIDTH,ZK_HEIGHT, myblend_info->m_blend_mess.font);	
				ZoomUp((int8 *)tempData, myblend_info->m_blendData + iOffset, ZK_WIDTH/2, ZK_HEIGHT, myblend_info->m_blend_mess.iSize);
				AddBlackBoard( myblend_info->m_blendData + iOffset,ZK_WIDTH/2 * myblend_info->m_blend_mess.iSize,ZK_HEIGHT*myblend_info->m_blend_mess.iSize,myblend_info->m_blend_mess.isAddEdge);
				iOffset += BYTE_SIZE/2 * myblend_info->m_blend_mess.iSize*myblend_info->m_blend_mess.iSize;
			}
		}
	}

	return ret;
}

/*
函数名称 yuv420PixelSet
函数功能 修改420P的一个像素点的颜色
参数：
	yuvData[in/out] yuv数据缓冲区
	color[in] 颜色参数
	x[in] 	  像素的x坐标
	y[in] 	  像素的y坐标
	width[in] 图像宽度
	height[in]图像的高度
返回值:
	null

*/
void yuv420PixelSet(byte *yuvData,Scolor color,int x,int y,int width,int height)
{
	if(y%2 == 0)//偶数
	{
		*(yuvData+y*width+x) = color.Y;
		*(yuvData+ width*height + y*width/4+x/2) = color.U;
		*(yuvData+ width*height + width*height /4 +y*width/4+x/2) = color.V;
	}else //奇数
	{
		*(yuvData+y*width+x) = color.Y;
	}
}



/*
函数名称: UYVYSetPixelColor
函数功能:
	根据像素索引修改UYVY这个像素数据的颜色
参数:
	pixelIndex[in]  像素点相对于UYVY数据起点的偏移索引
	yuvAddr[in/out]	   当前需要被修改的UYVY数据的地址，该地址开始的两个字节分别为该像素的Y 数据 和U/V数据
	color[in]颜色
返回值 0
*/
int UYVYSetPixelColor(int pixelIndex,byte* yuvAddr,Scolor color)
{	

	//由于是UYVY 数据 每相邻的两个字节中只有一个是Y 别的是U 或者V 所以需要根据字摸同时修改这两个字节
	//根据像素偏移计算出yuvAddr 是YUV 中的那一个然后修改
	if((pixelIndex%4) == 2)
		*(yuvAddr)=  color.V;
	else if((pixelIndex%4) == 0)
		*(yuvAddr)= color.U;
	else
		*(yuvAddr) = color.Y;
	//根据像素偏移计算出yuvAddr+1 是YUV 中的那一个然后修改
	if((((pixelIndex+1))%4) == 2)
		*(yuvAddr+1) =  color.V;
	else if((((pixelIndex+1))%4) == 0)
		*(yuvAddr+1) =  color.U;
	else
		*(yuvAddr+1) = color.Y;
	return 0;
	
}

/*
函数名称: YUV422SetGroundPixel
函数功能
	根据像素索引修改UYVY数据为底色
参数:
	pixelIndex[in]  像素点相对于UYVY数据起点的偏移索引
	yuvAddr[in]	   当前需要被修改的UYVY数据的地址，该地址开始的两个字节分别为该像素的Y 数据 和U/V数据
返回值 0
*/
int YUV422SetGroundPixel(int pixelIndex,byte* yuvAddr)
{
	//设置成为底色	
 	UYVYSetPixelColor(pixelIndex,yuvAddr,m_BordColor);//uyvy
	return 0;
}
/*
函数名称:YUV422SetFontPixel
函数功能:
	根据像素索引修改UYVY数据为字体颜色
参数:
	pixelIndex  像素点相对于UYVY数据起点的偏移索引
	yuvAddr	   当前需要被修改的UYVY数据的地址，该地址开始的两个字节分别为该像素的Y 数据 和U/V数据
返回值 0
*/
int YUV422SetFontPixel(int pixelIndex,byte* yuvAddr)
{
	//设置成为字体颜色
 	UYVYSetPixelColor(pixelIndex,yuvAddr,m_FontColor);

	return 0;
}
/*
 * 作者：yiweijiao
 * time :218-04-11
 * function: 主要用于叠加时间到图像上，调用一次函数叠加一个字符
 * argument：		yuvData[in/out]要要叠加的yuv数据 
 * 			width[in] yuv数据的宽
 * 			height[in]yuv数据的高
 * 			left[in]叠加位置的左边据
 * 			top[in[叠加位置的上边据
 * 			zoom[in]字体大小
 * 			groundType[in]是否添加背景 0无背景 1纯色背景 2点状背景 3 线状背景
			imagetype[in] 图像类型  0 UYVY 1 yuv420P
 * 返回值:
	0
 */

int blend_time_yuv(byte* yuvData, char num , int width, int height, int left, int top, int zoom,int groundType,int imagetype,blendInfo *myblend_info)
{
	int i,j,k,t;
	int iBase = 0;
	if(imagetype == IMAGE_UYVY)//UYVY一个像素占用两个紧挨着字节需要*2
	{
		left*=2;
		width*=2;
	}
	
	iBase = (top-1) * width + left;//422图像使用
	for(k = 0; k < strlen(m_timeStr); k ++)
	{		
		//根据字符找到点阵数据的子摸
		if(num == m_timeStr[k])
		{
			for(i = 0; i < ZK_HEIGHT * zoom; i ++)
			{
				t=0;//422图像使用
				for(j = 0; j < ZK_WIDTH/2 * zoom; j ++)
				{
					//显示字
					if(*((myblend_info->m_digitData[k]) + i * ZK_WIDTH/2 * zoom + j) ==  1)
					{
						if(imagetype == IMAGE_YUV420P)//420P
							yuv420PixelSet(yuvData,m_FontColor,left+j,top+i,width,height);
						else if (imagetype == IMAGE_UYVY)//UYVY
							YUV422SetFontPixel(iBase+t,yuvData+iBase + t);
					}else
					{
						//底色
						if(1==groundType )
						{
							if(imagetype == IMAGE_YUV420P)//420P
								yuv420PixelSet(yuvData,m_BordColor,left+j,top+i,width,height);
							else if (imagetype == IMAGE_UYVY)//UYVY
								YUV422SetGroundPixel(iBase + t,yuvData + iBase  + t);
						}else if(2 == groundType && i%2 == 0 && j%2 == 0)
						{
							if(imagetype == IMAGE_YUV420P)//420P
								yuv420PixelSet(yuvData,m_BordColor,left+j,top+i,width,height);
							else if (imagetype == IMAGE_UYVY)//UYVY
								YUV422SetGroundPixel(iBase + t,yuvData + iBase  + t);
						}else if(3 == groundType  && j%2 == 0)
						{	
							if(imagetype == IMAGE_YUV420P)//420P
								yuv420PixelSet(yuvData,m_BordColor,left+j,top+i,width,height);
							else if (imagetype == IMAGE_UYVY)//UYVY
								YUV422SetGroundPixel(iBase + t,yuvData + iBase  + t);	
						}
						//判断是否需要描边
						if(*((myblend_info->m_digitData[k]) + i * ZK_WIDTH/2 * zoom + j) ==  2)
						{
							if(imagetype == IMAGE_YUV420P)//420P
								yuv420PixelSet(yuvData,m_BordColor,left+j,top+i,width,height);
							else if (imagetype == IMAGE_UYVY)//UYVY
								YUV422SetGroundPixel(iBase + t,yuvData + iBase  + t);
						}
						
					}
					t= t+2;//UYVY因为一个像素两个字节 所以一次走两个字节
				}
				iBase += width;//叠加下一行
			}
		}
	}
	return 0;
}
/*
 * 函数名：blend_string_yuv
 * 功能：叠加静态字符串，一次叠加一行，叠加的数据在m_blend_mess里面存储好
 * 参数：iNum【in】要叠加的字符串在m_blend_mess中的下彪，
 * 		tmplen [in]要叠加的字符串的长度
 * 		iLeft[in]左边据单位：像素
 * 		iTop[in]上边据大　单位：像素
 * 		width [in]yuv的数据宽：单位像素
 * 		height[in]yuv的数据高；单位像素
 * 		data[in]点阵数据
 * 		pInBuf[in/out]yuv数据
 * 		zoom[in]字体大小
 * 		groundType[in]是否添加背景 0无背景 1纯色背景 2点状背景 3 线状背景
		imagetype[in] 图像类型  0 UYVY 1 yuv420P
		myblend_info [in]叠加结构体
 * 返回值:
	0
 */
int blend_string_yuv(int iNum,int tmpLen,int iLeft,int iTop,int width,int height,char *data,byte * pInBuf, int zoom,int groundType,int imagetype,blendInfo *myblend_info)
{
	int i,j,k;
	char *p = data;//点阵字库数据
	byte *yuvData = pInBuf;//yuv数据
	if(imagetype == IMAGE_UYVY)//UYVY一个像素占用两个紧挨着字节需要*2
	{
		iLeft*=2;
		width*=2;
	}
	int iBase,t; //422图像使用
	iBase = (iTop-1) * width + iLeft;
	
	int iOffset = 0;
	//显示文字
	for(k = 0; k < tmpLen;)
	{
		iBase = (iTop-1) * width + iLeft; //422图像使用
		if(myblend_info->m_blend_mess.strMess[iNum].string[k] >= 0xA1)
		{//汉字
			for(i = 0; i < ZK_HEIGHT * zoom; i ++)
			{
				t = 0; 	//422图像使用			
				for(j = 0; j < ZK_WIDTH * zoom; j ++)
				{
					if(*(p + i * ZK_WIDTH * zoom + j) ==  1)
					{	//字体
						if(imagetype == IMAGE_YUV420P)
							yuv420PixelSet(yuvData,m_FontColor,iLeft+j+iOffset,iTop+i,width,height);
						else if (imagetype == IMAGE_UYVY)
							YUV422SetFontPixel(iBase + iOffset + t,yuvData + iBase + iOffset*2 + t);

					}else
					{
						//底色
						if(1==groundType )
						{
								if(imagetype == IMAGE_YUV420P)
									yuv420PixelSet(yuvData,m_BordColor,iLeft+j+iOffset,iTop+i,width,height);
								else if (imagetype == IMAGE_UYVY)
									YUV422SetGroundPixel(iBase + iOffset + t,yuvData + iBase + iOffset*2 + t);
						}else if(2 == groundType && i%2 == 0 && j%2 == 0)
						{
								if(imagetype == IMAGE_YUV420P)
									yuv420PixelSet(yuvData,m_BordColor,iLeft+j+iOffset,iTop+i,width,height);
								else if (imagetype == IMAGE_UYVY)
									YUV422SetGroundPixel(iBase + iOffset + t,yuvData + iBase + iOffset*2 + t);
						}else if(3 == groundType  && j%2 == 0)
						{	
								if(imagetype == IMAGE_YUV420P)
									yuv420PixelSet(yuvData,m_BordColor,iLeft+j+iOffset,iTop+i,width,height);
								else if (imagetype == IMAGE_UYVY)
									YUV422SetGroundPixel(iBase + iOffset + t,yuvData + iBase + iOffset*2 + t);	
						}
						//判断是否需要描边
						if(*(p + i * ZK_WIDTH * zoom + j) ==  2)
						{
							if(imagetype == IMAGE_YUV420P)
								yuv420PixelSet(yuvData,m_BordColor,iLeft+j+iOffset,iTop+i,width,height);
							else if (imagetype == IMAGE_UYVY)
								YUV422SetGroundPixel(iBase + iOffset + t,yuvData + iBase + iOffset*2 + t);
						}
					}
					t = t+2;//UVVY数据一个像素两个字节
				}
				iBase += width;//UYVY 
			}
			iOffset += ZK_WIDTH *zoom;
			p += ZK_WIDTH*ZK_HEIGHT * zoom*zoom;
			k+=2;//下个字符 汉字占用两个字节
 
		}
		else
		{	//字母 字库的宽减半
			for(i = 0; i < ZK_HEIGHT * zoom; i ++)
			{
				t = 0;//UYVY
				for(j = 0; j < ZK_WIDTH/2 * zoom; j ++)
				{
					if(*(p + i * ZK_WIDTH/2 * zoom + j) ==  1)
					{	//字体
						if(imagetype == IMAGE_YUV420P)
							yuv420PixelSet(yuvData,m_FontColor,iLeft+j+iOffset,iTop+i,width,height);
						else if (imagetype == IMAGE_UYVY)
							YUV422SetFontPixel(iBase + iOffset + t,yuvData + iBase + iOffset*2 + t);
					}else
					{
						//底色
						if(1==groundType )
						{
								if(imagetype == IMAGE_YUV420P)
									yuv420PixelSet(yuvData,m_BordColor,iLeft+j+iOffset,iTop+i,width,height);
								else if (imagetype == IMAGE_UYVY)
									YUV422SetGroundPixel(iBase + iOffset + t,yuvData + iBase + iOffset*2 + t);
						}else if(2 == groundType && i%2 == 0 && j%2 == 0)
						{
								if(imagetype == IMAGE_YUV420P)
									yuv420PixelSet(yuvData,m_BordColor,iLeft+j+iOffset,iTop+i,width,height);
								else if (imagetype == IMAGE_UYVY)
									YUV422SetGroundPixel(iBase + iOffset + t,yuvData + iBase + iOffset*2 + t);
						}else if(3 == groundType  && j%2 == 0)
						{	
								if(imagetype == IMAGE_YUV420P)
									yuv420PixelSet(yuvData,m_BordColor,iLeft+j+iOffset,iTop+i,width,height);
								else if (imagetype == IMAGE_UYVY)
									YUV422SetGroundPixel(iBase + iOffset + t,yuvData + iBase + iOffset*2 + t);
						}
						//判断是否需要描边
						if(*(p + i * ZK_WIDTH/2 * zoom + j) ==  2)
						{
							if(imagetype == IMAGE_YUV420P)
								yuv420PixelSet(yuvData,m_BordColor,iLeft+j+iOffset,iTop+i,width,height);
							else if (imagetype == IMAGE_UYVY)
								YUV422SetGroundPixel(iBase + iOffset + t,yuvData + iBase + iOffset*2 + t);
						}
					}
					t =t+2;//UVVY数据一个像素两个字节
				}				
				iBase += width;		//UYVY数据用于定位像素
			}
			p += ZK_WIDTH/2*ZK_HEIGHT * zoom * zoom;
			iOffset += ZK_WIDTH/2 * zoom;
			k++;//下个字符
		}
	}
	return 0;
}


/*
 * 函数名：blend_begin_no_vdce
 * 函数功能：开始叠加
 * 参数： pInBuf[in/out]yuv数据
 * 		width【in】yuv数据的宽
 * 		height【in】yuv数据的高
 *		imageType [in] :0 UYVY 1 yuv420P　参照头文件中的宏定义
		myblend_info[in]:叠加信息结构体
 */
int blend_begin_no_vdce (byte * pInBuf, int width, int height,int imagetype,blendInfo *myblend_info)
{
	int i;
	int iLeft,iTop;
	int tmpLen,offsetLen;
	char timeStr[100] = {0};
	if(m_iReset == 1)
	{//重启静态叠加
		if(blend_init_no_vdce(myblend_info)==-1)
			return -1;
		m_iReset = 0;
	}
	if(myblend_info->m_blend_mess.time == 1)
	{//叠加时间
		GetTimeString(timeStr);
		for(i = 0; i < strlen(timeStr); i ++)
		{
			blend_time_yuv(pInBuf,timeStr[i], width, height, myblend_info->m_blend_mess.iTLeft + i * ZK_WIDTH/2 * myblend_info->m_blend_mess.iSize,myblend_info->m_blend_mess.iTTop, myblend_info->m_blend_mess.iSize,myblend_info->m_blend_mess.backGround,imagetype,myblend_info);

		}
	}
	offsetLen = 0;
	for(i=0;i<3;i++)
	{//叠加文本
		tmpLen = strlen((char *)myblend_info->m_blend_mess.strMess[i].string);
		iLeft = myblend_info->m_blend_mess.strMess[i].iLeft;
		iTop = myblend_info->m_blend_mess.strMess[i].iTop;
		offsetLen = myblend_info->iBitLen[i];
		if(tmpLen>0)
			blend_string_yuv(i,tmpLen,iLeft,iTop,width,height,myblend_info->m_blendData+offsetLen,pInBuf, myblend_info->m_blend_mess.iSize,myblend_info->m_blend_mess.backGround,imagetype,myblend_info);
	}
	return 0;
}

/*
 * 函数名： setFondColor
 * 函数功能：设置叠加字体的颜色
 * 参数：R[in]红色
 * 		G[in]绿
 * 		B[in]蓝
 */

void setFondColor(int R,int G,int B)
{

	if(R == 255 && G == 255 && B == 255)
	{		
		m_FontColor.Y = 255;
		m_FontColor.U = 128;
		m_FontColor.V = 128;
	}else
	{
		//白色转换失真使用上面值
		m_FontColor.Y = 0.257*R + 0.504*G + 0.098*B + 16;
		m_FontColor.U=  -0.148*R - 0.291*G + 0.439*B + 128;
		m_FontColor.V=  0.439*R - 0.368*G - 0.071*B + 128;
	}

}
/*
 * 函数名： setBordColor
 * 函数功能：设置叠加底色/边框的颜色（未使用）
 * 参数：R[in]红色
 * 		G[in]绿
 * 		B[in]蓝
 */
void setBordColor(int R,int G,int B)
{
	m_BordColor.Y = 0.257*R + 0.504*G + 0.098*B + 16;
	m_BordColor.U=  -0.148*R - 0.291*G + 0.439*B + 128;
	m_BordColor.V=  0.439*R - 0.368*G - 0.071*B + 128;
}
/*
 * 函数名：setblent
 * 函数功能：设置静态叠加信息
 * 参数：newblendinfo[in]静态叠加信息
	myblend_info [in]叠加信息结构题
 */
int setblent(blendInfo *myblend_info)
{
	if(blend_init_no_vdce(myblend_info)==-1)
		return -1;
	return 0;
}


/*
 * 函数名：ExitBlend
 * 函数功能：释放静态叠加申请的内存
 */

void ExitBlend(blendInfo *myblend_info)
{
	int i = 0;
	if(myblend_info->m_blendData != NULL)
	{
		free(myblend_info->m_blendData);
		myblend_info->m_blendData = NULL;
	}
	for(i = 0; i < strlen(m_timeStr); i ++)//初始化点阵存储空间
	{
		if(myblend_info->m_digitData[i] != NULL)
		{
			free(myblend_info->m_digitData[i]);
			myblend_info->m_digitData[i] = NULL;
		}
	}
	myblend_info->initFlag = 0;
}

