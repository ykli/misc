/*
 * IMP Encoder emulator header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#ifndef __IMP_EMU_FRAMESOURCE_H__
#define __IMP_EMU_FRAMESOURCE_H__

/**
 * @file
 * Encoder emulator func interface.
 */

#include <stdint.h>
#include <linux/videodev2.h>

#include <imp/imp_common.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

static inline int imppixfmt_to_v4l2pixfmt(IMPPixelFormat imp_pixfmt)
{
	int v4l2_pixfmt = 0;
	switch (imp_pixfmt) {
	case PIX_FMT_NV12: v4l2_pixfmt = V4L2_PIX_FMT_NV12; break;
	case PIX_FMT_YUYV422: v4l2_pixfmt = V4L2_PIX_FMT_YUYV; break;
	default: v4l2_pixfmt = -1; break;
	}
	return v4l2_pixfmt;
}

static inline int calc_pic_size(int width, int height, IMPPixelFormat imp_pixfmt)
{
	int bpp1 = 0, bpp2 = 1,size;

#define BPP(FMT, A, B) case FMT: bpp1 = A;bpp2 = B;break
	switch (imp_pixfmt) {
		BPP(PIX_FMT_NV12, 3, 2);
		BPP(PIX_FMT_YUYV422, 2, 1);
	default: break;
	}
#undef BPP
	size = width * height * bpp1 / bpp2;

	return size;
}

int IMP_EmuFrameSource_SetDevAttr(IMPFSDevAttr *dev_attr);
int IMP_EmuFrameSource_GetDevAttr(IMPFSDevAttr *dev_attr);
int IMP_EmuFrameSource_SetChnAttr(uint32_t chn_num, IMPFSChnAttr *chn_attr);
int IMP_EmuFrameSource_GetChnAttr(uint32_t chn_num, IMPFSChnAttr *chn_attr);
int IMP_EmuFrameSource_EnableChn(uint32_t chn_num);
int IMP_EmuFrameSource_DisableChn(uint32_t chn_num);
int IMP_EmuFrameSource_EnableDev(void);
int IMP_EmuFrameSource_DisableDev(void);
int IMP_EmuFrameSource_StreamOn(void);
int IMP_EmuFrameSource_StreamOff(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_EMU_FRAMESOURCE_H__ */
