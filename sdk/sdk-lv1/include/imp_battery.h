/*
 * IMP battery API header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#ifndef __IMP_BATTERY_H__
#define __IMP_BATTERY_H__

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
 * Initialize the IMP system.
 */
int IMP_Battery_GetCapacity(void);
int IMP_Battery_GetChargeStatus(void);
int IMP_Battery_GetBatteryEvent(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_BATTERY_H__ */
