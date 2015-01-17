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

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

int IMP_EmuEncoder_CreateGroup(int group_num);
int IMP_EmuEncoder_DestroyGroup(int group_num);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_EMU_ENC_H__ */
