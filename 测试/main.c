#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./include/blend.h"
#include "time.h"

#define MAXSIZE 1024*1024*3
unsigned char strtm[200];

blend_mess m_new_blend_mess_channelx =
{
	.font = 3,
	.strMess =
	{
	{"English", 50, 100},
	{"12345678", 0, 150},
	{"ÖÐÎÄ", 0, 200},
	},
	.time = 1,
	.iTLeft = 50,
	.iTTop = 20,
	.iSize =2
};

int main()
{
	FILE *fp;
	int size = 0;
	unsigned char buff[MAXSIZE],temp;
	memset(buff,0,MAXSIZE);
	unsigned char *s=buff;
	unsigned char * str = strtm;
	int i = 0;
	
	size = 0;
	temp = 0;
	fp=fopen("1.yuv","rb");
	if (fp==NULL)
	{
		printf("fopen error\n");
		return 1;
	}
	while ((temp=fread(s,1,1,fp))!=0)
	{
			size++;
			s++;
	};
	printf("fread over \n");
	fclose(fp);

	/////////////////???????//////////////////////////////////////////

	struct timeval tpstart1,tpend1;
	float timeuse1;
	if(blend_init_no_vdce(0) == -1)
	{
		printf("blend_init_no_vdce error\n");
		return -1;
	}
	sleep(2);
	setblent(m_new_blend_mess_channelx);

	setFondColor(255,255,255);
	setBordColor(0,0,0);

	gettimeofday(&tpstart1,NULL);
	blend_begin_no_vdce(buff,1280,720,IMAGE_YUV420P);
	gettimeofday(&tpend1,NULL);
	timeuse1=1000000*(tpend1.tv_sec-tpstart1.tv_sec)+tpend1.tv_usec-tpstart1.tv_usec;
	printf("blend use time %f\n",timeuse1);

	ExitBlend();
	int t1=0;
	fp=fopen("./blendOSD.yuv","wb");
	if (fp==NULL)
	{
		printf("./blendOSD.yuv open error \n");
		return 1;
	}
	t1 = fwrite(buff,1,size,fp);
	fclose(fp);
///////////////////////////////////////////////////////////////////////////
	printf("OVER\n");
	return 0;
}
