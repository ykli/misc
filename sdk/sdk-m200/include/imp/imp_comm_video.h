/*
 * IMP common video func header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Justin <pengtao.kang@ingenic.com>
 */

#ifndef __IMP_COMM_VIDEO_H__
#define __IMP_COMM_VIDEO_H__

/**
 * @file
 * Video data type interface.
 */

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/**
 * 定义视频原始图像帧结构
 */
typedef struct {
    uint32_t          u32Width;			/**< 图像宽度 */
    uint32_t          u32Height;		/**< 图像高度 */
    uint32_t		  u32Field;			/**< 帧场模式 */
    uint32_t		  enPixelFormat;	/**< 视频图像像素格式 */

    uint32_t          u32PhyAddr[3];	/**< 物理地址 */
    void	         *pVirAddr[3];		/**< 虚拟地址 */
    uint32_t          u32Stride[3];		/**< 图像跨距 */

    uint16_t          u16OffsetTop;		/**< 图像顶部剪裁宽度 */
    uint16_t          u16OffsetBottom;	/**< 图像底部剪裁宽度 */
    uint16_t          u16OffsetLeft;    /**< 图像左侧剪裁宽度 */
    uint16_t          u16OffsetRight;   /**< 图像右侧剪裁宽度 */

    uint64_t          u64pts;			/**< 图像时间戳 */
    uint32_t          u32TimeRef;		/**< 图像帧序列号 */

    uint32_t          u32PrivateData;	/**< 私有数据 */
}IMPVideoFrameSt;

/**
 * 定义视频图像帧信息结构体
 */
typedef struct {
	IMPVideoFrameSt	stVFrame;	/**< 视频图像帧 */
	uint32_t		u32PoolId;		/**< 视频缓存池 ID */
	uint32_t		u32BlockId;	/**< 视频缓存块 ID */
} IMPVideoFrameInfoSt;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_COMM_VIDEO_H_ */
