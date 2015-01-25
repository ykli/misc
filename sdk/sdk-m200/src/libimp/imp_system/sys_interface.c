/*
 * Ingenic IMP System Control module implement.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <stdio.h>

#include <imp/imp_constraints.h>
#include <imp/imp_log.h>
#include <imp/imp_system.h>

#include <system/module.h>
#include <system/group.h>
#include <system/system.h>

#define TAG "System"

int IMP_System_Init(void)
{
	IMP_LOG_DBG(TAG, "%s\n", __func__);
	return system_init();
}

int IMP_System_Exit(void)
{
	IMP_LOG_DBG(TAG, "%s\n", __func__);	return 0;
	return system_exit();
}

uint64_t IMP_System_GetTimeStamp(void)
{
	IMP_LOG_DBG(TAG, "%s TODO...\n", __func__);	return 0;
}
void IMP_System_RebaseTimeStamp(uint64_t basets)
{
	IMP_LOG_DBG(TAG, "%s TODO...\n", __func__);
}

uint32_t IMP_System_ReadReg32(uint32_t regAddr)
{
	IMP_LOG_DBG(TAG, "%s TODO...\n", __func__);	return 0;
}

void IMP_System_WriteReg32(uint32_t regAddr, uint32_t value)
{
	IMP_LOG_DBG(TAG, "%s TODO...\n", __func__);
}

int IMP_System_Bind(IMPChannel *srcChn, IMPChannel *dstChn)
{
	if (srcChn == NULL) {
		IMP_LOG_ERR(TAG, "%s(): src channel is NULL\n", __func__);
		return -1;
	}
	if (dstChn == NULL) {
		IMP_LOG_ERR(TAG, "%s(): dst channel is NULL\n", __func__);
		return -1;
	}

	return system_bind(srcChn, dstChn);
}

int IMP_System_UnBind(IMPChannel *srcChn, IMPChannel *dstChn)
{
	if (srcChn == NULL) {
		IMP_LOG_ERR(TAG, "%s(): src channel is NULL\n", __func__);
		return -1;
	}
	if (dstChn == NULL) {
		IMP_LOG_ERR(TAG, "%s(): dst channel is NULL\n", __func__);
		return -1;
	}

	return system_unbind(srcChn, dstChn);
}

int IMP_System_GetBindbyDest(IMPChannel *dstChn, IMPChannel *srcChn)
{
	if (dstChn == NULL) {
		IMP_LOG_ERR(TAG, "%s(): dst channel is NULL\n", __func__);
		return -1;
	}

	if (srcChn == NULL) {
		IMP_LOG_ERR(TAG, "%s(): src channel is NULL\n", __func__);
		return -1;
	}

	return system_get_bind_src(dstChn, srcChn);
}
