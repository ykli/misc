/*
 * IMP common rc data structure and macro header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Justin <pengtao.kang@ingenic.com>
 */

#ifndef __IMP_COMM_RC_H__
#define __IMP_COMM_RC_H__

/**
 * @file
 * venc data type interface.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/**
 * 定义编码通道码率控制器模式
 */
typedef enum ENUM_IMP_VENC_RC_MODE_E
{
    VENC_RC_MODE_H264CBR = 1,	/**< H.264 CBR模式 */
    VENC_RC_MODE_H264VBR,       /**< H.264 VBR 模式 */
    VENC_RC_MODE_H264ABR,       /**< H.264 ABR 模式 */
    VENC_RC_MODE_H264FIXQP,     /**< H.264 Fixqp 模式 */

    VENC_RC_MODE_BUTT,

}VENC_RC_MODE_E;

/**
 * 定义H.264编码通道Fixqp属性结构
 */
typedef struct IMP_VENC_ATTR_H264_FIXQP_S
{
    uint32_t	u32Gop;					/**< H.264 gop 值,取值范围:[1, 65536] */
    uint32_t	u32ViFrmRate;           /**< 编码通道的输入帧率,以 fps 为单位,取值范围:[1, 60] */
    uint32_t	u32TargetFrmRate ;		/**< 编码通道的输出帧率,以 fps 为单位,取值范围:
										  (0, u32ViFrmRate] */
    uint32_t	u32IQp;                 /**< I 帧所有宏块Qp值,取值范围:[0, 51] */
    uint32_t	u32PQp;                 /**< P 帧所有宏块Qp值,取值范围:[0, 51] */
} VENC_ATTR_H264_FIXQP_S;

/**
 * 定义H.264编码通道CBR属性结构
 */
typedef struct IMP_VENC_ATTR_H264_CBR_S
{
    uint32_t	u32Gop;                 /**< H.264 gop 值,取值范围:[1, 65536] */
    uint32_t	u32StatTime;            /**< CBR 码率静态统计时间,以秒为单位,取值范围:[1, 16] */
    uint32_t	u32ViFrmRate;           /**< 编码通道输入帧率,以 fps 为单位,取值范围:[1, 60] */
    uint32_t	u32TargetFrmRate ;      /**< 编码通道的输出帧率,以 fps 为单位,取值范围:
										  (0, u32ViFrmRate] */
    uint32_t	u32BitRate;             /**< 平均bitrate,以kbps为单位,取值范围:[2, 40960] */
    uint32_t	u32FluctuateLevel;      /**< 最大码率相对平均码率的波动等级,取值范围:[0, 5],
										  推荐使用波动等级0 */
} VENC_ATTR_H264_CBR_S;

/**
 * 定义H.264编码通道VBR属性结构
 */
typedef struct IMP_VENC_ATTR_H264_VBR_S
{
    uint32_t	u32Gop;                 /**< H.264 gop 值,取值范围:[1, 65536] */
    uint32_t	u32StatTime;            /**< VBR 码率静态统计时间,以秒为单位,取值范围:[1, 16] */
    uint32_t	u32ViFrmRate;           /**< 编码通道输入帧率,以fps为单位,取值范围:[1, 60] */
    uint32_t	u32TargetFrmRate ;      /**< 编码通道的输出帧率,以fps为单位,取值范围:
										  (0, u32ViFrmRate] */
    uint32_t	u32MaxBitRate;          /**< 编码器输出最大码率,以kbps为单位,取值范围:[2, 40960] */
    uint32_t	u32MaxQp;               /**< 编码器支持图像最大QP,取值范围:(u32MinQp, 51] */
    uint32_t	u32MinQp;               /**< 编码器支持图像最小QP,取值范围:[0, 51] */
}VENC_ATTR_H264_VBR_S;

/**
 * 定义H.264编码通道ABR属性结构
 */
typedef struct IMP_VENC_ATTR_H264_ABR_S
{
    uint32_t	u32Gop;                 /**< H.264 gop 值,取值范围:[1, 65536] */
    uint32_t	u32StatTime;            /**< VBR 码率静态统计时间,以秒为单位,取值范围:[1, 16] */
    uint32_t	u32ViFrmRate;           /**< 编码通道输入帧率,以fps为单位,取值范围:[1, 60] */
    uint32_t	u32TargetFrmRate ;      /**< 编码通道的输出帧率,以fps为单位,取值范围:
										  (0, u32ViFrmRate] */
    uint32_t	u32AvgBitRate;          /**< 编码器输出平均码率,以kbps为单位,取值范围:[2,40960] */
    uint32_t	u32MaxBitRate;          /**< 编码器输出最大码率,以kbps为单位,取值范围:[2, 40960] */
}VENC_ATTR_H264_ABR_S;

/**
 * 定义编码通道码率控制器属性
 */
typedef struct IMP_VENC_RC_ATTR_S
{
    VENC_RC_MODE_E enRcMode;            /**< RC 模式 */
    union
    {
        VENC_ATTR_H264_CBR_S    stAttrH264Cbr; /**< H.264 协议编码通道 Cbr 模式属性 */
        VENC_ATTR_H264_VBR_S    stAttrH264Vbr; /**< H.264 协议编码通道 Vbr 模式属性 */
        VENC_ATTR_H264_FIXQP_S  stAttrH264FixQp; /**< H.264 协议编码通道 Fixqp 模式属性 */
        VENC_ATTR_H264_ABR_S    stAttrH264Abr;	/**< H.264 协议编码通道 Abr 模式属性 */
    };
    void*       pRcAttr ;     /**< 用户定制的码率控制属性 */
}VENC_RC_ATTR_S;

/**
 * 定义码率控制中超大帧处理模式
 */
typedef enum ENUM_IMP_RC_SUPERFRM_MODE_EN
{
    SUPERFRM_NONE,          /**< 无特殊策略 */
    SUPERFRM_DISCARD,       /**< 丢弃超大帧 */
    SUPERFRM_REENCODE,      /**< 重编超大帧 */
    SUPERFRM_BUTT
}VENC_SUPERFRM_MODE_EN;

/**
 * 定义 H264 协议编码通道CBR码率控制模式高级参数配置
 */
typedef struct IMP_VENC_PARAM_H264_CBR_S
{
    uint32_t  u32MinIprop;      /**< 最小IP帧比例, 取值范围:(0, 100] */
    uint32_t  u32MaxIprop;      /**< 最大IP帧比例,取值范围:(u32MinIprop, 100] */
    uint32_t  u32MaxQp;			/**< 帧最大 QP,用于钳位质量,取值范围:(u32MinQp, 51] */
    uint32_t  u32MaxStartQp;    /**< 帧级码率控制输出的最大 QP,取值范围:[u32MinQp, u32MaxQp],
								  宏块级码率控制可能会使某些宏块的 QP 大于或小于 u32MaxStartQp,
								  但都会被钳位到,[u32MinQp, u32MaxQp] */
    uint32_t  u32MinQp;         /**< 帧最小 QP,用于钳位码率波动,取值范围:[0, 51] */
    uint32_t  u32MaxPPDeltaQp;  /**< P 帧间的最大 QP差异,取值范围:[0, 10] */
    uint32_t  u32MaxIPDeltaQp;  /**< I/P 帧之间的最大QP,取值范围:[0, 10] */
    bool bLostFrmOpen;       /**< 瞬时码率超出,是否允许丢帧,TRUE:允许丢帧,FALSE:不允许丢帧 */
    uint32_t  u32LostFrmBpsThr; /**< 瞬时码率阈值,超过此阈值,允许丢帧,单位bit,取值范围:
								  [64x1024, 80x1024x1024] */
    VENC_SUPERFRM_MODE_EN  enSuperFrmMode;  /**< 超大帧模式 */
    uint32_t  u32SuperIFrmBitsThr;  /**< 超大 I 帧码率阈值,单位 bit,取值范围:大于等于 0 */
    uint32_t  u32SuperPFrmBitsThr;	/**< 超大 P 帧码率阈值,单位 bit,取值范围:大于等于 0 */
    int  s32IPQPDelta;				/**< I帧和前一个GOP的平均P帧QP的差异,取值范围:[-10, 10] */
    uint32_t  u32RQRatio[8];		/**< R-Q 的侧重比例,即码率恒定权重,取值范围:[0, 100] */
}VENC_PARAM_H264_CBR_S;

/**
 * 定义 H264 协议编码通道VBR码率控制模式高级参数配置
 */
typedef struct IMP_VENC_PARAM_H264_VBR_S
{
    int s32DeltaQP;						/**< VBR调整Qp时的帧与帧之间Qp变化的最大值,取值范围:[0, 10] */
    int s32ChangePos;                   /**< VBR 开始调整 Qp 时的码率相对于最大码率的比例,取值范围:
										  [50, 100] */
    uint32_t  u32MinIprop;              /**< 最小IP帧码率的比值,最小值为 1 */
    uint32_t  u32MaxIprop;              /**< 最大 IP 帧码率的比值,最大值为 100 */

    bool bLostFrmOpen;               /**< 是否丢帧开关 */
    uint32_t  u32LostFrmBpsThr;         /**< 丢帧的码率阈值,丢帧开关打开时有效 */

    VENC_SUPERFRM_MODE_EN  enSuperFrmMode;/**< 超大帧模式 */
    uint32_t  u32SuperIFrmBitsThr;      /**< 超大I帧的码率阈值,以 bit 为单位 */
    uint32_t  u32SuperPFrmBitsThr;      /**< 超大P帧的码率阈值,以 bit 为单位 */
}VENC_PARAM_H264_VBR_S;

/**
 * 定义 H264 协议编码通道ABR码率控制模式高级参数配置
 */
typedef struct IMP_VENC_PARAM_H264_ABR_S
{
    uint32_t u32MaxQp;		/**< 帧最大 QP,用于钳位质量,取值范围:(u32MinQp, 51] */
    uint32_t u32MinQp;		/**< 帧最小 QP,用于钳位码率波动,取值范围:[0, 51] */
    int s32IPQPDelta;		/**< I帧和前一个GOP的平均P帧QP的差异,取值范围:[-10, 10] */
    uint32_t u32StartQp;	/**< 帧级码率控制输出的QP,取值范围:[u32MinQp, u32MaxQp],
							  宏块级码率控制可能会使某些宏块的 QP 大于或小于 u32MaxStartQp,
							  但都会被钳位到,[u32MinQp, u32MaxQp] */
}VENC_PARAM_H264_ABR_S;

/**
 * 定义编码通道的码率控制高级参数
 */
typedef struct IMP_VENC_RC_PARAM_S
{
    uint32_t u32ThrdI[12];  /**< I 帧宏块级码率控制的 mad 门限,取值范围:[0, 255] */
    uint32_t u32ThrdP[12];	/**< P 帧宏块级码率控制的 mad 门限,取值范围:[0, 255] */
    uint32_t u32QpDelta;	/**< I 帧和 P 帧的 QP 的差值,取值范围:[0, 10] */
    union
    {
        VENC_PARAM_H264_CBR_S stParamH264Cbr; /**< H.264 通道 CBR(Constant Bit Rate)码率控制模式
												高级参数 */
        VENC_PARAM_H264_VBR_S stParamH264VBR; /**< H.264 通道 VBR(Variable Bit Rate)码率控制模式
												高级参数*/
        VENC_PARAM_H264_ABR_S stParamH264ABR; /**< H.264 通道 ABR(Average Bit Rate)码率控制模式
												高级参数 */
    };
    void	*pRcParam;  /**< 用户自定义的码率控制参数 */
}VENC_RC_PARAM_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_COMM_RC_H__ */
