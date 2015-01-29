/*
 * IMP Encoder func header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Justin <pengtao.kang@ingenic.com>
 */

#ifndef __IMP_ENCODER_H__
#define __IMP_ENCODER_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * @file
 * IMP视频编码头文件
 */

/**
 * @defgroup IMP视频编码
 * @brief Encoder -----------------TODO.
 * @{
 */

/**
 * 编码协议类型
 */
typedef enum {
	PT_JPEG,
	PT_H264,
} IMPPayloadType;

/**
 * 定义编码通道码率控制器模式
 */
typedef enum {
	ENC_RC_MODE_H264VBR,		/**< H.264 VBR 模式 */
	ENC_RC_MODE_H264FIXQP,		/**< H.264 Fixqp 模式 */
} IMPEncoderRcMode;

/**
 * 定义H.264编码通道Fixqp属性结构
 */
typedef struct {
	uint32_t	maxGop;			/**< H.264 gop 值,取值范围:[1, 65536] */
	uint32_t	inFrmRate;		/**< 编码通道的输入帧率,以 fps 为单位,取值范围:[1, 60] */
	uint32_t	outFrmRate;		/**< 编码通道的输出帧率,以 fps 为单位,取值范围:[0, u32ViFrmRate] */
	uint32_t	qp;				/**< I 帧所有宏块Qp值,取值范围:[0, 51] */
} IMPEncoderAttrH264FixQP;

/**
 * 定义H.264编码通道VBR属性结构
 */
typedef struct {
	uint32_t	maxGop;			/**< H.264 gop 值,取值范围:[1, 65536] */
	uint32_t	inFrmRate;		/**< 编码通道输入帧率,以fps为单位,取值范围:[1, 60] */
	uint32_t	outFrmRate;		/**< 编码通道的输出帧率,以fps为单位,取值范围:(0, u32ViFrmRate] */
	uint32_t	maxBitRate;		/**< 编码器输出最大码率,以kbps为单位,取值范围:[2, 40960] */
	uint32_t	maxQp;			/**< 编码器支持图像最大QP,取值范围:(u32MinQp, 51] */
	uint32_t	minQp;			/**< 编码器支持图像最小QP,取值范围:[0, 51] */
} IMPEncoderAttrH264VBR;

/**
 * 定义编码通道码率控制器属性
 */
typedef struct {
	IMPEncoderRcMode rcMode;						/**< RC 模式 */
	union {
		IMPEncoderAttrH264VBR	 attrH264Vbr;		/**< H.264 协议编码通道 Vbr 模式属性 */
		IMPEncoderAttrH264FixQP	 attrH264FixQp;		/**< H.264 协议编码通道 Fixqp 模式属性 */
	};
} IMPEncoderRcAttr;

/**
 * H264码流NALU类型
 */
typedef enum {
	IMP_NAL_UNKNOWN		= 0,
	IMP_NAL_SLICE		= 1,
	IMP_NAL_SLICE_DPA	= 2,
	IMP_NAL_SLICE_DPB	= 3,
	IMP_NAL_SLICE_DPC	= 4,
	IMP_NAL_SLICE_IDR	= 5,
	IMP_NAL_SEI			= 6,
	IMP_NAL_SPS			= 7,
	IMP_NAL_PPS			= 8,
	IMP_NAL_AUD			= 9,
	IMP_NAL_FILLER		= 12,
} IMPEncoderH264NaluType;

/**
 * 定义JPEG码流的PACK类型
 */
typedef enum {
	JPEGE_PACK_ECS = 5,			/**< ECS 类型 */
	JPEGE_PACK_APP = 6,			/**< APP 类型 */
	JPEGE_PACK_VDO = 7,			/**< VDO 类型 */
	JPEGE_PACK_PIC = 8,			/**< PIC 类型 */
	JPEGE_PACK_BUTT
} IMPEncoderJpegPackType;

/**
 * 定义编码码流类型
 */
typedef union {
	IMPEncoderH264NaluType		h264Type;		/**< H264E NALU 码流包类型 */
	IMPEncoderJpegPackType		jpegType;		/**< JPEGE 码流包类型 */
} IMPEncoderDataType;

/**
 * 定义帧码流包结构体
 */
typedef struct {
	uint32_t	phyAddr;						/**< 码流包物理地址 */
	uint32_t	virAddr;						/**< 码流包虚拟地址 */
	uint32_t	length;							/**< 码流包长度 */

	uint64_t	timestamp;						/**< 时间戳，单位us */
	bool		frameEnd;						/**< 帧结束标识 */

	IMPEncoderDataType	dataType;				/**< 码流类型 */
} IMPEncoderPack;

/**
 * 定义帧码流类型结构体
 */
typedef struct {
	IMPEncoderPack	*pack;				/**< 帧码流包结构 */
	uint32_t	packCount;				/**< 一帧码流的所有包的个数 */
	uint32_t	seq;					/**< 码流序列号，按帧获取帧序号，按包获取包序号 */
} IMPEncoderStream;

/**
 * 定义H264编码属性结构体
 */
typedef struct {
	uint32_t	bufSize;		/**< 码流 buffer 大小，取值范围:[Min, Max],以 byte 为单位，必须是 64 的整数倍，为静态属性 */
	uint32_t	profile;		/**< 编码的等级, 0: baseline; 1:MP; 2:HP [0,2] */
	bool		byFrame;		/**< 按帧/包模式获取码流，取值范围:{TRUE, FALSE}，TRUE:按帧获取，FALSE:按包获取，为静态属性 */
	uint32_t	picWidth;		/**< 编码图像宽度，取值范围:[MIN_WIDTH, u32MaxPicWidth],以像素为单位，必须是 MIN_ALIGN 的整数倍，为静态属性*/
	uint32_t	picHeight;		/**< 编码图像高度，取值范围:[MIN_HEIGHT, u32MaxPicHeight],以像素为单位，必须是 MIN_ALIGN 的整数倍，为静态属性 */
} IMPEncoderAttrH264;

/**
 * 定义 JPEG 抓拍属性结构体
 */
typedef struct {
	uint32_t	bufSize;		/**< 配置 buffer 大小，取值范围:不小于图像宽高乘积的1.5倍，必须是 64 的整数倍，为静态属性 */
	uint32_t	profile;		/**< 编码的等级, 0: baseline; 1:MP; 2:HP [0,2] */
	bool		byFrame;		/**< 获取码流模式,帧或包，取值范围:{TRUE, FALSE}，TRUE:按帧获取，FALSE:按包获取，为静态属性 */
	uint32_t	picWidth;		/**< 编码图像宽度，取值范围:[MIN_WIDTH, u32MaxPicWidth],以像素为单位，必须是 MIN_ALIGN 的整数倍，为静态属性 */
	uint32_t	picHeight;		/**< 编码图像高度，取值范围:[MIN_HEIGHT, u32MaxPicHeight],以像素为单位，必须是 MIN_ALIGN 的整数倍，为静态属性 */
} IMPEncoderAttrJpeg;

/**
 * 定义编码器属性结构体
 */
typedef struct {
	IMPPayloadType	enType;				/**< 编码协议类型 */
	union
	{
		IMPEncoderAttrH264	attrH264;	/**< H264编码属性结构体 */
		IMPEncoderAttrJpeg	attrJpeg;	/**< JPEG 抓拍属性结构体 */
	};
} IMPEncoderAttr;

/**
 * 定义编码通道属性结构体
 */
typedef struct {
	IMPEncoderAttr		encAttr;	/**< 编码器属性结构体 */
	IMPEncoderRcAttr	rcAttr;			/**< 码率控制器属性结构体 */
} IMPEncoderCHNAttr;

/**
 * 定义编码通道的状态结构体
 */
typedef struct {
	bool		registered;				/**< 注册到通道组标志，取值范围:{TRUE, FALSE}，TRUE:注册，FALSE:未注册 */
	uint32_t	leftPics;				/**< 待编码的图像数 */
	uint32_t	leftStreamBytes;		/**< 码流buffer剩余的byte数 */
	uint32_t	leftStreamFrames;		/**< 码流buffer剩余的帧数 */
	uint32_t	curPacks;				/**< 当前帧的码流包个数 */
	uint32_t	leftRecvPics;			/**< 剩余待接收的帧数,在用户设置，IMP_ENC_StartRecvPicEx后有效 */
	uint32_t	leftEncPics;			/**< 剩余待编码的帧数,在用户设置，IMP_ENC_StartRecvPicEx后有效 */
} IMPEncoderCHNStat;

/**
 * @fn int IMP_Encoder_CreateGroup(int encGroup)
 *
 * 创建编码通道组
 *
 * @param[in] encGroup 通道组号,取值范围:[0, ENC_MAX_GRP_NUM)
 *
 * @retval 零 成功
 * @retval 小于零 失败
 *
 * @remarks 编码通道组指芯片能够同时处理的编码通道的集合
 * @remarks 一路通道组仅支持一路分辨率，不同分辨率需启动新的通道组
 * @remarks 一路通道组同时支持一路H264和一路JPEG抓拍
 * @remarks 次码流重建一个通道组，不启用次通道
 *
 * @note 如果指定的通道组已经存在，则返回失败
 */
int IMP_Encoder_CreateGroup(int encGroup);

/**
 * @fn int IMP_Encoder_DestroyGroup(int encGroup)
 *
 * 销毁编码通道组.
 *
 * @param[in] encGroup 通道组号,取值范围:[0, ENC_MAX_GRP_NUM)
 *
 * @retval 零 成功
 * @retval 小于零 失败
 *
 * @remarks 销毁通道组时，必须保证通道组为空，即没有任何通道在通道组中注册，或注册到通道组中
 * 的通道已经反注册，否则返回失败
 *
 * @note 销毁并不存在的通道组，则返回失败
 */
int IMP_Encoder_DestroyGroup(int encGroup);

/**
 * @fn int IMP_Encoder_CreateChn(int encChn, const IMPEncoderCHNAttr *attr)
 *
 * 创建编码通道
 *
 * @param[in] encChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
 * @param[in] attr 编码通道属性指针
 *
 * @retval 零 成功
 * @retval 小于零 失败
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
int IMP_Encoder_CreateChn(int encChn, const IMPEncoderCHNAttr *attr);

/**
 * @fn int IMP_Encoder_DestroyChn(int encChn)
 *
 * 销毁编码通道
 *
 * @param[in] encChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
 *
 * @retval 零 成功
 * @retval 小于零 失败
 *
 * @note 销毁并不存在的通道，则返回失败
 * @note 销毁前必须保证通道已经从通道组反注册，否则返回失败
 */
int IMP_Encoder_DestroyChn(int encChn);

/**
 * @fn int IMP_Encoder_RegisterChn(int encGroup, int encChn)
 *
 * 注册编码通道到通道组
 *
 * @param[in] encGroup 编码通道组号,取值范围: [0, ENC_MAX_GRP_NUM)
 * @param[in] encChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
 *
 * @retval 零 成功
 * @retval 小于零 失败
 *
 * @note 注册并不存在的通道，则返回失败
 * @note 注册通道到不存在的通道组，否则返回失败
 * @note 同一个编码通道只能注册到一个通道组，如果该通道已经注册到某个通道组，则返回失败
 * @note 如果一个通道组已经被注册，那么这个通道组就不能再被其他的通道注册，除非之前注册
 * 关系被解除
 */
int IMP_Encoder_RegisterChn(int encGroup, int encChn);

/**
 * @fn int IMP_Encoder_UnRegisterChn(int encChn)
 *
 * 反注册编码通道到通道组
 *
 * @param[in] encChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
 *
 * @retval 零 成功
 * @retval 小于零 失败
 *
 * @remarks 通道注销之后，编码通道会被复位，编码通道里的码流buffer都会被清空，如果用户还在使用
 * 未及时释放的码流buffer，将不能保证buffer数据的正确性，用户可以使用IMP_Encoder_Query接口来查询编
 * 码通道码流buffer状态，确认码流buffer里的码流取完之后再反注册通道
 *
 * @note 注销未创建的通道，则返回失败
 * @note 注销未注册的通道，则返回失败
 * @note 如果编码通道未停止接收图像编码，则返回失败
 */
int IMP_Encoder_UnRegisterChn(int encChn);

/**
 * @fn int IMP_Encoder_StartRecvPic(int encChn)
 *
 * 开启编码通道接收图像,超出指定的帧数后自动停止接收图像
 *
 * @param[in] encChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
 *
 * @retval 零 成功
 * @retval 小于零 失败
 *
 * @remarks 如果通道已经调用了IMP_Encoder_StartRecvPic开始接收图像而没有停止接收图像，或者上次调用
 * IMP_Encoder_StartRecvPicEx后还没有接收到足够的图像，再次调用此接口不允许
 *
 * @note 如果通道未创建，则返回失败
 * @note 如果通道没有注册到通道组，则返回失败
 */
int IMP_Encoder_StartRecvPic(int encChn);

/**
 * @fn int IMP_Encoder_StopRecvPic(int encChn)
 *
 * 停止编码通道接收图像
 *
 * @param[in] encChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
 *
 * @retval 零 成功
 * @retval 小于零 失败
 *
 * @remarks 此接口并不判断当前是否停止接收，即允许重复停止接收不返回错误
 * @remarks 此接口用于编码通道停止接收图像来编码，编码通道反注册前必须停止接收图像
 * @remarks 调用此接口仅停止接收原始数据编码，码流buffer并不会被消除
 *
 * @note 如果通道未创建，则返回失败
 * @note 如果通道没有注册到通道组，则返回失败
 */
int IMP_Encoder_StopRecvPic(int encChn);

/**
 * @fn int IMP_Encoder_Query(int encChn, IMPEncoderCHNStat *stat)
 *
 * xxx
 *
 * @param[in] encChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM]
 * @param[out] stat abc
 *
 * @retval 零 成功
 * @retval 小于零 失败
 *
 * @remarks 此接口并不判断当前是否停止接收，即允许重复停止接收不返回错误
 * @remarks 此接口用于编码通道停止接收图像来编码，编码通道反注册前必须停止接收图像
 * @remarks 调用此接口仅停止接收原始数据编码，码流buffer并不会被消除
 *
 * @note 如果通道未创建，则返回失败
 * @note 如果通道没有注册到通道组，则返回失败
 */
int IMP_Encoder_Query(int encChn, IMPEncoderCHNStat *stat);

/**
 * @fn int IMP_Encoder_GetStream(int encChn, IMPEncoderStream *stream, bool blockFlag)
 *
 * 获取编码的码流
 *
 * @param[in] encChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
 * @param[out] stream 码流结构体指针
 * @param[in] blockFlag 是否使用阻塞方式获取，0：非阻塞，1：阻塞
 *
 * @retval 零 成功
 * @retval 小于零 失败
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
 * @remarks 码流结构体IMPEncoderStreamSt包含4个部分：
 *
 *		pstPack 码流包信息指针，指向一组Encoder_PACK_S的内存空间,该空间由调用者分配。如果是
 *		按包获取,则此空间不小于 sizeof(Encoder_PACK_S)的大小; 如果按帧获取,则此空间不小于
 *		N × sizeof(Encoder_PACK_S)的大小,其中 N 代表当前帧之中的包的个数,可以在select 之后
 *		通过查询接口获得；
 *
 *		u32PackCount码流包个数，在输入时,此值指定pstPack中Encoder_PACK_S的个数。按包获取时,
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
int IMP_Encoder_GetStream(int encChn, IMPEncoderStream *stream, bool blockFlag);

/**
 * @fn int IMP_Encoder_ReleaseStream(int encChn, IMPEncoderStream *stream)
 *
 * 释放码流缓存
 *
 * @param[in] encChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
 * @param[in] stream 码流结构体指针
 *
 * @retval 零 成功
 * @retval 小于零 失败
 *
 * @remarks 此接口应当和IMP_Encoder_GetStream配对起来使用，用户获取码流后必须及时释放已经
 * 获取的码流缓存，否则可能会导致码流buffer满，影响编码器编码，并且用户必须按先获取先
 * 释放的顺序释放已经获取的码流缓存；
 * @remarks 在编码通道反注册后，所有未释放的码流包均无效，不能再使用或者释放这部分无效的码流缓存。
 *
 * @note 如果pstStream为NULL,则返回失败；
 * @note 如果通道未创建，则返回失败；
 * @note 释放无效的码流会返回失败。
 */
int IMP_Encoder_ReleaseStream(int encChn, IMPEncoderStream *stream);

int IMP_Encoder_PoolingStream(int encChn, uint32_t timeoutMsec);

int IMP_Encoder_RequestIDR(int encChn);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_ENCODER_H__ */
