/*
 * System utils header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: 
 */

#ifndef __SU_SYS_H__
#define __SU_SYS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

int SU_SYS_GetDevID(SUDevID *devID);

int SU_SYS_GetTime(SUTime *time);
int SU_SYS_SetTime(SUTime *time);

int SU_SYS_Shutdown(void);
int SU_SYS_Reboot(void);

#endif
#endif /* __cplusplus */

#endif /* __SU_SYS_H__ */
