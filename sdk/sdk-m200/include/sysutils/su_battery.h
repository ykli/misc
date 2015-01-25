/*
 * Battery utils header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: 
 */

#ifndef __SU_BATTERY_H__
#define __SU_BATTERY_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

typedef enum {
	Charging = 0,
	Full,
	xxx,
} SUBatStatus;

SUBatStatus SU_Battery_GetStatus(void);
SUBatEvent SU_Battery_GetEvent(void);
int SU_Battery_GetCapacity(void);

#endif
#endif /* __cplusplus */

#endif /* __SU_BATTERY_H__ */
