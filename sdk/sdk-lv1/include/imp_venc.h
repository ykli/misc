/*
 * IMP venc func header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Justin <pengtao.kang@ingenic.com>
 */

#ifndef __IMP_VENC_H__
#define __IMP_VENC_H__

#include "imp_comm_rc.h"
#include "imp_comm_venc.h"
#include "imp_comm_video.h"

/**
 * @file
 * venc func interface.
 */

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/**
 * IMP_VENC_CreateGroup 创建编码通道组
 *
 * @fn int IMP_VENC_CreateGroup(VENC_GRP VeGroup)
 *
 * @param[in] VeGroup 通道组号,取值范围:[0, VENC_MAX_GRP_NUM)
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 编码通道组指芯片能够同时处理的编码通道的集合
 * @remarks 一路通道组仅支持一路分辨率，不同分辨率需启动新的通道组
 * @remarks 一路通道组同时支持一路H264和一路JPEG抓拍
 * @remarks 次码流重建一个通道组，不启用次通道
 *
 * @note 如果指定的通道组已经存在，则返回失败
 */
int IMP_VENC_CreateGroup(VENC_GRP VeGroup);

/**
 * IMP_VENC_DestroyGroup 销毁编码通道组.
 *
 * @fn int IMP_VENC_DestroyGroup(VENC_GRP VeGroup)
 *
 * @param[in] VeGroup 通道组号,取值范围:[0, VENC_MAX_GRP_NUM)
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 销毁通道组时，必须保证通道组为空，即没有任何通道在通道组中注册，或注册到通道组中
 * 的通道已经反注册，否则返回失败
 *
 * @note 销毁并不存在的通道组，则返回失败
 */
int IMP_VENC_DestroyGroup(VENC_GRP VeGroup);

/**
 * IMP_VENC_CreateChn 创建编码通道
 *
 * @fn int IMP_VENC_CreateChn(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstAttr)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] pstAttr 编码通道属性指针
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 编码通道属性由两部分组成，编码器属性和码率控制属性
 * @remarks 编码器属性首先需要选择编码协议，然后分别对各种协议对应的属性进行赋值
 * @remarks 编码器属性不支持次码流，必须将编码器码流类型设置为主码流类型
 * @remarks 编码器属性不支持场编码模式，必须将通道编码类型设置为帧编码模式
 * @remarks 编码器属性最大宽高，通道宽高必须满足如下约束：
 *
 *		MaxPicWidth∈[MIN_WIDTH, MAX_WIDTH]
 *
 *		MaxPicHeight∈[MIN_HEIGHT, MAX_HEIGHT]
 *
 *		PicWidth∈[MIN_WIDTH, MaxPicwidth]
 *
 *		PicHeight∈[MIN_HEIGHT, MaxPicHeight]
 *
 *		最大宽高,通道宽高必须是 MIN_ALIGN 的整数倍
 *
 * @remarks 其中MIN_WIDTH、MAX_WIDTH、MIN_HEIGHT、MAX_HEIGHT、MIN_ALIGN分别表示编码通道支持的
 * 最小宽度,最大宽度,最小高度,最大高度,最小对齐单元(像素)
 * @remarks 编码器属性必须设置编码码流buffer深度,获取码流方式等
 * @remarks 当输入图像大小不大于编码通道编码图像最大宽高时，才能启动编码通道进行编码
 * @remarks 编码器属性中所有项都是静态属性，一旦创建编码通道成功，静态属性不支持被修改, 除非该
 * 通道被销毁，重新创建
 * @remarks 码率控制器属性首先需要配置RC模式，JPEG抓拍通道不需要配置码率控制器属性，H264协议
 * 通道都必须配置，码率控制器属性RC模式必须与编码器属性协议类型匹配
 * @remarks H264码率控制器支持四种模式:CBR，VBR，ABR, FIXQP。并且对于不同协议，相同RC模式的
 * 属性变量基本一致
 * @remarks ViFrmRate应该设置为输入编码器的实际帧率，RC需要根据ViFrmRate统计实际帧率以及进行
 * 码率控制。假设 VI 的输出帧率是 30，如果 GROUP 不进行帧率控制，ViFrmRate 应该设置为30,如果
 * GROUP 进行帧率控制,ViFrmRate 应该设置为 GROUP 进行帧率控制后的输出帧率。设置目标帧率
 * TargetFrmRate 时,目标帧率类型定义为分数类型uint32_t类型，高 16 位用于表示分母，
 * 低 16 位表示分子
 * @remarks CBR 除了上述的属性之外,还需要设置平均比特率和波动等级。平均比特率的单位是 kbps,
 * 平均比特率的设置与编码通道宽高以及图像帧率都有关系；波动等级设置分为 6 档,波动等级越大，
 * 系统允许码率的波动范围更大
 * @remarks FIXQP 除了上述属性之外,还需要设置 IQp,PQp； 为了减少呼吸效应，推荐 I 帧 QP 始终比
 * P 帧的 QP 小 2~3
 */
int IMP_VENC_CreateChn(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstAttr);

/**
 * IMP_VENC_DestroyChn 销毁编码通道
 *
 * @fn int IMP_VENC_DestroyChn(VENC_CHN VeChn)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @note 销毁并不存在的通道，则返回失败
 * @note 销毁前必须保证通道已经从通道组反注册，否则返回失败
 */
int IMP_VENC_DestroyChn(VENC_CHN VeChn);

/**
 * IMP_VENC_RegisterChn 注册编码通道到通道组
 *
 * @fn int IMP_VENC_RegisterChn(VENC_GRP VeGroup, VENC_CHN VeChn)
 *
 * @param[in] VeGroup 编码通道组号,取值范围: [0, VENC_MAX_GRP_NUM)
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @note 注册并不存在的通道，则返回失败
 * @note 注册通道到不存在的通道组，否则返回失败
 * @note 同一个编码通道只能注册到一个通道组，如果该通道已经注册到某个通道组，则返回失败
 * @note 如果一个通道组已经被注册，那么这个通道组就不能再被其他的通道注册，除非之前注册
 * 关系被解除
 */
int IMP_VENC_RegisterChn(VENC_GRP VeGroup, VENC_CHN VeChn );

/**
 * IMP_VENC_UnRegisterChn 反注册编码通道到通道组
 *
 * @fn int IMP_VENC_UnRegisterChn(VENC_CHN VeChn)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 通道注销之后，编码通道会被复位，编码通道里的码流buffer都会被清空，如果用户还在使用
 * 未及时释放的码流buffer，将不能保证buffer数据的正确性，用户可以使用IMP_VENC_Query接口来查询编
 * 码通道码流buffer状态，确认码流buffer里的码流取完之后再反注册通道
 *
 * @note 注销未创建的通道，则返回失败
 * @note 注销未注册的通道，则返回失败
 * @note 如果编码通道未停止接收图像编码，则返回失败
 */
int IMP_VENC_UnRegisterChn(VENC_CHN VeChn);

/**
 * IMP_VENC_StartRecvPic 开启编码通道接收图像
 *
 * @fn int IMP_VENC_StartRecvPic(VENC_CHN VeChn)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 此接口不判断当前是否已经开启接收，即允许重复开启不返回错误
 * @remarks 开始接收输入是针对通道的，只有开启接收之后编码器才能开始接收图像编码
 *
 * @note 如果通道未创建，则返回失败
 * @note 如果通道没有注册到通道组，则返回失败
 */
int IMP_VENC_StartRecvPic(VENC_CHN VeChn);

/**
 * IMP_VENC_StartRecvPicEx开启编码通道接收图像,超出指定的帧数后自动停止接收图像
 *
 * @fn int IMP_VENC_StartRecvPicEx(VENC_CHN VeChn, VENC_RECV_PIC_PARAM_S *pstRecvParam)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] pstRecvParam 接收图像参数结构体指针，用于指定需要接收图像的帧数
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 如果通道已经调用了IMP_VENC_StartRecvPic开始接收图像而没有停止接收图像，或者上次调用
 * IMP_VENC_StartRecvPicEx后还没有接收到足够的图像，再次调用此接口不允许
 *
 * @note 如果通道未创建，则返回失败
 * @note 如果通道没有注册到通道组，则返回失败
 */
int IMP_VENC_StartRecvPicEx(VENC_CHN VeChn, VENC_RECV_PIC_PARAM_S *pstRecvParam);

/**
 * IMP_VENC_StopRecvPic 停止编码通道接收图像
 *
 * @fn int IMP_VENC_StopRecvPic(VENC_CHN VeChn)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 此接口并不判断当前是否停止接收，即允许重复停止接收不返回错误
 * @remarks 此接口用于编码通道停止接收图像来编码，编码通道反注册前必须停止接收图像
 * @remarks 调用此接口仅停止接收原始数据编码，码流buffer并不会被消除
 *
 * @note 如果通道未创建，则返回失败
 * @note 如果通道没有注册到通道组，则返回失败
 */
int IMP_VENC_StopRecvPic(VENC_CHN VeChn);

/**
 * IMP_VENC_Query 查询编码通道状态
 *
 * @fn int IMP_VENC_Query(VENC_CHN VeChn, VENC_CHN_STAT_S *pstStat)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[out] pstStat 编码通道的状态指针
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 此接口用于查询此函数调用时刻的编码器状态，pststat包含五个主要的信息：
 *
 *		bRegistered 表示当前编码通道的注册状态;
 *
 *		u32LeftPics 表示待编码帧个数；反注册通道前，可以通过查询是否还有图像待编码来
 *		决定反注册的时机，防止反注册时将可能需要编码的帧清理出去；
 *
 *		u32LeftStreamBytes 表示码流 buffer 中剩余的 byte数目；在反注册通道前 可以通过
 *		查询是否还有码流没有被处理来决定反注册时机；
 *
 *		u32LeftStreamFrames 表示码流 buffer 中剩余的帧数目；在反注册通道前,可以通过
 *		查询是否还有图像的码流没有被取走来决定反注册时机,防止反注册时将可能需要的码流
 *		清理出去；
 *
 *		u32CurPacks 表示当前帧的码流包个数。在调用HI_MPI_VENC_GetStream 之前应确保
 *
 *		u32CurPacks 大于 0；在按包获取时当前帧可能不是一个完整帧(被取走一部分),按帧获
 *		取时表示当前一个完整帧的包个数(如果没有一帧数据则为 0)。用户在需要按帧获取码
 *		流时,需要查询一个完整帧的包个数,在这种情况下,通常可以在select 成功后执行query
 *		操作,此时u32CurPacks是当前完整帧中包的个数。
 *
 *		u32LeftRecvPics 表示调用IMP_VENC_StartRecvPicEx 接口后剩余等待接收的帧数目；
 *
 *		u32LeftEncPics 表示调用IMP_VENC_StartRecvPicEx 接口后剩余等待编码的帧数目；
 *
 * @remarks 如果没有调用IMP_VENC_StartRecvPicEx,u32LeftRecvPics 和u32LeftEncPics 数目始终为0
 *
 * @note 如果通道未创建，则返回失败
 */
int IMP_VENC_Query(VENC_CHN VeChn, VENC_CHN_STAT_S *pstStat);

/**
 * IMP_VENC_SetChnAttr 设置编码通道属性
 *
 * @fn int IMP_VENC_SetChnAttr(VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstAttr)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] pstAttr 编码通道的属性指针
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 编码通道属性包括了编码器属性和码率控制器属性两部分；
 * @remarks 编码通道属性分为动态属性和静态属性两种，其中，动态属性的属性值在通道创建时配置，
 * 在通道销毁之前可以被修改，静态属性在通道创建时配置，在通道创建后不能被修改；
 * @remarks 编码通道的宽、高、最大宽、最大高、主次码流、编码协议、编码的帧场模式、输入图像的
 * 帧场模式、获取码流的方式等均属于静态属性。
 *
 * @note 如果pstAttr为NULL,则返回失败；
 * @note 设置未创建的通道的属性，则返回失败。
 */
int IMP_VENC_SetChnAttr( VENC_CHN VeChn, const VENC_CHN_ATTR_S *pstAttr);

/**
 * IMP_VENC_GetChnAttr 获取编码通道属性
 *
 * @fn int IMP_VENC_GetChnAttr( VENC_CHN VeChn, VENC_CHN_ATTR_S *pstAttr)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[out] pstAttr 编码通道的属性指针
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @note 如果pstAttr为NULL,则返回失败；
 * @note 获取未创建的通道的属性，则返回失败。
 */
int IMP_VENC_GetChnAttr( VENC_CHN VeChn, VENC_CHN_ATTR_S *pstAttr);

/**
 * IMP_VENC_GetStream 获取编码的码流
 *
 * @fn int IMP_VENC_GetStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream, bool bBlockFlag)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[out] pstStream 码流结构体指针
 * @param[in] bBlockFlag 是否使用阻塞方式获取，0：非阻塞，1：阻塞
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 支持阻塞或非阻塞两种方式获取，支持select/poll调用：
 *
 *		非阻塞获取时，如果缓冲无数据，则返回失败；
 *
 *		阻塞获取时，若果缓冲无数据，则会等待有数据时才返回获取成功。
 *
 * @remarks 支持按包或按帧方式获取码流，如果按包获取，则：
 *
 *		对于H264编码协议，每次获取的是一个NAL单元；
 *
 *		对JPEG编码协议，每次获取的是一个ECS或图像参数码流包；
 *
 * @remarks 码流结构体VENC_STREAM_S包含4个部分：
 *
 *		pstPack 码流包信息指针，指向一组VENC_PACK_S的内存空间,该空间由调用者分配。如果是
 *		按包获取,则此空间不小于 sizeof(VENC_PACK_S)的大小; 如果按帧获取,则此空间不小于
 *		N × sizeof(VENC_PACK_S)的大小,其中 N 代表当前帧之中的包的个数,可以在select 之后
 *		通过查询接口获得；
 *
 *		u32PackCount码流包个数，在输入时,此值指定pstPack中VENC_PACK_S的个数。按包获取时,
 *		u32PackCount 必须不小于 1;按帧获取时,u32PackCount 必须不小于当前帧的包个数。在函数
 *		调用成功后,u32PackCount 返回实际填充 pstPack 的包的个数；
 *
 *		u32Seq序列号,按帧获取时是帧序列号;按包获取时为包序列号；
 *
 *		码流特征信息,数据类型为联合体,包含了不同编码协议对应的码流特征信息stH264Info/
 *		stJpegInfo,码流特征信息的输出用于支持用户的上层应用；
 *
 * @remarks 如果用户长时间不获取码流,码流缓冲区就会满。一个编码通道如果发生码流缓冲区满,就会把后
 * 面接收的图像丢掉,直到用户获取码流,从而有足够的码流缓冲可以用于编码时,才开始继续编码。建议用户
 * 获取码流接口调用与释放码流的接口调用成对出现,且尽快释放码流,防止出现由于用户态获取码流,释放不
 * 及时而导致的码流 buffer 满,停止编码。
 *
 * @note 如果pstStream为NULL,则返回失败；
 * @note 如果通道未创建，则返回失败；
 */
int IMP_VENC_GetStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream, bool bBlockFlag);

/**
 * IMP_VENC_ReleaseStream 释放码流缓存
 *
 * @fn int IMP_VENC_ReleaseStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] pstStream 码流结构体指针
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 此接口应当和IMP_VENC_GetStream配对起来使用，用户获取码流后必须及时释放已经
 * 获取的码流缓存，否则可能会导致码流buffer满，影响编码器编码，并且用户必须按先获取先
 * 释放的顺序释放已经获取的码流缓存；
 * @remarks 在编码通道反注册后，所有未释放的码流包均无效，不能再使用或者释放这部分无效的码流缓存。
 *
 * @note 如果pstStream为NULL,则返回失败；
 * @note 如果通道未创建，则返回失败；
 * @note 释放无效的码流会返回失败。
 */
int IMP_VENC_ReleaseStream(VENC_CHN VeChn, VENC_STREAM_S *pstStream);

/**
 * IMP_VENC_InsertUserData 插入用户数据
 *
 * @fn int IMP_VENC_InsertUserData(VENC_CHN VeChn, uint8_t *pu8Data, uint32_t u32Len)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] pu8Data 用户数据指针
 * @param[in] u32Len 用户数据长度
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 只支持H264和JPEG协议；
 * @remarks 向H264中插入用户数据时建议用户在启动编码器后再操作，因为如果用户在编码器启动
 * 之前插入用户数据,编码器编出的码流就会出现以下状况:在码流的第一个 SPS 包之前会出现若干
 * 个 SEI 包。很多解码器遇到这种码流会把第一个SPS包之前的SEI包丢掉,造成用户丢失存储在这些
 * SEI包里的用户数据；
 * @remarks H.264 协议通道最多同时分配4块内存空间用于缓存用户数据,且每段用户数据大小不超过
 * 1kbyte。如果用户插入的数据多余4块,或插入的一段用户数据大于1kbyte 时,此接口会返回错误。
 * 每段用户数据以SEI包的形式被插入到最新的图像码流包之前。在某段用户数据包被编码发送之后,
 * H.264通道内缓存这段用户数据的内存空间被清零,用于存放新的用户数据。
 * @remarks JPEG协议通道最多分配1块内存空间,用于缓存1Kbyte的用户数据。用户数据以 APP segment
 * (0xFFEF)形式添加到图像码流中。在用户数据被编码发送之后,JPEG通道内缓存这段用户数据的内存空
 * 间被清零,用于存放新的用户数据。
 *
 * @note 如果pu8data为NULL,则返回失败；
 * @note 如果通道未创建，则返回失败；
 */
int IMP_VENC_InsertUserData(VENC_CHN VeChn, uint8_t *pu8Data, uint32_t u32Len);

/**
 * IMP_VENC_SendFrame 支持用户发送原始图像进行编码
 *
 * @fn int IMP_VENC_SendFrame(VENC_GRP VeGroup, VIDEO_FRAME_INFO_S *pstFrame)
 *
 * @param[in] VeGroup 编码通道组号,取值范围: [0, VENC_MAX_GRP_NUM)
 * @param[in] pstFrame 原始图像信息结构指针
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 此接口支持用户发送图像到编码通道组，图像格式必须为NV12，图像大小必须
 * 不小于编码通道大小
 *
 * @note 此接口不支持用户发送图像直方图至编码通道组
 */
int IMP_VENC_SendFrame(VENC_GRP VeGroup, VIDEO_FRAME_INFO_S *pstFrame);

/**
 * IMP_VENC_SetMaxStreamCnt 设置码流最大缓存帧数
 *
 * @fn int IMP_VENC_SetMaxStreamCnt(VENC_CHN VeChn, uint32_t u32MaxStrmCnt)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] u32MaxStrmCnt 最大码流缓存帧数
 *
 * @retval 零 成功.
 * @retval 小于零 失败, 见错误码.
 *
 * @remarks 此接口用于设置编码通道的码流 buffer 中能够缓存的最大码流帧数,若缓存码流帧数
 * 已达到最大码流帧数,当前待编码图像会被丢弃；最大码流帧数在创建通道时由系统内部指定默认
 * 值,默认值为3；
 * @remarks 此接口在创建通道之后,销毁编码通道之前均可以被调用。在下一帧开始编码之前生效。
 * 此接口允许被多次调用。建议在创建编码通道成功之后,启动编码前进行设置,不建议在编码过程
 * 中动态调整。
 *
 */
int IMP_VENC_SetMaxStreamCnt(VENC_CHN VeChn, uint32_t u32MaxStrmCnt);

/**
 * IMP_VENC_GetMaxStreamCnt 获取码流最大缓存帧数
 *
 * @fn int IMP_VENC_GetMaxStreamCnt(VENC_CHN VeChn, uint32_t *pu32MaxStrmCnt)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[out] u32MaxStrmCnt 最大码流缓存帧数
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 */
int IMP_VENC_GetMaxStreamCnt(VENC_CHN VeChn, uint32_t *pu32MaxStrmCnt);

/**
 * IMP_VENC_RequestIDR 请求I帧
 *
 * @fn int IMP_VENC_RequestIDR(VENC_CHN VeChn)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 接受IDR帧或I帧请求后,在尽可能短的时间内编出IDR帧或I帧；I 帧请求,只支持H.264编码协议
 *
 * @note 如果通道未创建，则返回失败
 */
int IMP_VENC_RequestIDR(VENC_CHN VeChn);

/**
 * IMP_VENC_GetFd 获取编码通道对应的设备文件句柄
 *
 * @fn int IMP_VENC_GetFd(VENC_CHN VeChn)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 *
 * @retval 正数值 有效返回值
 * @retval 小于零 无效返回值
 *
 */
int IMP_VENC_GetFd(VENC_CHN VeChn);

/**
 * IMP_VENC_SetRoiCfg 设置H.264通道的ROI属性
 *
 * @fn int IMP_VENC_SetRoiCfg(VENC_CHN VeChn, VENC_ROI_CFG_S *pstVencRoiCfg)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] pstVencRoiCfg ROI区域参数
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks ROI参数主要由5个参数决定：
 *
 *		u32Index:系统支持每个通道可设置 8 个 ROI 区域,系统内部按照 0~7 的索引号对ROI区域进行
 *		管理,u32Index 表示的用户设置ROI的索引号。ROI区域之间可以互相叠加,且当发生叠加时,ROI区
 *		域之间的优先级按照索引号0~7依次提高；
 *
 *		bEnable:指定当前的ROI区域是否使能；
 *
 *		bAbsQp:指定当前的ROI区域采用绝对QP方式或是相对QP；
 *
 *		s32Qp:当bAbsQp为true时,s32Qp表示ROI区域内部的所有宏块采用的QP值,当bAbsQp为false时,
 *		s32Qp表示ROI区域内部的所有宏块采用的相对QP值；
 *
 *		stRect:指定当前的ROI区域的位置坐标和区域的大小。ROI 区域的起始点坐标必须在图像范围内,
 *		且必须16对齐;ROI 区域的长宽必须是16对齐;ROI区域必须在图像范围内；
 *
 * @remarks 本接口属于高级接口,系统默认没有ROI区域使能,用户必须调用此接口启动ROI；
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前设置。此接口在编码过程中被调用时,
 * 等到下一个帧时生效；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数；
 * @remarks 建议用户在调用此接口之前,先调用IMP_VENC_GetRoiCfg接口,获取当前通道的ROI配置,
 * 然后再进行设置。
 *
 */
int IMP_VENC_SetRoiCfg(VENC_CHN VeChn, VENC_ROI_CFG_S *pstVencRoiCfg);

/**
 * IMP_VENC_SetRoiCfg 获取H.264通道的ROI配置属性
 *
 * @fn int IMP_VENC_GetRoiCfg(VENC_CHN VeChn, uint32_t u32Index,
 * VENC_ROI_CFG_S *pstVencRoiCfg)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] u32Index H.264协议编码通道ROI区域索引
 * @param[out] pstVencRoiCfg 对应的ROI区域配置参数
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前调用；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数。
 */
int IMP_VENC_GetRoiCfg(VENC_CHN VeChn, uint32_t u32Index, VENC_ROI_CFG_S *pstVencRoiCfg);

/**
 * IMP_VENC_SetRoiCfg 设置H.264通道的slice分割属性.
 *
 * @int IMP_VENC_SetH264SliceSplit(VENC_CHN VeChn,
 *		const VENC_PARAM_H264_SLICE_SPLIT_S *pstSliceSplit)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM).
 *
 * @param[in] pstSliceSplit H.264码流slice分割参数.
 *
 * @return 返回值.
 *
 * @retval 零 成功.
 *
 * @retval 小于零 失败, 见错误码.
 *
 * @remarks Slice 分割属性主要由三个参数决定：
 * bSplitEnable：当前帧是否进行 slice 分割；
 * u32SplitMode：slice 分割模式,u32SplitMode = 0 表示按照 bit 数来进行分割,
 * u32SplitMode = 1 表示按照宏块行进行分割；u32SplitMode>=2 不被支持，当前
 * Ingenic仅支持按宏块进行分割；
 * u32SplitMode 必须在 bSplitEnable 为真的条件下有效,否则,无效；
 * u32SliceSize：slice 的大小,当u32SplitMode不同时,u32SliceSize表示不同的意义；
 * 当u32SplitMode=0时,u32SliceSize表示平均每个slice的bit数的多少,编码器从图像的
 * 左上角开始,从左至右,从上至下,按光栅顺序按宏块进行编码。每个 slice 大小按照
 * u32SliceSize 进行划分。最终编码 slice 大小不一定与用户设置的 u32SliceSize
 * 严格相等,存在少许的误差量,且实际的偏移量总是正数。当编码至图像的最后一个 slice
 * 时,剩余的宏块 bit 数不足u32SliceSize,剩余的所有宏块被编码成一个 slice；
 * 当u32SplitMode = 1 时,u32SliceSize 表示每个 slice 占图像宏块行数,u32SliceSize
 * 的单位为宏块行。且当编码至图像的最后几行,不足 u32SliceSize 时,剩余的宏块行被划
 * 分为一个slice；
 * 本接口属于高级接口,用户可以选择性调用,建议不调用,系统默认 bSplitEnable为 false；
 * 本接口可在编码通道创建之后,编码通道销毁之前设置。此接口在编码过程中被调用时,等到
 * 下一帧时生效；
 * 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数；
 * 建议用户在调用此接口之前,先调用IMP_VENC_GetH264SliceSplit接口,获取当前通道的
 * slicesplit 配置,然后再进行设置；
 */
int IMP_VENC_SetH264SliceSplit(VENC_CHN VeChn, const VENC_PARAM_H264_SLICE_SPLIT_S *pstSliceSplit);

/**
 * IMP_VENC_GetH264SliceSplit 获取H.264通道的slice分割属性
 *
 * @fn int IMP_VENC_GetH264SliceSplit(VENC_CHN VeChn,
 *		VENC_PARAM_H264_SLICE_SPLIT_S *pstSliceSplit).
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[out] pstSliceSplit H.264码流slice分割参数
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 本接口用于获取H.264协议编码通道的slice分割属性；
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前调用；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数。
 *
 */
int IMP_VENC_GetH264SliceSplit(VENC_CHN VeChn, VENC_PARAM_H264_SLICE_SPLIT_S *pstSliceSplit);

/**
 * IMP_VENC_SetH264InterPred 设置H.264协议编码通道的帧间预测属性
 *
 * @fn int IMP_VENC_SetH264InterPred(VENC_CHN VeChn,
 *		const VENC_PARAM_H264_INTER_PRED_S *pstH264InterPred)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] pstH264InterPred H.264协议编码通道的帧间预测配置
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 帧间预测的属性主要由七个参数决定：
 *
 *		u32HWSize:帧间预测时的水平搜索窗的大小；
 *
 *		u32VWSize:帧间预测时的垂直搜索窗的大小；
 *
 *		bInter16x16PredEn:16x16 块帧间预测使能标识；
 *
 *		bInter16x8PredEn:16x8 块帧间预测使能标识；
 *
 *		bInter8x16PredEn:8x16 块帧间预测使能标识；
 *
 *		bInter8x8PredEn:8x8 块帧间预测使能标识；
 *
 *		bExtedgeEn:帧间预测扩边搜索使能。
 *
 * @remarks 此接口属于高阶接口,用户可以选择性调用,建议不调用,系统会有默认值；
 * @remarks bInter16x16PredEn、bInter16x8PredEn 、bInter8x16PredEn 、bInter8x8PredEn 的四
 * 个帧间预测使能开关必须有一个是开启的,系统不支持四个开关全部关闭；
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前设置。本接口在编码过程中被调用时,
 * 等到下一帧时生效；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数；
 * @remarks 建议用户在调用此接口之前,先调用IMP_VENC_GetH264InterPred接口,获取当前编码通道的
 * InterPred 配置,然后再进行设置；
 * @remarks 水平搜索窗的大小存在以下约束：
 *
 *		水平搜索窗必须大于等于 MIN_HW,且小于等于 MAX_HW；
 *
 *		图像宽度(以宏块为单位)必须比搜索窗实际宽度至少大2个宏块，如搜索窗的宽度设置为10,
 *		则实际的图像宽度至少大于:((1+ 10) x 2+1+1)x 16 = 384,如386；
 *
 * @remarks 垂直搜索窗的大小存在以下约束：
 *
 *		垂直搜索窗必须大于等于MIN_VW,且小于等于 MAX_VW；
 *
 *		图像高度(以宏块为单位)必须比搜索窗实际高度至少大1个宏块，如搜索窗的高度设置为4,
 *		则实际的图像高度至少大于:(1+ 4) x 2+1) x 16 = 176,如 178；
 */
int IMP_VENC_SetH264InterPred(VENC_CHN VeChn, const VENC_PARAM_H264_INTER_PRED_S *pstH264InterPred);

/**
 * IMP_VENC_GetH264InterPred 获取H.264协议编码通道的帧间预测属性
 *
 * @fn int IMP_VENC_GetH264InterPred(VENC_CHN VeChn,
 *		VENC_PARAM_H264_INTER_PRED_S *pstH264InterPred)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[out] pstH264InterPred H.264协议编码通道的帧间预测参数
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前调用；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数。
 */
int IMP_VENC_GetH264InterPred(VENC_CHN VeChn, VENC_PARAM_H264_INTER_PRED_S *pstH264InterPred);

/**
 * IMP_VENC_SetH264IntraPred 设置H.264协议编码通道的帧内预测属性
 *
 * @fn int IMP_VENC_SetH264IntraPred(VENC_CHN VeChn,
 *		const VENC_PARAM_H264_INTRA_PRED_S *pstH264IntraPred)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] pstH264IntraPred H.264协议编码通道的帧内预测配置.
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 帧间预测的属性主要由四个参数决定：
 *
 *		bIntra16x16PredEn:16x16 块帧内预测使能标识；
 *
 *		bIntraNxNPredEn:NxN 块帧内预测使能标识。其中,NxN表示4x4和8x8；
 *
 *		bIpcmEn:IPCM 块帧内预测使能标识；
 *
 *		constrained_intra_pred_flag:具体的含义,请参见 H.264 协议；
 *
 * @remarks 此接口属于高阶接口,用户可以选择性调用,不建议调用；
 * @remarks bIntra16x16PredEn、bIntraNxNPredEn 两种帧内预测使能开关必须有一个是开启的,系统不
 * 支持两个开关全部关闭；
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前设置。本接口在编码过程中被调用时,等到
 * 下一个 I 帧时生效；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数；
 * @remarks 建议用户在调用此接口之前,先调用IMP_VENC_GetH264IntraPred接口,获取当前编码通道的
 * IntraPred 配置,然后再进行设置。
 *
 */
int IMP_VENC_SetH264IntraPred(VENC_CHN VeChn, const VENC_PARAM_H264_INTRA_PRED_S *pstH264IntraPred);

/**
 * IMP_VENC_GetH264IntraPred 获取H.264协议编码通道的帧内预测属性
 *
 * @fn int IMP_VENC_GetH264IntraPred(VENC_CHN VeChn,
 *     VENC_PARAM_H264_INTRA_PRED_S *pstH264IntraPred)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[out] pstH264IntraPred H.264协议编码通道的帧内预测参数
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前调用；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数。
 */
int IMP_VENC_GetH264IntraPred(VENC_CHN VeChn, VENC_PARAM_H264_INTRA_PRED_S *pstH264IntraPred);

/**
 * IMP_VENC_SetH264Trans 设置H.264协议编码通道的变换、量化的属性
 *
 * @fn int IMP_VENC_SetH264Trans(VENC_CHN VeChn,
 *		const VENC_PARAM_H264_TRANS_S *pstH264Trans)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] pstH264Trans H.264协议编码通道的变换、量化属性
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 变换、量化属性主要由六个参数组成:
 *
 *		u32IntraTransMode:帧内预测宏块的变换属性。
 *		u32IntraTransMode=0 表示支持对帧内预测宏块支持4x4变换和8x8变换;
 *		u32IntraTransMode=1表示只支持对帧内预测宏块支持4x4变换；
 *		u32IntraTransMode=2表示只支持对帧内预测宏块支持8x8变换。
 *		只有在编码通道协议为highprofile时,才能选择8x8变换,即在 baseline profile和main profile下,
 *		系统只支持u32IntraTransMode=1的配置；
 *
 *		u32InterTransMode:帧间预测宏块的变换属性。
 *		u32InterTransMode=0 表示支持对帧间预测宏块支持4x4变换和8x8变换;
 *		u32InterTransMode=1表示只支持对帧间预测宏块支持4x4变换;
 *		u32InterTransMode=2表示只支持对帧间预测宏块支持8x8变换。
 *		只有在编码通道协议为high profile时,才能选择8x8变换,即在baseline profile和main profile下,
 *		系统只支持u32InterTransMode=1的配置；
 *
 *		bScalingListValid:表示InterScalingList8x8、IntraScalingList8x8量化表是否有效。Baseline、
 *		main profile 时,协议不支持8x8变换,此标志位没有意义。high profile 编码通道支持用户传递
 *		量化表。如果此标志位置为false,那么系统将使用H.264协议推荐的量化表,否则,将使用用户传递
 *		的量化表；
 *
 *		InterScalingList8x8:对帧间预测宏块进行8x8变换时,可由用户通过此数组提供量化表；
 *
 *		IntraScalingList8x8:对帧内预测宏块进行 8x8 变换时,可由用户通过此数组提供量化表；
 *
 *		chroma_qp_index_offset:具体含义请参见 H.264 协议；
 *
 * @remarks 此接口属于高阶接口,用户可以选择性调用,不建议调用,系统会有默认值；
 * @remarks 接口可在编码通道创建之后,编码通道销毁之前设置。本接口在编码过程中被调用时,等到下一个
 * I 帧时生效；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数；
 * @remarks 建议用户在调用此接口之前,先调用IMP_VENC_GetH264Trans接口,获取当前编码通道的trans配置,
 * 然后再进行设置.
 */
int IMP_VENC_SetH264Trans(VENC_CHN VeChn, const VENC_PARAM_H264_TRANS_S *pstH264Trans);

/**
 * IMP_VENC_GetH264Trans 获取H.264协议编码通道的变换、量化的属性
 *
 * @fn int IMP_VENC_GetH264Trans(VENC_CHN VeChn, VENC_PARAM_H264_TRANS_S *pstH264Trans)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[out] pstH264Trans H.264协议编码通道的变换、量化属性
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前调用；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数。
 */
int IMP_VENC_GetH264Trans(VENC_CHN VeChn, VENC_PARAM_H264_TRANS_S *pstH264Trans);

/**
 * IMP_VENC_SetH264Entropy 设置H.264协议编码通道的熵编码模式
 *
 * @fn int IMP_VENC_SetH264Entropy(VENC_CHN VeChn,
 *		const VENC_PARAM_H264_ENTROPY_S *pstH264EntropyEnc)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] pstH264EntropyEnc H.264 协议编码通道的熵编码模式
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 变换、量化属性主要由四个参数组成：
 *
 *     u32EntropyEncModeI:I帧的熵编码方式,
 *     u32EntropyEncModeI=0表示I帧使用cavlc编码,
 *     u32EntropyEncModeI=1表示I帧使用cabac编码方式；
 *
 *     u32EntropyEncModeP:p帧的熵编码方式,
 *     u32EntropyEncModeP=0表示P帧使用cavlc编码,
 *     u32EntropyEncModeP=1表示P帧使用cabac编码方式；
 *
 *     cabac_stuff_en:cabac编码填充使能,系统默认为0。具体含义请参见H.264协议；
 *
 *     Cabac_init_idc:cabac 初始化索引,系统默认为 0。具体含义请参见 H.264 协议；
 *
 * @remarks Baseline profile不支持cabac编码方式,支持cavlc编码方式,main profile和high profile
 * 支持cabac编码方式和cavlc编码方式；
 * @remarks 此接口属于高阶接口,用户可以选择性调用,不建议调用,系统会有默认值。系统会根据不同的
 * profile 设置默认参数；
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前设置。本接口在编码过程中被调用时,等到下
 * 一个I帧时生效；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数；
 * @remarks 建议用户在调用此接口之前,先调用IMP_VENC_GetH264Entropy接口,获取当前编码通道的Entropy
 * 配置,然后再进行设置。
 */
int IMP_VENC_SetH264Entropy(VENC_CHN VeChn, const VENC_PARAM_H264_ENTROPY_S *pstH264EntropyEnc);

/**
 * IMP_VENC_GetH264Entropy 获取H.264协议编码通道的熵编码属性
 *
 * @fn int IMP_VENC_GetH264Entropy(VENC_CHN VeChn,
 *		VENC_PARAM_H264_ENTROPY_S *pstH264EntropyEnc)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[out] pstH264EntropyEnc H.264 协议编码通道的熵编码属性
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前调用；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数。
 */
int IMP_VENC_GetH264Entropy(VENC_CHN VeChn, VENC_PARAM_H264_ENTROPY_S *pstH264EntropyEnc);

/**
 * IMP_VENC_SetH264Poc 设置H.264协议编码通道的POC类型
 *
 * @fn int IMP_VENC_SetH264Poc(VENC_CHN VeChn, const VENC_PARAM_H264_POC_S *pstH264Poc)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] pstH264Poc 设置H.264协议编码通道的POC类型
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks POC 属性主要指H.264码流的POC类型,具体含义请见 H.264 协议；
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前设置。本接口在编码过程中被调用时,等到
 * 下一个 I 帧时生效；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数；
 * @remarks 建议用户在调用此接口之前,先调用IMP_VENC_GetH264Poc接口,获取当前编码通道的Poc配置,
 * 然后再进行设置。
 */
int IMP_VENC_SetH264Poc(VENC_CHN VeChn, const VENC_PARAM_H264_POC_S *pstH264Poc);

/**
 * IMP_VENC_GetH264Poc 获取H.264协议编码通道的POC类型
 *
 * @fn int IMP_VENC_GetH264Poc(VENC_CHN VeChn, VENC_PARAM_H264_POC_S *pstH264Poc)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[out] pstH264Poc 设置H.264协议编码通道的POC类型
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前调用；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数。
 */
int IMP_VENC_GetH264Poc(VENC_CHN VeChn, VENC_PARAM_H264_POC_S *pstH264Poc);

/**
 * IMP_VENC_SetH264Dblk 设置H.264协议编码通道的Deblocking类型
 *
 * @fn int IMP_VENC_SetH264Dblk(VENC_CHN VeChn, const VENC_PARAM_H264_DBLK_S *pstH264Dblk)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] pstH264Dblk 设置H.264协议编码通道的POC类型
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 变换、量化属性主要由三个参数组成：
 *
 *     disable_deblocking_filter_idc:具体含义请参见H.264协议；
 *
 *     slice_alpha_c0_offset_div2:具体含义请参见H.264协议；
 *
 *     slice_beta_offset_div2:具体含义请参见H.264协议。
 *
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前设置。本接口在编码过程中被调用时,等到下
 * 一帧时生效；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数；
 * @remarks 建议用户在调用此接口之前,先调用IMP_VENC_GetH264Dblk接口,获取当前编码通道的Dblk配置,
 * 然后再进行设置。
 */
int IMP_VENC_SetH264Dblk(VENC_CHN VeChn, const VENC_PARAM_H264_DBLK_S *pstH264Dblk);

/**
 * IMP_VENC_GetH264Dblk 获取H.264协议编码通道的Deblocking类型
 *
 * @fn int IMP_VENC_GetH264Dblk(VENC_CHN VeChn, VENC_PARAM_H264_DBLK_S *pstH264Dblk)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[out] pstH264Dblk 设置H.264协议编码通道的POC类型
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前调用；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数。
 */
int IMP_VENC_GetH264Dblk(VENC_CHN VeChn, VENC_PARAM_H264_DBLK_S *pstH264Dblk);

/**
 * IMP_VENC_SetH264Vui 设置H.264协议编码通道的vui参数
 *
 * @fn int IMP_VENC_SetH264Vui(VENC_CHN VeChn, const VENC_PARAM_H264_VUI_S *pstH264Vui)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] pstH264Vui H.264协议编码通道的Vui参数
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks Vui 主要由四个参数组成：
 *
 *     timing_info_present_flag:具体含义请参见 H.264 协议；
 *
 *     num_units_in_tick:具体含义请参见 H.264 协议；
 *
 *     time_scale:具体含义请参见 H.264 协议；
 *
 *     fixed_frame_rate_flag:具体含义请参见 H.264 协议。
 *
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前设置。本接口在编码过程中被调用时,等到下
 * 一个 I 帧时生效；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数；
 * @remarks 建议用户在调用此接口之前,先调用IMP_VENC_GetH264Vui接口,获取当前编码通道的Vui配置,
 * 然后再进行设置。
 */
int IMP_VENC_SetH264Vui(VENC_CHN VeChn, const VENC_PARAM_H264_VUI_S *pstH264Vui);

/**
 * IMP_VENC_GetH264Vui 获取H.264协议编码通道的vui参数
 *
 * @fn int IMP_VENC_GetH264Vui(VENC_CHN VeChn, VENC_PARAM_H264_VUI_S *pstH264Vui)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[out] pstH264Vui H.264协议编码通道的Vui参数
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前调用；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数
 */
int IMP_VENC_GetH264Vui(VENC_CHN VeChn, VENC_PARAM_H264_VUI_S *pstH264Vui);

/**
 * IMP_VENC_SetJpegParam 设置JPEG协议编码通道的高级参数
 *
 * @fn int IMP_VENC_SetJpegParam(VENC_CHN VeChn, const VENC_PARAM_JPEG_S *pstJpegParam).
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] pstJpegParam JPEG 协议编码通道的高级参数集合.
 *
 * @retval 零 成功
 *
 * @retval 小于零 失败, 见错误码
 *
 * @remarks JPEG 协议编码通道的高级参数主要由5个参数组成：
 *
 *		u32Qfactor:量化表因子范围为[1, 99],u32Qfactor 越大,量化表中的量化系数越小,得到的图像
 *		质量会更好,同时,编码压缩率更低，具体的 u32Qfactor 与量化表的关系请见 RFC2435 标准；
 *
 *		u8YQt[64],u8CbQt[64],u8CrQt[64]:对应三个量化表空间,用户可以通过这三个参数设置用户的
 *		量化表；
 *
 *		u32MCUPerECS:每个Ecs中包含多少Mcu。系统模式u32MCUPerECS=0,表示当前帧的所有的MCU被编码为
 *		一个 ECS。u32MCUPerECS的最小值为0,最大值不超过(picwidth+15)>>4 x (picheight+15)>>4 x 2；
 *
 * @remarks 如果用户想使用自己的量化表,在设置量化表的同时,请将 Qfactor 设置为 50；
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前设置。本接口在编码过程中被调用时,等到下一个帧
 * 编码时生效；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数；
 * @remarks 建议用户在调用此接口之前，先调用IMP_VENC_GetJpegParam接口，获取当前编码通道的JpegParam
 * 配置，然后再进行设置。
 */
int IMP_VENC_SetJpegParam(VENC_CHN VeChn, const VENC_PARAM_JPEG_S *pstJpegParam);

/**
 * IMP_VENC_GetJpegParam 获取JPEG协议编码通道的高级参数
 *
 * @fn int IMP_VENC_GetJpegParam(VENC_CHN VeChn, VENC_PARAM_JPEG_S *pstJpegParam)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[out] pstJpegParam JPEG 协议编码通道的高级参数集合
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 本接口可在编码通道创建之后,编码通道销毁之前调用；
 * @remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数。
 *
 */
int IMP_VENC_GetJpegParam(VENC_CHN VeChn, VENC_PARAM_JPEG_S *pstJpegParam);

/**
 * IMP_VENC_SetGrpFrmRate 设置通道组帧率控制属性
 *
 * @fn int IMP_VENC_SetGrpFrmRate(VENC_GRP VeGroup, const GROUP_FRAME_RATE_S *pstGrpFrmRate)
 *
 * @param[in] VeGroup 编码通道组号,取值范围: [0, VENC_MAX_GRP_NUM)
 * @param[in] pstGrpFrmRate 通道组帧率控制属性指针
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 通道组帧率控制属性包括了输入帧率ViFrmRate和输出帧率VpssFrmRate两部分；
 * @remarks 设置通道组帧率控制属性时,输入帧率ViFrmRate和输出帧率VpssFrmRate必须同时大于 0
 * 或同时等于-1；
 * @remarks 当输入帧率ViFrmRate和输出帧率VpssFrmRate均大于0时,输入帧率ViFrmRate必须大于或
 * 等于输出帧率VpssFrmRate；
 * @remarks 当输入帧率ViFrmRate和输出帧率VpssFrmRate均等于-1时,此时表示不进行帧率控制；
 *
 * note 设置未创建的通道组的属性,则返回失败；
 * note 如果 pstGrpFrmRate 为空,则返回失败。
 */
int IMP_VENC_SetGrpFrmRate(VENC_GRP VeGroup, const GROUP_FRAME_RATE_S *pstGrpFrmRate);

/**
 * IMP_VENC_GetGrpFrmRate 获取通道组帧率控制属性
 *
 * @fn int IMP_VENC_GetGrpFrmRate(VENC_GRP VeGroup, GROUP_FRAME_RATE_S *pstGrpFrmRate)
 *
 * @param[in] VeGroup 编码通道组号,取值范围: [0, VENC_MAX_GRP_NUM)
 * @param[out] pstGrpFrmRate 通道组帧率控制属性指针
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * note 获取未创建的通道组的属性,返回失败；
 * note 如果 pstGrpFrmRate 为空,则返回失败。
 *
 */
int IMP_VENC_GetGrpFrmRate(VENC_GRP VeGroup, GROUP_FRAME_RATE_S *pstGrpFrmRate);

/**
 * IMP_VENC_SetRcPara 设置编码通道码率控制器的高级参数
 *
 * @fn int IMP_VENC_SetRcPara(VENC_CHN VeChn, VENC_RC_PARAM_S *pstRcPara)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] pstRcPara 编码通道码率控制器的高级参数
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * remarks 编码通道码率控制器的高级参数都有默认值,而不是必须调用这个接口才能启动编码通道;
 * remarks 建议用户先调用IMP_VENC_GetRcPara接口,获取RC高级参数,然后修改相应参数,再调用本接口对高
 * 级参数进行设置；
 * remarks RC高级参数现仅支持H.264 CBR、H.264 VBR、H264 CQP码率控制模式；
 * remarks 码率控制器的高级参数由以下参数组成:
 *
 *		u32ThrdI[12],u32ThrdP[12]:分别衡量I帧,P帧的宏块复杂度的一组阈值。这 12 个阈值按照从小到大
 *		的顺序依次排列,每个阈值的取值范围为[0, 255]。这组阈值用于在进行宏块级码率控制时,根据图像
 *		复杂度对每个宏块的Qp进行适当的调整。如果当前宏块的图像复杂度处于某两阈值之间时,当前宏块
 *		的Qp值就在宏块行起始Qp值的基础上加上较小阈值的索引号；
 *
 *		u32QpDelta:在宏块级码率控制时,每一行宏块的起始Qp相对于帧起始Qp的波动幅度值；
 *
 *		联合体：各种协议下码率控制参数；
 *
 *		pRcParam:由用户制定的RC高级参数,保留,暂时没有使用；
 *
 * remarks 建议用户在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数。
 */
int IMP_VENC_SetRcPara(VENC_CHN VeChn, VENC_RC_PARAM_S *pstRcPara);

/**
 * IMP_VENC_GetRcPara 获取编码通道码率控制器的高级参数
 *
 * @fn int IMP_VENC_GetRcPara(VENC_CHN VeChn, VENC_RC_PARAM_S *pstRcPara)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[out] pstRcPara 编码通道码率控制器的高级参数
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @note 如果pstRcPara为空,则返回失败
 */
int IMP_VENC_GetRcPara(VENC_CHN VeChn, VENC_RC_PARAM_S *pstRcPara);

/**
 * IMP_VENC_SetH264eRefMode 设置H.264编码通道跳帧参考模式
 *
 * @fn int IMP_VENC_SetH264eRefMode(VENC_CHN VeChn, VENC_ATTR_H264_REF_MODE_E enRefMode)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] enRefMode H.264 编码通道跳帧参考模式
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 创建H.264协议编码通道时,默认的跳帧参考模式是1倍跳帧参考模式(即没有开启跳帧参考)；
 * @remarks 如果用户需要修改编码通道的跳帧参考模式,建议在创建通道之后,启动编码之前调用此接口,
 * 减少在编码过程中调用的次数。
 */
int IMP_VENC_SetH264eRefMode(VENC_CHN VeChn, VENC_ATTR_H264_REF_MODE_E enRefMode);

/**
 * IMP_VENC_GetH264eRefMode 获取H.264编码通道跳帧参考模式
 *
 * @fn int IMP_VENC_GetH264eRefMode(VENC_CHN VeChn, VENC_ATTR_H264_REF_MODE_E *penRefMode).
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] penRefMode H.264 编码通道跳帧参考模式
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @note 如果 penRefMode 为空,则返回失败
 */
int IMP_VENC_GetH264eRefMode(VENC_CHN VeChn, VENC_ATTR_H264_REF_MODE_E *penRefMode);

/**
 * IMP_VENC_SetH264eRefParam 设置H.264编码通道高级跳帧参考参数
 *
 * @fn int IMP_VENC_SetH264eRefParam(VENC_CHN VeChn, VENC_ATTR_H264_REF_PARAM_S* pstRefParam).
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] pstRefParam H.264 编码通道高级跳帧参考参数
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 创建H.264协议编码通道时,默认跳帧参考模式是1倍跳帧参考模式。如果用户需要修改编
 * 码通道的跳帧参考,建议在创建通道之后,启动编码之前调用此接口,减少在编码过程中调用的次数;
 * @remarks 本接口在编码过程中被调用时,等到下一个 I 帧时生效。
 *
 * @note 如果 pstRefParam 为空,则返回失败。
 *
 */
int IMP_VENC_SetH264eRefParam( VENC_CHN VeChn, VENC_ATTR_H264_REF_PARAM_S* pstRefParam );

/**
 * IMP_VENC_GetH264eRefParam 获得H.264编码通道高级跳帧参考参数
 *
 * @fn int IMP_VENC_GetH264eRefParam(VENC_CHN VeChn, VENC_ATTR_H264_REF_PARAM_S* pstRefParam)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[out] pstRefParam H.264 编码通道高级跳帧参考参数
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @note 如果 pstRefParam 为空,则返回失败
 */
int IMP_VENC_GetH264eRefParam( VENC_CHN VeChn, VENC_ATTR_H264_REF_PARAM_S* pstRefParam );

/**
 * IMP_VENC_EnableIDR 强制使能IDR帧
 *
 * @fn int IMP_VENC_EnableIDR( VENC_CHN VeChn, bool bEnableIDR )
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] bEnableIDR 是否强制使能IDR帧
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 */
int IMP_VENC_EnableIDR( VENC_CHN VeChn, bool bEnableIDR );

/**
 * IMP_VENC_SetColor2GreyConf 设置彩转灰功能的全局配置,即调用此接口会影响所有 GROUP
 *
 * @fn int IMP_VENC_SetColor2GreyConf(const GROUP_COLOR2GREY_CONF_S *pstGrpColor2GreyConf)
 *
 * @param[in] pstGrpColor2GreyConf 彩转灰功能的全局配置参数
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks	GROUP_COLOR2GREY_CONF_S 由 3 个参数组成：
 *
 *		bEnable：所有 GROUP 是否开启彩转灰功能的全局开关；
 *
 *		u32MaxWidth：要开启彩转灰功能的GROUP的最大宽度。一个GROUP的宽度就是注册到其中的通道的宽度；
 *
 *		u32MaxHeight:要开启彩转灰功能的GROUP的最大高度。一个GROUP的高度就是注册到其中的通道的高度.
 *
 * @remarks	设置彩转灰功能的全局配置,即调用此接口会影响所有GROUP；一个GROUP如果要调用
 * IMP_VENC_SetGrpColor2Grey开启彩转灰功能,必须先调用此接口,并置bEnable为HI_TRUE；
 * @remarks	要改变上述 3 个参数中的任何一个,必须满足:所有 GROUP 均未开启彩转灰功能。
 *
 * @note 如果 pstGrpColor2GreyConf 为空,则返回失败。
 */
int IMP_VENC_SetColor2GreyConf(const GROUP_COLOR2GREY_CONF_S *pstGrpColor2GreyConf);

/**
 * IMP_VENC_GetColor2GreyConf 获取彩转灰功能的全局配置
 *
 * @fn int IMP_VENC_GetColor2GreyConf(GROUP_COLOR2GREY_CONF_S *pstGrpColor2GreyConf)
 *
 * @param[in] pstGrpColor2GreyConf 彩转灰功能的全局配置参数
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @note 如果 pstGrpColor2GreyConf 为空,则返回失败
 */
int IMP_VENC_GetColor2GreyConf(GROUP_COLOR2GREY_CONF_S *pstGrpColor2GreyConf);

/**
 * IMP_VENC_SetGrpColor2Grey 获取彩转灰功能的全局配置
 *
 * @fn int IMP_VENC_SetGrpColor2Grey(VENC_GRP VeGroup,
 *		const GROUP_COLOR2GREY_S *pstGrpColor2Grey).
 *
 * @param[in] VeGroup 编码通道组号,取值范围: [0, VENC_MAX_GRP_NUM)
 * @param[in] pstGrpColor2Grey 开启或关闭彩转灰功能的参数
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 一个GROUP如果要调用此API开启彩转灰功能,必须满足以下条件：
 *
 *		1、先调用IMP_VENC_SetColor2GreyConf设置彩转灰功能的全局配置,并置bEnable为HI_TRUE；
 *
 *		2、此 GROUP 中必须已经有通道注册；
 *
 *		3、注册到 GROUP 中的通道的宽高必须小于彩转灰功能全局配置中的最大宽高.
 */
int IMP_VENC_SetGrpColor2Grey(VENC_GRP VeGroup, const GROUP_COLOR2GREY_S *pstGrpColor2Grey);

/**
 * IMP_VENC_GetGrpColor2Grey 获取一个GROUP是否开启彩转灰功能
 *
 * @fn int IMP_VENC_GetGrpColor2Grey(VENC_GRP VeGroup, GROUP_COLOR2GREY_S *pstGrpColor2Grey)
 *
 * @param[in] VeGroup 编码通道组号,取值范围: [0, VENC_MAX_GRP_NUM)
 * @param[in] pstGrpColor2Grey 开启或关闭彩转灰功能的参数
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @note 如果 pstGrpColor2Grey 为空,则返回失败
 */
int IMP_VENC_GetGrpColor2Grey(VENC_GRP VeGroup, GROUP_COLOR2GREY_S *pstGrpColor2Grey);

/**
 * IMP_VENC_SetGrpCrop 设置GROUP的裁剪属性
 *
 * @fn int IMP_VENC_SetGrpCrop(VENC_GRP VeGroup, const GROUP_CROP_CFG_S *pstGrpCropCfg).
 *
 * @param[in] VeGroup 编码通道组号,取值范围: [0, VENC_MAX_GRP_NUM)
 * @param[in] pstGrpCropCfg 通道组裁剪属性
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 通道组会优先进行图像裁剪,然后基于裁剪之后的图像尺寸,与编码通道的尺寸进行比较,
 * 决定是否进行缩小；
 * @remarks 本接口必须在通道向通道组注册之后,通道从通道组注销之前调用；
 * @remarks 通道组内的通道注销之前,用户必须保证通道组的裁剪功能关闭；
 * @remarks 裁剪属性由两部分组成：
 *
 *     bEnable:是否使能通道组裁剪功能；
 *
 *     stRect:裁剪区域属性,包括裁剪区域起始点坐标,以及裁剪区域的尺寸。
 */
int IMP_VENC_SetGrpCrop(VENC_GRP VeGroup, const GROUP_CROP_CFG_S *pstGrpCropCfg);

/**
 * IMP_VENC_SetGrpCrop 获取GROUP的裁剪属性
 *
 * @fn int IMP_VENC_GetGrpCrop(VENC_GRP VeGroup, GROUP_CROP_CFG_S *pstGrpCropCfg).
 *
 * @param[in] VeGroup 编码通道组号,取值范围: [0, VENC_MAX_GRP_NUM)
 * @param[in] pstGrpCropCfg 通道组裁剪属性
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 本接口必须在通道组创建之后,通道组销毁之前调用。
 */
int IMP_VENC_GetGrpCrop(VENC_GRP VeGroup, GROUP_CROP_CFG_S *pstGrpCropCfg);

/**
 * IMP_VENC_SetJpegSnapMode 设置JPEG抓拍通道的抓拍模式
 *
 * @fn int IMP_VENC_SetJpegSnapMode(VENC_CHN VeChn, VENC_JPEG_SNAP_MODE_E enJpegSnapMode).
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] enJpegSnapMode 通道抓拍模式
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 本接口必须在编码通道创建之后,编码通道销毁之前调用；
 * @remarks 本接口只对 JPEG 类型的编码通道有效；
 * @remarks 通道抓拍模式有两种：
 *
 *     JPEG_SNAP_ALL 模式,通道启动接收图像之后,编码所有接收的图像；
 *
 *     JPEG_SNAP_FLASH 模式,通道启动接收图像之后,只编码被闪光的图像。
 *
 * @remarks 创建JPEG通道之后,JPEG 通道默认处于JPEG_SNAP_ALL模式。当用户需要只抓拍被闪光的图像时,
 * 可以调用本接口,设置JPEG通道为JPEG_SNAP_FLASH模式；
 * @remarks 当用户需要使用抓拍通道的JPEG_SNAP_FLASH模式时,必须配合VI的闪光灯功能一起使用；
 * @remarks 当JPEG设置为闪光灯抓拍模式时,通道组只能接收来自VIU模块(不经过VPSS模块)的图像,否则JPEG
 * 抓拍不到任何图像。
 */
int IMP_VENC_SetJpegSnapMode(VENC_CHN VeChn, VENC_JPEG_SNAP_MODE_E enJpegSnapMode);

/**
 * IMP_VENC_GetJpegSnapMode 获取JPEG抓拍通道的抓拍模式
 *
 * @fn int IMP_VENC_GetJpegSnapMode(VENC_CHN VeChn, VENC_JPEG_SNAP_MODE_E *penJpegSnapMode).
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, VENC_MAX_CHN_NUM)
 * @param[in] penJpegSnapMode 通道抓拍模式
 *
 * @retval 零 成功
 * @retval 小于零 失败, 见错误码
 *
 * @remarks 本接口必须在编码通道创建之后,编码通道销毁之前调用；
 * @remarks 本接口只对 JPEG 类型的编码通道有效。
 *
 */
int IMP_VENC_GetJpegSnapMode(VENC_CHN VeChn, VENC_JPEG_SNAP_MODE_E *penJpegSnapMode);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_VENC_H__ */
