/*
 * IMP System header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#ifndef __IMP_SYSTEM_H__
#define __IMP_SYSTEM_H__

#include "imp_common.h"

/**
 * @file
 * IMP系统模块头文件
 */

/**
 * @defgroup System
 * @brief System control -----------------TODO. 
 * @{
 */

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * @fn int IMP_System_Init(void)
 *
 * IMP系统初始化.
 *
 * @param 无.
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 此API调用后会初始化基础的数据结构，但不会初始化硬件单元.
 *
 * @note 在IMP的任何操作之前必须先调用此接口进行初始化.
 */
int IMP_System_Init(void);

/**
 * @fn int IMP_System_Exit(void)
 *
 * IMP系统去初始化.
 *
 * @param 无.
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 此函数调用后会释放IMP所有的内存以及句柄，并关闭硬件单元.
 *
 * @note 在调用此API后，若要再次使用IMP则需重新进行IMP系统初始化.
 */
int IMP_System_Exit(void);

/**
 * @fn uint64_t IMP_System_GetTimeStamp(void)
 *
 * 获得IMP系统的时间戳，单位为微秒。
 *
 * @param 无。
 *
 * @retval 时间(usec)
 *
 * @remarks 系统初始化后时间戳自动被初始化。系统去初始化后时间戳失效。
 *
 * @note 无。
 */
uint64_t IMP_System_GetTimeStamp(void);

/**
 * @fn void IMP_System_RebaseTimeStamp(uint64_t basets)
 *
 * 设置IMP系统的时间戳，单位为微秒。
 *
 * @param[in] basets 基础时间。
 *
 * @retval 无
 *
 * @remarks 无。
 *
 * @note 无。
 */
void IMP_System_RebaseTimeStamp(uint64_t basets);

/**
 * @fn uint32_t IMP_System_ReadReg32(uint32_t u32Addr)
 *
 * 读取32位寄存器的值。
 *
 * @param[in] regAddr 寄存器的物理地址。
 *
 * @retval 寄存器的值（32位）
 *
 * @remarks 无。
 *
 * @note 无。
 */
uint32_t IMP_System_ReadReg32(uint32_t regAddr);

/**
 * @fn void IMP_System_WriteReg32(uint32_t regAddr, uint32_t value)
 *
 * 向32位寄存器中写值。
 *
 * @param[in] regAddr 寄存器的物理地址。
 * @param[in] value 要写入的值。
 *
 * @retval 无
 *
 * @remarks 无。
 *
 * @note 在不明确寄存器的含义之前请谨慎调用此API，否则可能会导致系统错误。
 */
void IMP_System_WriteReg32(uint32_t regAddr, uint32_t value);

/**
 * @fn int IMP_System_Bind(IMPChannel *srcChn, IMPChannel *dstChn)
 *
 * 绑定源通道和目的组.
 *
 * @param[in] srcChn 源通道指针.
 * @param[in] dstChn 目的组指针.
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 根据设备、组和通道的概念，每个设备可能有多个组，每个组可能有多个通道，
 * 组作为设备的输入接口，而通道作为设备的输出接口.因此绑定实际上是将输出设备的某
 * 个通道连接到输入设备的某个通道组上.
 * @remarks 绑定关系成功后，源通道产生的数据会自动传送到目的组上.
 *
 * @note 绑定不成功会返回错误值（这只是个例子实际当然不会这么写~）.
 */
int IMP_System_Bind(IMPChannel *srcChn, IMPChannel *dstChn);

/**
 * @fn int IMP_System_UnBind(IMPChannel *srcChn, IMPChannel *dstChn)
 *
 * 解除源通道和目的组的绑定.
 *
 * @param[in] srcChn 源通道指针.
 * @param[in] dstChn 目的组指针.
 *
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 无.
 *
 * @note 无。
 */
int IMP_System_UnBind(IMPChannel *srcChn, IMPChannel *dstChn);

/**
 * @fn IMP_System_GetBindbyDest(IMPChannel *dstChn, IMPChannel *srcChn)
 *
 * 获取绑定在目的通道的源通道信息.
 *
 * @param[in] srcChn 源通道指针.
 * @param[in] dstChn 目的组指针.
 *
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 无.
 *
 * @note 无。
 */
int IMP_System_GetBindbyDest(IMPChannel *dstChn, IMPChannel *srcChn);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_SYSTEM_H__ */
