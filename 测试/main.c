#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "./include/blend.h"
#include "time.h"

#define MAXSIZE 1024*1024*3
unsigned char strtm[200];

blend_mess m_new_blend_mess_channelx1 =
{
	.font = 3,
	.strMess =
	{
	{"English", 0, 100},
	{"1234567890", 0, 200},
	{"中文", 0, 300},
	},
	.time = 1,
	.iTLeft = 50,
	.iTTop = 20,
	.iSize =4,
	.isAddEdge = 0,
	.backGround = 2
};


blend_mess m_new_blend_mess_channelx2 =
{
	.font = 3,
	.strMess =
	{
	{"tom ?", 0, 100},
	{"jerry ?", 0, 200},
	{"中文", 0, 300},
	},
	.time = 1,
	.iTLeft = 50,
	.iTTop = 20,
	.iSize =3,
	.isAddEdge = 0,
	.backGround = 1
};

void * fuc1()
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

	blendInfo myblendInfo;


	fp=fopen("1.yuv","rb");
	if (fp==NULL)
	{
		printf("fopen error\n");
		return 0;
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
	int count = 0;
	char filename[20] = {0};
	if(blend_init_no_vdce(&myblendInfo) == -1)
	{
		printf("blend_init_no_vdce error\n");
		return 0;
	}
	sleep(2);

	while(1)
	{
		count++;
		myblendInfo.m_blend_mess = m_new_blend_mess_channelx1;
		setblent(&myblendInfo);
		setFondColor(255,255,255);
		setBordColor(0,0,0);

		gettimeofday(&tpstart1,NULL);
		blend_begin_no_vdce(buff,1280,960,IMAGE_YUV420P,&myblendInfo);

		gettimeofday(&tpend1,NULL);
		timeuse1=1000000*(tpend1.tv_sec-tpstart1.tv_sec)+tpend1.tv_usec-tpstart1.tv_usec;
		printf("blend use time %f\n",timeuse1);

		sprintf(filename,"./fuc1_%d.yuv",count);
		int t1=0;
		
		fp=fopen(filename,"wb");
		if (fp==NULL)
		{
			printf("%s open error \n",filename);
			return 0;
		}
		t1 = fwrite(buff,1,size,fp);
		fclose(fp);
		usleep(10000);
	}

	ExitBlend(&myblendInfo);
	return 0;
}

void * fuc2()
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

	blendInfo myblendInfo;


	fp=fopen("1.yuv","rb");
	if (fp==NULL)
	{
		printf("fopen error\n");
		return 0;
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
	int count = 0;
	char filename[20] = {0};
	if(blend_init_no_vdce(&myblendInfo) == -1)
	{
		printf("blend_init_no_vdce error\n");
		return 0;
	}
	sleep(2);

	while(1)
	{
		count++;
		myblendInfo.m_blend_mess = m_new_blend_mess_channelx2;
		setblent(&myblendInfo);
		setFondColor(255,255,255);
		setBordColor(0,0,0);

		gettimeofday(&tpstart1,NULL);
		blend_begin_no_vdce(buff,1280,960,IMAGE_YUV420P,&myblendInfo);

		gettimeofday(&tpend1,NULL);
		timeuse1=1000000*(tpend1.tv_sec-tpstart1.tv_sec)+tpend1.tv_usec-tpstart1.tv_usec;
		printf("blend use time %f\n",timeuse1);

		sprintf(filename,"./fuc2_%d.yuv",count);
		int t1=0;
		
		fp=fopen(filename,"wb");
		if (fp==NULL)
		{
			printf("%s open error \n",filename);
			return 0;
		}
		t1 = fwrite(buff,1,size,fp);
		fclose(fp);
		usleep(10000);
	}

	ExitBlend(&myblendInfo);
	return 0;
}

int main()
{
	pthread_t ThrPid_fuc1;
	pthread_t ThrPid_fuc2;

	pthread_create(&ThrPid_fuc1,NULL,fuc1,NULL);
	pthread_create(&ThrPid_fuc2,NULL,fuc2,NULL);
	
	pthread_join(ThrPid_fuc1,NULL);
	pthread_join(ThrPid_fuc2,NULL);




	return 0;
}
