#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "FormatConvertor.hh"
#include "jzmedia.h"

#define DUAL_THREAD
#define i_pref(hint,base,offset)										\
	({ __asm__ __volatile__("pref %0,%2(%1)"::"i"(hint),"r"(base),"i"(offset):"memory");})

void cimvyuy_to_tile420(uint8_t* src_data,int srcwidth, int srcheight, uint8_t* dest,int start_mbrow, int mbrow_nums) {
	int width = srcwidth;
	int height = srcheight;

	if (srcwidth % 16){
		printf("%s: srcwidth is not >=16 aligned\n",__FUNCTION__);
		return;
	}
	if (srcheight % 16){
	    height = ((srcheight+15)>>4)<<4;
	}

	if(((int)dest)%32){
		printf("%s: dest not 32 aligned(%p), optimization may cause error\n",__FUNCTION__, dest);
		return;
	}

	int y_size = width * height;
	unsigned char * dest_y = dest-4+(start_mbrow*width*16);
	unsigned char * dest_u = dest-4 + y_size + (start_mbrow*width*8);
	unsigned char * src = (unsigned char *)(src_data-2*width + (start_mbrow*width*16*2));//-2*width for mxu loop 

	//cimvyuy = 0xVY1UY0
	int mbrow=0;
	for(mbrow=0; mbrow<mbrow_nums; mbrow++){
		int mbcol=0;
		for(mbcol=0; mbcol<(width>>4); mbcol++){
			unsigned char* src_temp= src;
			//per loop handle two rows, odd rows uv will be not used
			int i = 0;
			for (i = 0; i < 8; i++) {

				i_pref(30, dest_y, 4);
				if(!(i&1))
					i_pref(30, dest_u, 4);

				//even row, uv will be used
				S32LDIV(xr1,src_temp,width,1);
				S32LDD(xr2,src_temp,4);
				S32LDD(xr3,src_temp,8);
				S32LDD(xr4,src_temp,12);
				S32LDD(xr5,src_temp,16);
				S32LDD(xr6,src_temp,20);
				S32LDD(xr7,src_temp,24);
				S32LDD(xr8,src_temp,28);

				S32SFL(xr2,xr2,xr1,xr1,1);//xr2=0xV1 U1 V0 U0, xr1=0xY3 Y2 Y1 Y0
				S32SFL(xr4,xr4,xr3,xr3,1);//xr4=0xV3 U3 V2 U2, xr3=0xY7 Y6 Y5 Y4
				S32SFL(xr4,xr4,xr2,xr2,1);//xr4=0xV3 V2 V1 V0, xr2=0xU3 U2 U1 U0
				S32SFL(xr6,xr6,xr5,xr5,1);//xr6=0xV5 U5 V4 U4, xr5=0xY11 Y10 Y9 Y8
				S32SFL(xr8,xr8,xr7,xr7,1);//xr8=0xV7 U7 V6 U6, xr7=0xY15 Y14 Y13 Y12
				S32SFL(xr8,xr8,xr6,xr6,1);//xr8=0xV7 V6 V5 V4, xr6=0xU7 U6 U5 U4

				S32SDI(xr1,dest_y,4);
				S32SDI(xr3,dest_y,4);
				S32SDI(xr5,dest_y,4);
				S32SDI(xr7,dest_y,4);
				S32SDI(xr2,dest_u,4);//u0-u3
				S32SDI(xr6,dest_u,4);//u4-u7
				S32SDI(xr4,dest_u,4);//v0-v3
				S32SDI(xr8,dest_u,4);//v4-v7

				//odd row, uv will be discarded
				S32LDIV(xr1,src_temp,width,1);
				S32LDD(xr2,src_temp,4);
				S32LDD(xr3,src_temp,8);
				S32LDD(xr4,src_temp,12);
				S32LDD(xr5,src_temp,16);
				S32LDD(xr6,src_temp,20);
				S32LDD(xr7,src_temp,24);
				S32LDD(xr8,src_temp,28);

				S32SFL(xr2,xr2,xr1,xr1,1);//xr2=0xV1 U1 V0 U0, xr1=0xY3 Y2 Y1 Y0
				S32SFL(xr4,xr4,xr3,xr3,1);//xr4=0xV3 U3 V2 U2, xr3=0xY7 Y6 Y5 Y4
				S32SFL(xr6,xr6,xr5,xr5,1);//xr6=0xV5 U5 V4 U4, xr5=0xY11 Y10 Y9 Y8
				S32SFL(xr8,xr8,xr7,xr7,1);//xr8=0xV7 U7 V6 U6, xr7=0xY15 Y14 Y13 Y12

				S32SDI(xr1,dest_y,4);
				S32SDI(xr3,dest_y,4);
				S32SDI(xr5,dest_y,4);
				S32SDI(xr7,dest_y,4);
			}
			src+=32;//src proceed to next MB
		}
		src += 15*2*width;//src proceed to next mbrow
	}
}

void* convertor_thread(void *p) {
	FormatConvertor *fc = (FormatConvertor *)p;

	fc->convertor_thread1();

	return NULL;
}

void FormatConvertor::convertor_thread1() {
	while (thread_flag) {
		sem_wait(&sem_start);
		cimvyuy_to_tile420(args.src, args.width, args.height, args.dst, args.start_mbrow, args.nr_mbrow);
		sem_post(&sem_end);
	}
}

FormatConvertor::FormatConvertor(void) : thread_flag(1) {
#ifdef DUAL_THREAD
	int error;

	sem_init(&sem_start, 0, 0);
	sem_init(&sem_end, 0, 0);

	error = pthread_create(&tid, NULL, convertor_thread, this);
	if (error) {
		printf("thread create error\n");
	}
#endif
}

FormatConvertor::~FormatConvertor()  {
#ifdef DUAL_THREAD
	thread_flag = 0;
	pthread_join(tid, NULL);

	sem_destroy(&sem_start);
	sem_destroy(&sem_end);
#endif
}


void FormatConvertor::yuv422_to_tile420(uint8_t* src, uint8_t* dst, int width, int height)
{
#ifdef DUAL_THREAD
	args.src = src;
	args.width = width;
	args.height = height;
	args.dst = dst;
	args.start_mbrow = 0;
	args.nr_mbrow = height/16 / 2;

	sem_post(&sem_start);
	cimvyuy_to_tile420(src, width, height, dst, (height/16)/2, ((height+15)/16) - (height/16)/2);
	sem_wait(&sem_end);

#else
	cimvyuy_to_tile420(src, width, height, dst, 0, height / 16);
#endif
}

void FormatConvertor::tile420_to_grey(uint8_t* src, uint8_t* dest, int width, int height) {

	char *y_p = (char*)dest;

	char *y_t = (char*)(src);

	int i_w, j_h;
	int y_w_mcu = width>>4;
	int y_h_mcu = height>>4;

	int i_mcu;

	char *y_mcu = y_p;
	char *y_tmp = y_p;

	char *y_t_tmp = y_t;
	for(j_h = 0; j_h < y_h_mcu; j_h++)
	{
		y_mcu = y_p + j_h*y_w_mcu*256;
		for(i_w = 0; i_w < y_w_mcu; i_w++)
		{
			y_tmp = y_mcu;
			for(i_mcu = 0; i_mcu < 16; i_mcu++)
			{
				memcpy(y_tmp, y_t_tmp, 16);
				y_t_tmp += 16;
				y_tmp += width;
			}
			y_mcu += 16;
		}
	}
}
