/*
 * IMP system implement function header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <imp/imp_common.h>
#include <imp/imp_system.h>

#define FAKEDEV_MAX 8

enum {
	DEV_ID_EMU_FS = DEV_ID_RESERVED_START,		/**< 模拟视频源 */
	DEV_ID_EMU_ENC,								/**< 模拟编码器 */
	DEV_ID_EMU_OSD,								/**< OSD */
#define DEV_ID_EMU_FAKE(n) (DEV_ID_EMU_FAKE_START + n)
	DEV_ID_EMU_FAKE_START,
	DEV_ID_EMU_FAKE_END = DEV_ID_EMU_FAKE(FAKEDEV_MAX),
#define NR_FAKE_DEV (FAKEDEV_MAX + 1)
};

int system_init(void);
int system_exit(void);
int system_bind(IMPChannel *srcChn, IMPChannel *dstChn);
int system_unbind(IMPChannel *srcChn, IMPChannel *dstChn);
int system_get_bind_src(IMPChannel *dstChn, IMPChannel *srcChn);
int system_attach(IMPChannel *fromChn, IMPChannel *toChn);
int system_detach(IMPChannel *fromChn, IMPChannel *toChn);
int system_bind_dump(void);

#endif /* __SYSTEM_H__ */
