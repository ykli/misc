/**
 * IMP ISP header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: liangbohu <lbh@ingenic.cn>
 */

#ifndef __IMP_ISP_H__
#define __IMP_ISP_H__

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
 * 设置sensor帧率
 *
 * @fn int IMP_ISP_SetImageAttr(const IMP_ISP_Image_Attr_t attr);
 *
 * @param[in] attr 设置sensor帧率
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 */

int IMP_ISP_SetImageAttr(const IMP_ISP_Image_Attr_t attr);

/**
 * 获得sensor帧率
 *
 * @fn int IMP_ISP_GetImageAttr(IMP_ISP_Image_Attr_t *attr);
 *
 * @param[out] attr 获得sensor帧率
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 */

int IMP_ISP_GetImageAttr(IMP_ISP_Image_Attr_t *attr);

/**
 * 初始化ISP模块
 *
 * @fn int IMP_ISP_Init(void );
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 */

int IMP_ISP_Init(void );

/**
 * 启动ISP模块
 *
 * @fn int IMP_ISP_Run(void );
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 */

int IMP_ISP_Run(void );

/**
 * 停止ISP模块
 *
 * @fn int IMP_ISP_Exit(void );
 *
 * @retval 0 成功
 *
 * @retval 非0 失败，返回错误码
 */

int IMP_ISP_Exit(void );

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_ISP_H__ */
