/*
 * IMP FrameSource header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: liangbohu <lbh@ingenic.cn>
 */

#ifndef __IMP_FRAMESOURCE_H__
#define __IMP_FRAMESOURCE_H__

#include "imp_common.h"

/**
 * @file
 * FrameSource模块头文件
 */

/**
 * @mainpage
 * IMP SDK.
 */

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

typedef struct {
	int picWidth;
	int picHeight;
	IMPPixelFormat pixFmt;
	int outFrmRate;
} IMPFSChnAttr;

typedef struct {
	int inFrmRate;
	int nrVBs;
} IMPFSDevAttr;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_FRAMESOURCE_H__ */
