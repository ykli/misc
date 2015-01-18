/*
 * IMP Encoder emulator header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#ifndef __IMP_EMU_ENC_H__
#define __IMP_EMU_ENC_H__

/**
 * @file
 * Encoder emulator func interface.
 */

#include <stdint.h>
#include <imp_common.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef struct {
  int picWidth;
  int picHeight;
  IMP_PixelFormat pixFmt;
  int inFrmRate;
  int outFrmRate;
} IMP_FS_ChnAttr;

int IMP_EmuFrameSource_SetChnAttr(uint32_t chn_num, IMP_FS_ChnAttr *chn_attr);
int IMP_EmuFrameSource_GetChnAttr(uint32_t chn_num, IMP_FS_ChnAttr *chn_attr);
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

#endif /* __IMP_EMU_ENC_H__ */
