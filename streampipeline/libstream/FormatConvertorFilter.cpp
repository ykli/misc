#include "FormatConvertorFilter.hh"
#include "jzmedia.h"

#define i_pref(hint,base,offset)										\
	({ __asm__ __volatile__("pref %0,%2(%1)"::"i"(hint),"r"(base),"i"(offset):"memory");})

FormatConvertorFilter* FormatConvertorFilter::createNew(int w, int h)
{
	return new FormatConvertorFilter(w, h);
}

FormatConvertorFilter::FormatConvertorFilter(int w, int h)
	: Filter("FormatConvertorFilter"), width(w), height(h)
{
	for (int i = 0; i < NR_BUFFERS; i++)
		buffer[i] = valloc(width * height * 3 / 2);
	curBuffer = buffer[0];
}

FormatConvertorFilter::~FormatConvertorFilter()
{
	for (int i = 0; i < NR_BUFFERS; i++)
		if(buffer[i] != NULL)
			free(buffer[i]);
}

static void cimvyuy_to_tile420(uint8_t* src_data,int srcwidth, int srcheight, uint8_t* dest,int start_mbrow, int mbrow_nums) {
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

void FormatConvertorFilter::doProcess(frame_t* frame_list, int nFrames, uint32_t *params)
{
	frame_t* frame = getFrameByType(frame_list, nFrames, PRIMARY);

	LOCATION("\n");
	cimvyuy_to_tile420((uint8_t*)frame->addr, width, height, (uint8_t*)curBuffer, 0, height / 16);
	frame->addr = curBuffer;
	if (curBuffer == buffer[0])
		curBuffer = buffer[1];
	else
		curBuffer = buffer[0];
}
