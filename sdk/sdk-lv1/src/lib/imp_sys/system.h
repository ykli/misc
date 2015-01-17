/*
 * IMP system implement function header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <imp_sys.h>

int system_init(void);
int system_exit(void);
int system_bind(IMPChannel *pstSrcChn, IMPChannel *pstDestChn);
int system_unbind(IMPChannel *pstSrcChn, IMPChannel *pstDestChn);

#endif /* __SYSTEM_H__ */
