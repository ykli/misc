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

SUKeyEvent SU_Key_GetEvent(int gpio_num);

int SU_LED_Command(int gpio_num, SULedCmd cmd);

int SU_Lumina_GetValue(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SU_MISC_H__ */
