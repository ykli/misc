/*
 * IMP FrameSource header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: liangbohu <lbh@ingenic.cn>
 */

#ifndef __IMP_FRAMESOURCE_H__
#define __IMP_FRAMESOURCE_H__

/**
 * @file
 * FrameSource模块头文件
 */

/**
 * @defgroup IMP_FrameSource
 * @brief IMP FrameSource -----------------TODO.
 * @{
 */

#include "imp_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * 通道属性结构体
 */

typedef struct {
	int picWidth;			/* < 图片宽度 */
	int picHeight;			/* < 图片高度 */
	IMPPixelFormat pixFmt;		/* < 图片格式 */
	int outFrmRate;			/* < 通道的输出帧率 */
} IMPFSChnAttr;

/**
 * 设备属性结构体
 */

typedef struct {
	int inFrmRate;			/* < 设备的输出帧率 */
	int nrVBs;			/* < 缓存buf数量 */
} IMPFSDevAttr;

/**
 * 设置设备属性
 *
 * @fn int IMP_FrameSource_SetDevAttr(IMPFSDevAttr *dev_attr);
 *
 * @param[in] dev_attr 设备属性指针
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 *
 * @remark 可以设置摄像头的相关属性，包括：输出帧率，缓存buf数量.
 *
 * @note 无
 */

int IMP_FrameSource_SetDevAttr(IMPFSDevAttr *dev_attr);

/**
 * 获得设备属性
 *
 * @fn int IMP_FrameSource_GetDevAttr(IMPFSDevAttr *dev_attr);
 *
 * @param[out] dev_attr 设备属性指针
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 *
 * @remark 可以获得摄像头的相关属性，包括：输出帧率，缓存buf数量.
 *
 * @note 调用这个函数之前，必须先调用IMP_FrameSource_SetDevAttr这个函数
 */

int IMP_FrameSource_GetDevAttr(IMPFSDevAttr *dev_attr);

/**
 * 设置通道属性
 *
 * @fn int IMP_FrameSource_SetChnAttr(uint32_t chn_num, IMPFSChnAttr *chn_attr);
 *
 * @param[in] chn_num 设置第几路通道
 *
 * @param[in] chn_attr 通道属性指针
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 *
 * @remark 可以设置通道的相关属性，包括：图片宽度，图片高度，图片格式，通道的输出帧率
 *
 * @note 无
 */

int IMP_FrameSource_SetChnAttr(uint32_t chn_num, IMPFSChnAttr *chn_attr);

/**
 * 获得通道属性
 *
 * @fn int IMP_FrameSource_GetChnAttr(uint32_t chn_num, IMPFSChnAttr *chn_attr);
 *
 * @param[in] chn_num 获得第几路通道
 *
 * @param[out] chn_attr 通道属性指针
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 *
 * @remark 可以获得通道的相关属性，包括：图片宽度，图片高度，图片格式，通道的输出帧率
 *
 * @note 无
 */

int IMP_FrameSource_GetChnAttr(uint32_t chn_num, IMPFSChnAttr *chn_attr);

/**
 * 启动通道
 *
 * @fn int IMP_FrameSource_EnableChn(uint32_t chn_num);
 *
 * @param[in] chn_num 启动第几路通道
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 *
 * @remark 无
 *
 * @note 在使用这个函数之前，一定要先调用IMP_FrameSource_SetChnAttr这个函数.
 */

int IMP_FrameSource_EnableChn(uint32_t chn_num);

/**
 * 关闭通道
 *
 * @fn int IMP_FrameSource_DisableChn(uint32_t chn_num);
 *
 * @param[in] chn_num 停止第几路通道
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 *
 * @remark 无
 *
 * @note 无
 */

int IMP_FrameSource_DisableChn(uint32_t chn_num);

/**
 * 启动设备
 *
 * @fn int IMP_FrameSource_EnableDev(void);
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 *
 * @remark 无
 *
 * @note 在使用这个函数之前，一定要先调用IMP_FrameSource_SetDevAttr这个函数.
 */

int IMP_FrameSource_EnableDev(void);

/**
 * 关闭sensor
 *
 * @fn int IMP_FrameSource_DisableDev(void);
 *
 * @param 无
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 *
 * @remark 无
 *
 * @note 在使用这个函数之前，一定要先调用IMP_FrameSource_EnableDev这个函数.
 */

int IMP_FrameSource_DisableDev(void);

/**
 * 打开数据流
 *
 * @fn IMP_FrameSource_StreamOn(void);
 *
 * @param 无
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 *
 * @remark 启动设备的数据，给后端模块提供数据源
 *
 * @note 在使用这个函数之前，一定要先调用IMP_FrameSource_EnableDev和IMP_FrameSource_EnableChn函数
 */

int IMP_FrameSource_StreamOn(void);

/**
 * 关闭数据流
 *
 * @fn IMP_FrameSource_StreamOff(void);
 *
 * @param 无
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 *
 * @remark 停止设备数据的发送
 *
 * @note 在使用这个函数之前，一定要先调用IMP_FrameSource_StreamOn函数.
 */

int IMP_FrameSource_StreamOff(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

/**
 * @}
 */

#endif /* __IMP_FRAMESOURCE_H__ */
