/*
 * Ingenic IMP System Control module implement.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <stdio.h>
#include <imp_log.h>
#include <imp_sys.h>
#include <system/module.h>
#include <system/constraints.h>
#include <system/group.h>

#include "system.h"

#define TAG "System"

int IMP_SYS_Init(void)
{
	IMP_LOG_DBG(TAG, "%s\n", __func__);
	return system_init();
}

int IMP_SYS_Exit(void)
{
	IMP_LOG_DBG(TAG, "%s\n", __func__);	return 0;
	return system_exit();
}

void *IMP_SYS_Mmap(uint32_t paddr, uint32_t size)
{
	IMP_LOG_DBG(TAG, "%s\n", __func__);	return NULL;
	return NULL;
}

int IMP_SYS_Munmap(void *vaddr, uint32_t size)
{
	IMP_LOG_DBG(TAG, "%s\n", __func__);
	return -1;
}

int IMP_SYS_Bind(IMPChannel *pstSrcChn, IMPChannel *pstDestChn)
{
	return system_bind(pstSrcChn, pstDestChn);
}

int IMP_SYS_UnBind(IMPChannel *pstSrcChn, IMPChannel *pstDestChn)
{
	return system_unbind(pstSrcChn, pstDestChn);
}
