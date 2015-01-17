/*
 * IMP SVB (Shared Video Buffer) data structure and macro header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#ifndef __IMP_SVB_H__
#define __IMP_SVB_H__

/**
 * @file
 * Shared Video Buffer.
 */

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

typedef struct SVBConfig
{
    HI_U32 u32MaxPoolCnt;     /* max count of pools, (0,VB_MAX_POOLS]  */
    struct hiVB_CPOOL_S
    {
        HI_U32 u32BlkSize;
        HI_U32 u32BlkCnt;
        HI_CHAR acMmzName[MAX_MMZ_NAME_LEN];
    }astCommPool[VB_MAX_COMM_POOLS];
} SVBConfig;

/**
 * Creat memory pool.
 */
int IMP_SVB_CreatePool(uint32_t u32BlkSize, uint32_t u32BlkCnt);

/**
 * Destroy memory pool.
 */
int IMP_SVB_DestroyPool(int Pool);

/**
 * Get an SVB in memory pool.
 */
int IMP_SVB_GetBlock(int Pool, uint32_t u32BlkSize,const char *pcMmzName);

/**
 * Release an SVB in memory pool.
 */
int IMP_SVB_ReleaseBlock(int Block);

/**
 * Get the physical address of memory pool.
 */
uint32_t IMP_SVB_Handle2PhysAddr(int Block);

/**
 * Get the memory poll id of the block.
 */
int IMP_SVB_Handle2PoolId(int Block);

/**
 * Initialize the SVB module.
 */
int IMP_SVB_Init(void);

/**
 * Exit the SVB module.
 */
int IMP_SVB_Exit(void);

/**
 * IMP_SVB_MmapPool.
 */
int IMP_SVB_MmapPool(int Pool);

/**
 * IMP_SVB_MunmapPool.
 */
int IMP_SVB_MunmapPool(int Pool);

/**
 * IMP_SVB_GetBlkVirAddr.
 */
int IMP_SVB_GetBlkVirAddr(int Pool, uint32_t u32PhyAddr, void **ppVirAddr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_SVB_H__ */
