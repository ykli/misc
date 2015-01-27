/*
 * System utils header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author:
 */

#ifndef __SU_BASE_H__
#define __SU_BASE_H__

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * @file
 * Sysutils 基础功能头文件
 */

/**
 * @defgroup Sysutils基础功能
 * @brief Sysutils base -----------------TODO. 
 * @{
 */

#define DEVID_LENGTH 64

/**
 * 设备ID.设备ID为唯一值，不同的CPU芯片间的值有差异
 */
typedef union {
	char chr[DEVID_LENGTH];		/**< 设备ID字符串 */
	uint8_t hex[DEVID_LENGTH];	/**< 设备ID二进制 */
} SUDevID;

/**
 * 系统时间结构体.
 */
typedef struct {
	int sec;	/**< 秒数，范围：0~59 */
	int min;	/**< 分钟数，范围：0~59 */
	int hour;	/**< 小时数，范围：0~23 */
	int mday;	/**< 一个月中的第几天，范围：1~31 */
	int mon;	/**< 月份，范围：1~12 */
	int year;	/**< 年份，范围：>1900 */
} SUTime;

/**
 * @fn int SU_Base_GetDevID(SUDevID *devID)
 *
 * 获取设备ID.
 *
 * @param[in] devID 设备ID结构体指针.
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 每颗CPU芯片的设备ID是唯一的.
 *
 * @note 无.
 */
int SU_Base_GetDevID(SUDevID *devID);

/**
 * @fn int SU_Base_GetTime(SUTime *time)
 *
 * 获得系统时间.
 *
 * @param[in] time 系统时间结构体指针.
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 无.
 *
 * @note 无.
 */
int SU_Base_GetTime(SUTime *time);

/**
 * @fn int SU_Base_SetTime(SUTime *time)
 *
 * 设置系统时间.
 *
 * @param[out] time 系统时间结构体指针.
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 无.
 *
 * @note 系统时间参数需在合理范围，否则函数调用失败.
 */
int SU_Base_SetTime(SUTime *time);

/**
 * @fn int SU_Base_Shutdown(void)
 *
 * 设备关机.
 *
 * @param 无.
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 调用该函数后设备会立即关机并关闭主电源.
 *
 * @note 在调用此函数之前请确保已保存所有文件.
 */
int SU_Base_Shutdown(void);

/**
 * @fn int SU_Base_Reboot(void)
 *
 * 设备重启.
 *
 * @param 无.
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 调用该函数后设备会立即重启.
 *
 * @note 在调用此函数之前请确保已保存所有文件.
 */
int SU_Base_Reboot(void);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SU_BASE_H__ */
