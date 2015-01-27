/*
 * Misc utils header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author:
 */

#ifndef __SU_MISC_H__
#define __SU_MISC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * @file
 * Sysutils 其他功能头文件
 */

/**
 * @defgroup Sysutils其他功能
 * @brief Sysutils Misc -----------------TODO.
 * @{
 */

/**
 * 按键事件.
 */
typedef enum {
	KEY_RELEASED,	/**< 按键抬起 */
	KEY_PRESSED,	/**< 按键按下 */
} SUKeyEvent;

/**
 * LED行为命令.
 */
typedef enum {
	LED_ON,			/**< LED打开 */
	LED_OFF,		/**< LED关闭 */
} SULedCmd;

/**
 * @fn int SU_Key_GetEvent(int gpio_num, SUKeyEvent *event)
 *
 * 获取按键事件.
 *
 * @param[in] gpio_num GPIO号.
 * @param[out] event 按键事件指针.
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 该函数阻塞，直到有按键事件发生返回.
 *
 * @note 无。
 */
int SU_Key_GetEvent(int gpio_num, SUKeyEvent *event);

/**
 * @fn int SU_LED_Command(int gpio_num, SULedCmd *cmd)
 *
 * 发送LED命令.
 *
 * @param[in] gpio_num GPIO号.
 * @param[in] cmd LED行为命令.
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 无.
 *
 * @note 无。
 */
int SU_LED_Command(int gpio_num, SULedCmd *cmd);

/**
 * @fn int SU_Lumina_GetValue(int *value)
 *
 * 获取光感值.
 *
 * @param[out] value 光感值.
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 无.
 *
 * @note 获取的光感值可能由于板级的不同而不同，应用需要根据实际板级加入修正值。
 */
int SU_Lumina_GetValue(int *value);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SU_MISC_H__ */
