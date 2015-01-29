/*
 * IMP Encoder emulator header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#ifndef __IMP_EMU_ENCODER_H__
#define __IMP_EMU_ENCODER_H__

/**
 * @file
 * Encoder emulator func interface.
 */

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef int IMPEncoderAttr;
typedef int IMPEncoderRcAttr;

typedef struct {
	IMPEncoderAttr		encAttr;		/**< 编码器属性结构体 */
	IMPEncoderRcAttr	encRcAttr;		/**< 码率控制器属性结构体 */
} IMPEmuEncoderCHNAttr;

typedef struct {
	void *buf;
	int length;
	uint64_t timestamp;
} EmuStreamBuffer;

int IMP_EmuEncoder_CreateGroup(int groupNum);
int IMP_EmuEncoder_DestroyGroup(int groupNum);

int IMP_EmuEncoder_CreateChn(int chnNum, IMPEmuEncoderCHNAttr *chnAttr);
int IMP_EmuEncoder_DestroyChn(int chnNum);

int IMP_EmuEncoder_RegisterChn(int groupNum, int chnNum);
int IMP_EmuEncoder_UnregisterChn(int chnNum);

int IMP_EmuEncoder_GetStream(int chnNum, EmuStreamBuffer *stream, int blockFlag);
int IMP_EmuEncoder_ReleaseStream(int chnNum, EmuStreamBuffer *stream);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_EMU_ENCODER_H__ */
