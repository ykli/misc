/*
 * IMP Encoder emulator header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#ifndef __IMP_EMU_FAKE_H__
#define __IMP_EMU_FAKE_H__

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

int IMP_EmuFakedev_CreateGroup(int fakedev_idx, int group_index,
							   void (*call_back)(IMPFrameInfo *));
int IMP_EmuFakedev_DestroyGroup(int fakedev_idx, int group_index);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_EMU_FAKE_H__ */
