/*
 * IMP Video input header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: liangbohu <lbh@ingenic.cn>
 */

#ifndef __IMP_VI_H__
#define __IMP_VI_H__

#include "imp_common.h"

/**
 * @file
 * A sample for doxyfile of SDK-level1.
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

/**
 * 启动sensor
 *
 * @fn IMP_VI_EnableDev(void );
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 */

int IMP_VI_EnableDev(void );

/**
 * 关闭sensor
 *
 * @fn IMP_VI_DisableDev(void );
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 */

int IMP_VI_DisableDev(void );

/**
 * 设置VI通道属性
 *
 * @fn IMP_VI_SetChnAttr(int chn_num, const IMP_VI_Chn_Attr_t attr);
 *
 * @param[in] chn_num 设置第几路通道
 *
 * @param[in] attr 设置具体通道属性
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 */

int IMP_VI_SetChnAttr(int chn_num, const IMP_VI_Chn_Attr_t attr);

/**
 * 获得VI通道属性
 *
 * @fn IMP_VI_GetChnAttr(int chn_num, IMP_VI_Chn_Attr_t *attr);
 *
 * @param[in] chn_num 获得第几路通道
 *
 * @param[out] attr 获得具体通道属性
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 */

int IMP_VI_GetChnAttr(int chn_num, IMP_VI_Chn_Attr_t *attr);

/**
 * 启动VI通道
 *
 * @fn IMP_VI_EnableChn(int chn_num);
 *
 * @param[in] chn_num 启动第几路通道
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 */

int IMP_VI_EnableChn(int chn_num);

/**
 * 关闭VI通道
 *
 * @fn IMP_VI_DisableChn(int chn_num);
 *
 * @param[in] chn_num 停止第几路通道
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 */

int IMP_VI_DisableChn(int chn_num);

/**
 * 设置缓存帧数
 *
 * @fn int IMP_VI_SetFrameDepth(int chn_num, int frame_depth);
 *
 * @param[in] chn_num 缓存第几路通道
 *
 * @param[in] frame_depth 设置缓存帧数
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 */

int IMP_VI_SetFrameDepth(int chn_num, int frame_depth);

/**
 * 获得缓存帧数
 *
 * @fn int IMP_VI_GetFrameDepth(int chn_num, int *frame_depth);
 *
 * @param[in] chn_num 缓存第几路通道
 *
 * @param[out] frame_depth 获得缓存帧数
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 */

int IMP_VI_GetFrameDepth(int chn_num, int *frame_depth);

/**
 * 获得缓存帧
 *
 * @fn int IMP_VI_GetFrame(int chn_num, IMP_VI_Frame_t * frame);
 *
 * @param[in] chn_num 缓存第几路通道
 *
 * @param[in] frame 获得缓存帧
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 */

int IMP_VI_GetFrame(int chn_num, IMP_VI_Frame_t * frame);

/**
 * 带有timeout的获得缓存帧
 *
 * @fn int IMP_VI_GetFrameTimeOut(int chn_num, IMP_VI_Frame_t * frame, int ms);
 *
 * @param[in] chn_num 缓存第几路通道
 *
 * @param[in] frame 获得缓存帧
 *
 * @param[in] ms timeout的时间，单位毫秒
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 */

int IMP_VI_GetFrameTimeOut(int chn_num, IMP_VI_Frame_t * frame, int ms);

/**
 * 释放缓存帧
 *
 * @fn int IMP_VI_ReleaseFrame(int chn_num, IMP_VI_Frame_t * frame);
 *
 * @param[in] chn_num 缓存第几路通道
 *
 * @param[in] frame 释放缓存帧
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 */

int IMP_VI_ReleaseFrame(int chn_num, IMP_VI_Frame_t * frame);

/**
 * 获得VI模块的文件描述符
 *
 * @fn int IMP_VI_GetFd(void );
 *
 * @return 返回文件描述符
 *
 * @retval >0 成功
 *
 * @retval <0 失败，返回错误码
 */

int IMP_VI_GetFd(void );

/**
 * 获得VI通道状态
 *
 * @fn int IMP_VI_Query(IMP_VI_Stat_t *stat);
 *
 * @param[out] stat 获得VI通道状态
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 */

int IMP_VI_Query(IMP_VI_Stat_t *stat);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_VI_H__ */
