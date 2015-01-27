/*
 * IMP Encoder func header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Justin <pengtao.kang@ingenic.com>
 */

#ifndef __IMP_ENCODER_H__
#define __IMP_ENCODER_H__

#include "imp_comm_video.h"

/**
 * @file
 * Encoder数据格式和函数接口头文件
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef int ENC_CHN;
typedef int ENC_GRP;

/**
 * 编码协议类型
 */
typedef enum {
    PT_PCMU          = 0,
    PT_1016          = 1,
    PT_G721          = 2,
    PT_GSM           = 3,
    PT_G723          = 4,
    PT_DVI4_8K       = 5,
    PT_DVI4_16K      = 6,
    PT_LPC           = 7,
    PT_PCMA          = 8,
    PT_G722          = 9,
    PT_S16BE_STEREO  = 10,
    PT_S16BE_MONO    = 11,
    PT_QCELP         = 12,
    PT_CN            = 13,
    PT_MPEGAUDIO     = 14,
    PT_G728          = 15,
    PT_DVI4_3        = 16,
    PT_DVI4_4        = 17,
    PT_G729          = 18,
    PT_G711A         = 19,
    PT_G711U         = 20,
    PT_G726          = 21,
    PT_G729A         = 22,
    PT_LPCM          = 23,
    PT_CelB          = 25,
    PT_JPEG          = 26,
    PT_CUSM          = 27,
    PT_NV            = 28,
    PT_PICW          = 29,
    PT_CPV           = 30,
    PT_H261          = 31,
    PT_MPEGVIDEO     = 32,
    PT_MPEG2TS       = 33,
    PT_H263          = 34,
    PT_SPEG          = 35,
    PT_MPEG2VIDEO    = 36,
    PT_AAC           = 37,
    PT_WMA9STD       = 38,
    PT_HEAAC         = 39,
    PT_PCM_VOICE     = 40,
    PT_PCM_AUDIO     = 41,
    PT_AACLC         = 42,
    PT_MP3           = 43,
    PT_ADPCMA        = 49,
    PT_AEC           = 50,
    PT_X_LD          = 95,
    PT_H264          = 96,
    PT_D_GSM_HR      = 200,
    PT_D_GSM_EFR     = 201,
    PT_D_L8          = 202,
    PT_D_RED         = 203,
    PT_D_VDVI        = 204,
    PT_D_BT656       = 220,
    PT_D_H263_1998   = 221,
    PT_D_MP1S        = 222,
    PT_D_MP2P        = 223,
    PT_D_BMPEG       = 224,
    PT_MP4VIDEO      = 230,
    PT_MP4AUDIO      = 237,
    PT_VC1           = 238,
    PT_JVC_ASF       = 255,
    PT_D_AVI         = 256,
    PT_DIVX3		 = 257,
    PT_AVS			 = 258,
    PT_REAL8		 = 259,
    PT_REAL9		 = 260,
    PT_VP6			 = 261,
    PT_VP6F			 = 262,
    PT_VP6A			 = 263,
    PT_SORENSON		 = 264,
    PT_MAX           = 265,
    PT_BUTT
} IMPEncoderPayloadTypeEn;

/**
 * 定义编码通道码率控制器模式
 */
typedef enum {
    ENC_RC_MODE_H264CBR = 1,	/**< H.264 CBR模式 */
    ENC_RC_MODE_H264VBR,       /**< H.264 VBR 模式 */
    ENC_RC_MODE_H264ABR,       /**< H.264 ABR 模式 */
    ENC_RC_MODE_H264FIXQP,     /**< H.264 Fixqp 模式 */

    ENC_RC_MODE_BUTT,

} IMPEncoderRcModeEn;

/**
 * 定义H.264编码通道Fixqp属性结构
 */
typedef struct {
    uint32_t	u32Gop;					/**< H.264 gop 值,取值范围:[1, 65536] */
    uint32_t	u32ViFrmRate;           /**< 编码通道的输入帧率,以 fps 为单位,取值范围:[1, 60] */
    uint32_t	u32TargetFrmRate ;		/**< 编码通道的输出帧率,以 fps 为单位,取值范围:
										  (0, u32ViFrmRate] */
    uint32_t	u32IQp;                 /**< I 帧所有宏块Qp值,取值范围:[0, 51] */
    uint32_t	u32PQp;                 /**< P 帧所有宏块Qp值,取值范围:[0, 51] */
} IMPEncoderAttrH264FixQPSt;

/**
 * 定义H.264编码通道CBR属性结构
 */
typedef struct {
    uint32_t	u32Gop;                 /**< H.264 gop 值,取值范围:[1, 65536] */
    uint32_t	u32StatTime;            /**< CBR 码率静态统计时间,以秒为单位,取值范围:[1, 16] */
    uint32_t	u32ViFrmRate;           /**< 编码通道输入帧率,以 fps 为单位,取值范围:[1, 60] */
    uint32_t	u32TargetFrmRate ;      /**< 编码通道的输出帧率,以 fps 为单位,取值范围:
										  (0, u32ViFrmRate] */
    uint32_t	u32BitRate;             /**< 平均bitrate,以kbps为单位,取值范围:[2, 40960] */
    uint32_t	u32FluctuateLevel;      /**< 最大码率相对平均码率的波动等级,取值范围:[0, 5],
										  推荐使用波动等级0 */
} IMPEncoderAttrH264CBRSt;

/**
 * 定义H.264编码通道VBR属性结构
 */
typedef struct {
    uint32_t	u32Gop;                 /**< H.264 gop 值,取值范围:[1, 65536] */
    uint32_t	u32StatTime;            /**< VBR 码率静态统计时间,以秒为单位,取值范围:[1, 16] */
    uint32_t	u32ViFrmRate;           /**< 编码通道输入帧率,以fps为单位,取值范围:[1, 60] */
    uint32_t	u32TargetFrmRate ;      /**< 编码通道的输出帧率,以fps为单位,取值范围:
										  (0, u32ViFrmRate] */
    uint32_t	u32MaxBitRate;          /**< 编码器输出最大码率,以kbps为单位,取值范围:[2, 40960] */
    uint32_t	u32MaxQp;               /**< 编码器支持图像最大QP,取值范围:(u32MinQp, 51] */
    uint32_t	u32MinQp;               /**< 编码器支持图像最小QP,取值范围:[0, 51] */
} IMPEncoderAttrH264VBRSt;

/**
 * 定义H.264编码通道ABR属性结构
 */
typedef struct {
    uint32_t	u32Gop;                 /**< H.264 gop 值,取值范围:[1, 65536] */
    uint32_t	u32StatTime;            /**< VBR 码率静态统计时间,以秒为单位,取值范围:[1, 16] */
    uint32_t	u32ViFrmRate;           /**< 编码通道输入帧率,以fps为单位,取值范围:[1, 60] */
    uint32_t	u32TargetFrmRate ;      /**< 编码通道的输出帧率,以fps为单位,取值范围:
										  (0, u32ViFrmRate] */
    uint32_t	u32AvgBitRate;          /**< 编码器输出平均码率,以kbps为单位,取值范围:[2,40960] */
    uint32_t	u32MaxBitRate;          /**< 编码器输出最大码率,以kbps为单位,取值范围:[2, 40960] */
} IMPEncoderAttrH264ABRSt;

/**
 * 定义编码通道码率控制器属性
 */
typedef struct {
    IMPEncoderRcModeEn enRcMode;            /**< RC 模式 */
    union
    {
        IMPEncoderAttrH264CBRSt    stAttrH264Cbr; /**< H.264 协议编码通道 Cbr 模式属性 */
        IMPEncoderAttrH264VBRSt    stAttrH264Vbr; /**< H.264 协议编码通道 Vbr 模式属性 */
        IMPEncoderAttrH264FixQPSt  stAttrH264FixQp; /**< H.264 协议编码通道 Fixqp 模式属性 */
        IMPEncoderAttrH264ABRSt    stAttrH264Abr;	/**< H.264 协议编码通道 Abr 模式属性 */
    };
    void*       pRcAttr ;     /**< 用户定制的码率控制属性 */
} IMPEncoderRcAttrSt;

/**
 * 定义码率控制中超大帧处理模式
 */
typedef enum {
    ENC_SUPERFRM_NONE,          /**< 无特殊策略 */
    ENC_SUPERFRM_DISCARD,       /**< 丢弃超大帧 */
    ENC_SUPERFRM_REENCODE,      /**< 重编超大帧 */
    ENC_SUPERFRM_BUTT
} IMPEncoderSuperfrmModeEn;

/**
 * 定义 H264 协议编码通道CBR码率控制模式高级参数配置
 */
typedef struct {
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
    IMPEncoderSuperfrmModeEn  enSuperFrmMode;  /**< 超大帧模式 */
    uint32_t  u32SuperIFrmBitsThr;  /**< 超大 I 帧码率阈值,单位 bit,取值范围:大于等于 0 */
    uint32_t  u32SuperPFrmBitsThr;	/**< 超大 P 帧码率阈值,单位 bit,取值范围:大于等于 0 */
    int  s32IPQPDelta;				/**< I帧和前一个GOP的平均P帧QP的差异,取值范围:[-10, 10] */
    uint32_t  u32RQRatio[8];		/**< R-Q 的侧重比例,即码率恒定权重,取值范围:[0, 100] */
} IMPEncoderParamH264CBRSt;

/**
 * 定义 H264 协议编码通道VBR码率控制模式高级参数配置
 */
typedef struct {
    int s32DeltaQP;						/**< VBR调整Qp时的帧与帧之间Qp变化的最大值,取值范围:[0, 10] */
    int s32ChangePos;                   /**< VBR 开始调整 Qp 时的码率相对于最大码率的比例,取值范围:
										  [50, 100] */
    uint32_t  u32MinIprop;              /**< 最小IP帧码率的比值,最小值为 1 */
    uint32_t  u32MaxIprop;              /**< 最大 IP 帧码率的比值,最大值为 100 */

    bool bLostFrmOpen;               /**< 是否丢帧开关 */
    uint32_t  u32LostFrmBpsThr;         /**< 丢帧的码率阈值,丢帧开关打开时有效 */

    IMPEncoderSuperfrmModeEn  enSuperFrmMode;/**< 超大帧模式 */
    uint32_t  u32SuperIFrmBitsThr;      /**< 超大I帧的码率阈值,以 bit 为单位 */
    uint32_t  u32SuperPFrmBitsThr;      /**< 超大P帧的码率阈值,以 bit 为单位 */
} IMPEncoderParamH264VBRSt;

/**
 * 定义 H264 协议编码通道ABR码率控制模式高级参数配置
 */
typedef struct {
    uint32_t u32MaxQp;		/**< 帧最大 QP,用于钳位质量,取值范围:(u32MinQp, 51] */
    uint32_t u32MinQp;		/**< 帧最小 QP,用于钳位码率波动,取值范围:[0, 51] */
    int s32IPQPDelta;		/**< I帧和前一个GOP的平均P帧QP的差异,取值范围:[-10, 10] */
    uint32_t u32StartQp;	/**< 帧级码率控制输出的QP,取值范围:[u32MinQp, u32MaxQp],
							  宏块级码率控制可能会使某些宏块的 QP 大于或小于 u32MaxStartQp,
							  但都会被钳位到,[u32MinQp, u32MaxQp] */
} IMPEncoderParamH264ABRSt;

/**
 * 定义编码通道的码率控制高级参数
 */
typedef struct {
    uint32_t u32ThrdI[12];  /**< I 帧宏块级码率控制的 mad 门限,取值范围:[0, 255] */
    uint32_t u32ThrdP[12];	/**< P 帧宏块级码率控制的 mad 门限,取值范围:[0, 255] */
    uint32_t u32QpDelta;	/**< I 帧和 P 帧的 QP 的差值,取值范围:[0, 10] */
    union
    {
        IMPEncoderParamH264CBRSt stParamH264Cbr; /**< H.264 通道 CBR(Constant Bit Rate)码率控制模式
												高级参数 */
        IMPEncoderParamH264VBRSt stParamH264VBR; /**< H.264 通道 VBR(Variable Bit Rate)码率控制模式
												高级参数*/
        IMPEncoderParamH264ABRSt stParamH264ABR; /**< H.264 通道 ABR(Average Bit Rate)码率控制模式
												高级参数 */
    };
    void	*pRcParam;  /**< 用户自定义的码率控制参数 */
} IMPEncoderRcParamSt;

/**
 * @def TYPE_MAIN_STREAM
 * main stream
 */
#define TYPE_MAIN_STREAM  (1)

/**
 * @def TYPE_MINOR_STREAM
 * minor stream
 */
#define TYPE_MINOR_STREAM (0)

/**
 * H264码流NALU类型
 */
typedef enum {
     H264E_NALU_PSLICE = 1, /**< PSLICE 类型 */
     H264E_NALU_ISLICE = 5, /**< ISLICE 类型 */
     H264E_NALU_SEI    = 6, /**< SEI 类型 */
     H264E_NALU_SPS    = 7, /**< SPS 类型*/
     H264E_NALU_PPS    = 8, /**< PPS 类型 */
     H264E_NALU_BUTT
} IMPEncoderH264NaluTypeEn;

/**
 * 定义获取的H264码流属于何种跳帧参考模式下的参考帧
 */
typedef enum {
     H264E_REFSLICE_FOR_1X = 1, /**< 1倍跳帧参考时的参考帧 */
     H264E_REFSLICE_FOR_2X = 2, /**< 2倍跳帧参考时的参考帧 */
     H264E_REFSLICE_FOR_4X = 5, /**< 4倍跳帧参考时的参考帧 */
     H264E_REFSLICE_FOR_BUTT    /**< 非参考帧 */
} IMPEncoderH264RefsliceTypeEn;

/**
 * 定义JPEG码流的PACK类型
 */
typedef enum {
     JPEGE_PACK_ECS = 5, /**< ECS 类型 */
     JPEGE_PACK_APP = 6, /**< APP 类型 */
     JPEGE_PACK_VDO = 7, /**< VDO 类型 */
     JPEGE_PACK_PIC = 8, /**< PIC 类型 */
     JPEGE_PACK_BUTT
} IMPEncoderJpegPackTypeEn;

/**
 * 定义编码码流类型
 */
typedef union {
    IMPEncoderH264NaluTypeEn    enH264EType; /**< H264E NALU 码流包类型 */
    IMPEncoderJpegPackTypeEn    enJPEGEType; /**< JPEGE 码流包类型 */
} IMPEncoderDataTypeUn;

/**
 * 定义帧码流包结构体
 */
typedef struct {
    uint32_t	u32PhyAddr[2];      /**< 码流包物理地址 */
    uint8_t		*pu8Addr[2];        /**< 码流包虚拟地址 */
    uint32_t	u32Len[2];          /**< 码流包长度 */

    uint64_t	u64PTS;             /**< 时间戳，单位us */
    bool		bFieldEnd;          /**< 场结束标识 */
    bool		bFrameEnd;			/**< 帧结束标识 */

    IMPEncoderDataTypeUn  DataType;		/**< 码流类型 */
} IMPEncoderPackSt;

/**
 * 定义H264高级跳帧参考帧类型
 */
typedef enum {
    BASE_IDRSLICE = 0,                  /**< 基本IDR帧或I帧 */
    BASE_PSLICE_REFBYBASE,              /**< 基本P帧，被其他基本P帧参考 */
    BASE_PSLICE_REFBYENHANCE,           /**< 基本P帧，被其他增强P帧参考 */
    ENHANCE_PSLICE_REFBYENHANCE,        /**< 增强P帧，被其他增强P帧参考 */
    ENHANCE_PSLICE_NOTFORREF,           /**< 增强P帧，不被参考 */
    ENHANCE_PSLICE_BUTT
} IMPEncoderRefTypeEn;

/**
 * 定义H264协议码流特帧信息
 */
typedef struct {
    uint32_t	u32PicBytesNum;     /**< 编码当前帧的字节数 */
    uint32_t	u32PSkipMbNum;      /**< 编码当前帧中采用跳跃(SKIP)编码模式的宏块数 */
    uint32_t	u32IpcmMbNum;       /**< 编码当前帧中采用IPCM编码模式的宏块数 */
    uint32_t	u32Inter16x8MbNum;  /**< 编码当前帧中采用Inter16x8编码模式的宏块数 */
    uint32_t	u32Inter16x16MbNum; /**< 编码当前帧中采用Inter16x16编码模式的宏块数 */
    uint32_t	u32Inter8x16MbNum;  /**< 编码当前帧中采用Inter8x16编码模式的宏块数 */
    uint32_t	u32Inter8x8MbNum;   /**< 编码当前帧中采用Inter8x8编码模式的宏块数 */
    uint32_t	u32Intra16MbNum;    /**< 编码当前帧中采用Intra16编码模式的宏块数 */
    uint32_t	u32Intra8MbNum;     /**< 编码当前帧中采用Intra8编码模式的宏块数 */
    uint32_t	u32Intra4MbNum;     /**< 编码当前帧中采用Intra4编码模式的宏块数 */

    IMPEncoderH264RefsliceTypeEn enRefSliceType;	/**< 编码当前帧属于何种跳帧参考模式下的参考帧 */
    IMPEncoderRefTypeEn      enRefType;	/**< 编码当前帧属于何种高级跳帧参考模式下的参考帧 */
    uint32_t	u32UpdateAttrCnt;		/**< 更新属性的次数 */
} IMPEncoderStreamInfoH264St;

/**
 * 定义JPEG协议码流特征信息
 */
typedef struct {
    uint32_t	u32PicBytesNum;     /**< 一帧jpeg码流大小,以字节(byte)为单位 */
    uint32_t	u32UpdateAttrCnt;	/**< 更新属性的次数 */
} IMPEncoderStreamInfoJpegSt;

/**
 * 定义帧码流类型结构体
 */
typedef struct {
    IMPEncoderPackSt	*pstPack;       /**< 帧码流包结构 */
    uint32_t	u32PackCount;   /**< 一帧码流的所有包的个数 */
    uint32_t	u32Seq;         /**< 码流序列号，按帧获取帧序号，按包获取包序号 */

    union
    {
        IMPEncoderStreamInfoH264St stH264Info;   /**< H264码流包特征信息 */
        IMPEncoderStreamInfoJpegSt stJpegInfo;   /**< JPEG码流包特征信息 */
    };
} IMPEncoderStreamSt;

/**
 * 定义H264编码属性结构体
 */
typedef struct {
    uint32_t	u32MaxPicWidth;     /**< 编码图像最大宽度，取值范围:[MIN_WIDTH,MAX_WIDTH],
									  以像素为单位,必须是 MIN_ALIGN 的整数倍，为静态属性 */
    uint32_t	u32MaxPicHeight;    /**< 编码图像最大高度，取值范围:[MIN_HEIGHT,MAX_HEIGHT],
									  以像素为单位，必须是 MIN_ALIGN 的整数倍，为静态属性 */

    uint32_t	u32BufSize;         /**< 码流 buffer 大小，取值范围:[Min, Max],以 byte 为单位，
									  必须是 64 的整数倍，为静态属性 */
    uint32_t	u32Profile;         /**< 编码的等级, 0: baseline; 1:MP; 2:HP [0,2] */
    bool		bByFrame;           /**< 按帧/包模式获取码流，取值范围:{TRUE, FALSE}，
										TRUE:按帧获取，FALSE:按包获取，为静态属性 */

    int			bField;             /**< 帧场编码模式，取值范围:{TRUE, FALSE}，TRUE:
									  场编码，FALSE:帧编码，这里只支持帧编码，为静态属性 */
    int			bMainStream;        /**< 主次码流标识，取值范围:{TRUE, FALSE}，TRUE:
									  主码流，FALSE:次码流，只支持主码流，为静态属性 */
    uint32_t	u32Priority;        /**< 通道优先级 */
    bool		bVIField;           /**< 输入图像的帧场标志 */

    uint32_t	u32PicWidth;        /**< 编码图像宽度，取值范围:[MIN_WIDTH, u32MaxPicWidth],以像素
									  为单位，必须是 MIN_ALIGN 的整数倍，为静态属性*/
    uint32_t	u32PicHeight;       /**< 编码图像高度，取值范围:[MIN_HEIGHT, u32MaxPicHeight],
									  以像素为单位，必须是 MIN_ALIGN 的整数倍，为静态属性 */
} IMPEncoderAttrH264St;

/**
 * 定义 JPEG 抓拍属性结构体
 */
typedef struct {
    uint32_t	u32MaxPicWidth;      /**< 编码图像最大宽度，取值范围:[MIN_WIDTH,MAX_WIDTH],
									   以像素为单位，必须是 MIN_ALIGN 的整数倍，为静态属性 */
    uint32_t	u32MaxPicHeight;     /**< 编码图像最大高度，取值范围:[MIN_HEIGHT,MAX_HEIGHT],
									   以像素为单位，必须是 MIN_ALIGN 的整数倍，为静态属性 */
    uint32_t	u32BufSize;          /**< 配置 buffer 大小，取值范围:不小于图像宽高乘积的1.5
									   倍，必须是 64 的整数倍，为静态属性 */
    bool		bByFrame;            /**< 获取码流模式,帧或包，取值范围:{TRUE, FALSE}，
										TRUE:按帧获取，FALSE:按包获取，为静态属性 */

    bool		bVIField;            /**< 输入图像的帧场标志，取值范围:{TRUE, FALSE}，
										TRUE:场，FALSE:帧，为静态属性 */
    uint32_t	u32Priority;         /**< 通道优先级 */

    uint32_t	u32PicWidth;         /**< 编码图像宽度，取值范围:[MIN_WIDTH, u32MaxPicWidth],
									   以像素为单位，必须是 MIN_ALIGN 的整数倍，为静态属性 */
    uint32_t	u32PicHeight;        /**< 编码图像高度，取值范围:[MIN_HEIGHT, u32MaxPicHeight],
									   以像素为单位，必须是 MIN_ALIGN 的整数倍，为静态属性 */

} IMPEncoderAttrJpegSt;

/**
 * 定义编码器属性结构体
 */
typedef struct {
    IMPEncoderPayloadTypeEn	enType;      /**< 编码协议类型 */
    union
    {
        IMPEncoderAttrH264St  stAttrH264e;     /**< H264编码属性结构体 */
        IMPEncoderAttrJpegSt  stAttrJpeg;      /**< JPEG 抓拍属性结构体 */
    };
} IMPEncoderAttrSt;

/**
 * 定义编码通道属性结构体
 */
typedef struct {
    IMPEncoderAttrSt		stVeAttr;		/**< 编码器属性结构体 */
    IMPEncoderRcAttrSt	stRcAttr;		/**< 码率控制器属性结构体 */
} IMPEncoderCHNAttrSt;

/**
 * 定义编码通道的状态结构体
 */
typedef struct {
    bool		bRegistered;			/**< 注册到通道组标志，取值范围:{TRUE, FALSE}，
										  TRUE:注册，FALSE:未注册 */
    uint32_t	u32LeftPics;			/**< 待编码的图像数 */
    uint32_t	u32LeftStreamBytes;		/**< 码流buffer剩余的byte数 */
    uint32_t	u32LeftStreamFrames;	/**< 码流buffer剩余的帧数 */
    uint32_t	u32CurPacks;			/**< 当前帧的码流包个数 */
    uint32_t	u32LeftRecvPics;		/**< 剩余待接收的帧数,在用户设置，IMP_ENC_StartRecvPicEx
										  后有效 */
    uint32_t	u32LeftEncPics;			/**< 剩余待编码的帧数,在用户设置，IMP_ENC_StartRecvPicEx
										  后有效 */
} IMPEncoderCHNStatSt;

/**
 * H264编码能力属性结构体
 */
typedef struct {
    uint8_t		u8Profile       ;   /**< 编码的档次，0:baseline 1:mainprofile  2:high profile */
    uint8_t		u8Level         ;   /**< 编码等级，比如: 22对应level2.2 */
    uint8_t		u8BaseAttr      ;   /**< 基本属性，bit0到bit5依次对应于MBAFF, PAFF、
									  B SLICE、FMO、ASO、PARTITION */
    uint8_t		u8ViFormat      ;   /**< 视频输入制式 bit0对应PAL(25)，bit1对应NTSC(30) */
    uint16_t	u16MaxWInMb     ;   /**< 输入视频的最大宽度 */
    uint16_t	u16MaxIMPnMb     ;  /**< 输入视频的最大高度 */
    uint16_t	u16MaxCifNum    ;   /**< 最大的编码帧数 */
    uint16_t	u16MaxBitrate   ;   /**< 最大的输出码率 */
    uint16_t	upperbandwidth  ;   /**< 带宽上限值 */
    uint16_t	lowerbandwidth  ;   /**< 带宽下限值 */
    uint8_t		palfps          ;   /**< PAL制式的帧率 */
    uint8_t		ntscfps         ;   /**< NTSC制式的帧率 */
} IMPEncoderH264CapabilitySt;

/**
 * JPEG编码能力属性结构体
 */
typedef struct {
    uint8_t		u8Profile        ;   /**< 编码的档次，0:baseline 1:extened profile
										2:loseless profile 3:hierarchical profile */
    uint8_t		u8ViFormat       ;   /**< 视频输入制式 bit.0对应PAL(25)，bit1对应于NTSC(30) */
    uint16_t	u16MaxWInMb      ;   /**< 输入视频的最大宽度 */
    uint16_t	u16MaxIMPnMb     ;   /**< 输出视频的最大高度 */
    uint16_t	u16MaxCifNum     ;   /**< 最大的编码吞吐量，即最大编码帧数 */
    uint16_t	u16MaxBitrate    ;   /**< 最大的输出码率 */
    uint16_t	upperbandwidth   ;   /**< 带宽上限 */
    uint16_t	lowerbandwidth   ;   /**< 带宽下限 */
    uint8_t		palfps           ;   /**< PAL制式的帧率 */
    uint8_t		ntscfps          ;   /**< NTSC制式的帧率 */
} IMPEncoderJpegCapabilitySt;

/**
 * 编码能力属性结构体
 */
typedef struct {
    IMPEncoderPayloadTypeEn enType;	 /**< 编码协议类型 */
    union
    {
        IMPEncoderH264CapabilitySt  stH264Cap; /**< H264编码能力属性结构体 */
        IMPEncoderJpegCapabilitySt  stJpegCap; /**< JPEG编码能力属性结构体 */
    };
} IMPEncoderCapabilitySt;

/**
 * 定义H.264协议编码通道SLICE分割结构体
 */
typedef struct {
    bool		bSplitEnable;      /**< slice分割使能, TRUE:enable, FALSE:diable, 默认值:FALSE */
    uint32_t	u32SplitMode;      /**< slice分割模式，0:按bit数分割, 1:按宏块行分割, >=2:无意义 */
    uint32_t	u32SliceSize;      /**< 当u32SplitMode=0，表示每个slice的bit数，最小值为128,
									 最大值为min(0xffff，u32PicSize/2)，其中u32PicSize=PicWidth*
									 PicHeight*3/2；
									 当u32SplitMode=1，表示每个slice占的宏块行数，最小值为1，
									 最大值为(图像高+15)/16 */
} IMPEncoderParamH264SliceSplitSt;

/**
 * 定义H.264协议编码通道帧间预测结构体
 */
typedef struct {
    uint32_t	u32HWSize;      /**< 水平搜索窗大小 */
    uint32_t	u32VWSize;      /**< 垂直搜索窗大小 */

    bool	bInter16x16PredEn;  /**< 16x16 帧间预测使能开关,默认使能 */
    bool	bInter16x8PredEn;   /**< 16x8 帧间预测使能开关,默认使能 */
    bool	bInter8x16PredEn;   /**< 8x16 帧间预测使能开关,默认使能 */
    bool	bInter8x8PredEn;    /**< 8x8 帧间预测使能开关,默认使能 */
    bool	bExtedgeEn;         /**< 当搜索遇见图像边界时,超出了图像范围,是否进行补搜的使
								  能开关,默认使能 */
} IMPEncoderParamH264InterPredSt;

/**
 * 定义 H.264 协议编码通道帧内预测结构体
 */
typedef struct {
    bool	bIntra16x16PredEn;  /**< 16x16 帧内预测使能,默认使能，0:不使能，1:使能 */
    bool	bIntraNxNPredEn;    /**< NxN 帧内预测使能,默认使能，0:不使能，1:使能 */
    uint32_t constrained_intra_pred_flag; /**< 默认为 0，取值范围:0 或 1 */
    bool	bIpcmEn; /**< IPCM 预测使能，取值范围:0 或 1 */
} IMPEncoderParamH264IntraPredSt;

/**
 * 定义 H.264 协议编码通道变换、量化结构体
 */
typedef struct {
    uint32_t	u32IntraTransMode;       /**< 帧内预测的变换模式, 0: trans4x4, trans8x8; 1: trans4x4,
											2: trans8x8 */
    uint32_t	u32InterTransMode;       /**< 帧间预测的变换模式, 0: trans4x4, trans8x8; 1: trans4x4,
											2: trans8x8 */
    bool		bScalingListValid;		 /**< InterScalingList8x8、IntraScalingList8x8是否有效标
										  识,只在 high profile 下才有意义,0:无效,1:有效 */
    uint8_t		InterScalingList8X8[64]; /**< 帧间预测 8x8 的量化表,在 high profile 下,用户可以
										   使用自己的量化表,取值范围:[1, 255] */
    uint8_t		IntraScalingList8X8[64]; /**< 帧内预测 8x8 的量化表,在 high profile 下,用户可以
										   使用自己的量化表,取值范围:[1, 255] */
    int			chroma_qp_index_offset;   /**< 取值范围:[-12, 12] */
} IMPEncoderParamH264TransSt;

/**
 * 定义 H.264 协议编码通道熵编码结构体
 */
typedef struct {
    uint32_t	u32EntropyEncModeI;         /**< I 帧熵编码模式,0:cavlc, 1:cabac,Baseline
											   不支持 cabac */
    uint32_t	u32EntropyEncModeP;         /**< P 帧熵编码模式,0:cavlc, 1:cabac,Baseline
											   不支持 cabac */
    uint32_t	cabac_stuff_en;             /**< 具体含义请参见 H.264 协议,系统默认为 0 */
    uint32_t	Cabac_init_idc;             /**< 取值范围[0, 2], 默认值 0 */
} IMPEncoderParamH264EntropySt;

/**
 * 定义H.264协议编码通道Poc结构体
 */
typedef struct {
    uint32_t	pic_order_cnt_type; /**< 取值范围[0, 2],默认值2,具体含义请参见H.264协议 */

} IMPEncoderParamH264PocSt;

/**
 * 定义H.264协议编码通道Dblk结构体
 */
typedef struct {
    uint32_t	disable_deblocking_filter_idc;   /**< 取值范围[0,2],默认值0,具体含义请参见H.264协议 */
    int			slice_alpha_c0_offset_div2;      /**< 取值范围[-6,6],默认值0,具体含义请参见
												   H.264协议 */
    int			slice_beta_offset_div2;          /**< 取值范围[-6,6],默认值0,具体含义请参见
												   H.264协议 */
} IMPEncoderParamH264DblkSt;

/**
 * 定义 H.264 协议编码通道 Vui 结构体
 */
typedef struct {
    int		timing_info_present_flag;    /**< 具体含义请参见H.264协议,系统默认为0,取值范围:0或1 */
    int		num_units_in_tick;           /**< 具体含义请参见H.264协议,系统默认为1,取值范围:大于0 */
    int		time_scale;                  /**< 具体含义请参见H.264协议,系统默认为60,取值范围:大于0 */
    int		fixed_frame_rate_flag;       /**< 具体含义请参见H.264协议,系统默认为1,取值范围:0或1 */
} IMPEncoderParamH264VuiSt;

/**
 * 定义JPEG协议编码通道高级参数结构体
 */
typedef struct {
    uint32_t	u32Qfactor;     /**< 具体含义请参见RFC2435协议,系统默认为90,取值范围:[1,99] */

    uint8_t		u8YQt[64];      /**< Y 量化表,取值范围:[1, 255] */
    uint8_t		u8CbQt[64];     /**< Cb 量化表,取值范围:[1, 255] */
    uint8_t		u8CrQt[64];     /**< Cb 量化表,取值范围:[1, 255] */

    uint32_t	u32MCUPerECS;   /**< 每个 ECS 中包含多少个 MCU,系统默认为 0,表示不划分Ecs,
									u32MCUPerECS:[0, (picwidth+15)>>4 x (picheight+15)>>4 x 2] */
} IMPEncoderParamJpegSt;

/**
 * 开启或关闭一个 GROUP 的彩转灰功能
 */
typedef struct {
    bool	bColor2Grey;    /**< 开启或关闭一个GROUP的彩转灰功能 */
} IMPEncoderGroupColor2GreySt;

/**
 * 彩转灰功能的全局配置参数
 */
typedef struct {
    bool		bEnable;		/**< 所有 GROUP 是否开启彩转灰功能的全局开关 */
    uint32_t	u32MaxWidth;	/**< 要开启彩转灰功能的GROUP的最大宽度。一个GROUP 的宽度就是
								  注册到其中的通道的宽度 */
    uint32_t	u32MaxHeight;	/**< 要开启彩转灰功能的GROUP的最大高度。一个GROUP 的高度就是
								  注册到其中的通道的高度*/
} IMPEncoderGroupColor2GreyConfSt;

/**
 * 定义帧率设置结构体
 */
typedef struct {
    int		s32ViFrmRate;       /**< 通道组的输入帧率 */
    int		s32VpssFrmRate;     /**< 通道组的输出帧率 */
} IMPEncoderGroupFrameRateSt;

/**
 * 定义 H.264 编码的高级跳帧参考参数
 */
typedef struct {
    uint32_t	u32Base;		/**< base 层的周期 */
    uint32_t	u32Enhance;		/**< enhance 层的周期 */
    bool		bEnablePred;	/**< 代表base层的帧是否被base层其他帧用作参考;当为HI_FALSE时,
								  base层的所有帧都参考IDR帧 */
} IMPEncoderAttrH264RefParamSt;

/**
 * 定义 H.264 编码的跳帧参考模式
 */
typedef enum {
    H264E_REF_MODE_1X = 1,	/**< 正常参考模式 */
    H264E_REF_MODE_2X = 2,	/**< 间隔 2 的跳帧参考模式 */
    H264E_REF_MODE_4X = 5,	/**< 间隔 4 的跳帧参考模式 */
    H264E_REF_MODE_BUTT,
} IMPEncoderAttrH264RefModeEn;

/**
 * JPEG 编码通道的抓拍模式
 */
typedef enum {
    JPEG_SNAP_ALL   = 0,    /**< 全部抓拍模式,JPEG 通道的默认抓拍模式 */
    JPEG_SNAP_FLASH = 1,    /**< 闪光灯抓拍模式,JPEG 通道配合前端闪光灯时使用的抓拍模式 */
    JPEG_SNAP_BUTT,

} IMPEncoderJpegSnapModeEn;

/**
 * 接收图像参数
 */
typedef struct {
    int s32RecvPicNum;       /**< 编码通道连续接收并编码的帧数 */
} IMPEncoderRecvPicParamSt;

/**
 * IMP_Encoder_CreateGroup 创建编码通道组
 *
 * @fn int IMP_Encoder_CreateGroup(ENC_GRP VeGroup)
 *
 * @param[in] VeGroup 通道组号,取值范围:[0, ENC_MAX_GRP_NUM)
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
int IMP_Encoder_CreateGroup(ENC_GRP VeGroup);

/**
 * IMP_Encoder_DestroyGroup 销毁编码通道组.
 *
 * @fn int IMP_Encoder_DestroyGroup(ENC_GRP VeGroup)
 *
 * @param[in] VeGroup 通道组号,取值范围:[0, ENC_MAX_GRP_NUM)
 *
 * @retval 零 成功
 * @retval 小于零 失败
 *
 * @remarks 销毁通道组时，必须保证通道组为空，即没有任何通道在通道组中注册，或注册到通道组中
 * 的通道已经反注册，否则返回失败
 *
 * @note 销毁并不存在的通道组，则返回失败
 */
int IMP_Encoder_DestroyGroup(ENC_GRP VeGroup);

/**
 * IMP_Encoder_CreateChn 创建编码通道
 *
 * @fn int IMP_Encoder_CreateChn(ENC_CHN VeChn, const IMPEncoderCHNAttrSt *pstAttr)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
 * @param[in] pstAttr 编码通道属性指针
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
int IMP_Encoder_CreateChn(ENC_CHN VeChn, const IMPEncoderCHNAttrSt *pstAttr);

/**
 * IMP_Encoder_DestroyChn 销毁编码通道
 *
 * @fn int IMP_Encoder_DestroyChn(ENC_CHN VeChn)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
 *
 * @retval 零 成功
 * @retval 小于零 失败
 *
 * @note 销毁并不存在的通道，则返回失败
 * @note 销毁前必须保证通道已经从通道组反注册，否则返回失败
 */
int IMP_Encoder_DestroyChn(ENC_CHN VeChn);

/**
 * IMP_Encoder_RegisterChn 注册编码通道到通道组
 *
 * @fn int IMP_Encoder_RegisterChn(ENC_GRP VeGroup, ENC_CHN VeChn)
 *
 * @param[in] VeGroup 编码通道组号,取值范围: [0, ENC_MAX_GRP_NUM)
 * @param[in] VeChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
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
int IMP_Encoder_RegisterChn(ENC_GRP VeGroup, ENC_CHN VeChn );

/**
 * IMP_Encoder_UnRegisterChn 反注册编码通道到通道组
 *
 * @fn int IMP_Encoder_UnRegisterChn(ENC_CHN VeChn)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
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
int IMP_Encoder_UnRegisterChn(ENC_CHN VeChn);

/**
 * IMP_Encoder_StartRecvPicEx开启编码通道接收图像,超出指定的帧数后自动停止接收图像
 *
 * @fn int IMP_Encoder_StartRecvPicEx(ENC_CHN VeChn, IMPEncoderRecvPicParamSt *pstRecvParam)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
 * @param[in] pstRecvParam 接收图像参数结构体指针，用于指定需要接收图像的帧数
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
int IMP_Encoder_StartRecvPicEx(ENC_CHN VeChn, IMPEncoderRecvPicParamSt *pstRecvParam);

/**
 * IMP_Encoder_StopRecvPic 停止编码通道接收图像
 *
 * @fn int IMP_Encoder_StopRecvPic(ENC_CHN VeChn)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
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
int IMP_Encoder_StopRecvPic(ENC_CHN VeChn);

/**
 * IMP_Encoder_GetStream 获取编码的码流
 *
 * @fn int IMP_Encoder_GetStream(ENC_CHN VeChn, IMPEncoderStreamSt *pstStream, bool bBlockFlag)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
 * @param[out] pstStream 码流结构体指针
 * @param[in] bBlockFlag 是否使用阻塞方式获取，0：非阻塞，1：阻塞
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
int IMP_Encoder_GetStream(ENC_CHN VeChn, IMPEncoderStreamSt *pstStream, bool bBlockFlag);

/**
 * IMP_Encoder_ReleaseStream 释放码流缓存
 *
 * @fn int IMP_Encoder_ReleaseStream(ENC_CHN VeChn, IMPEncoderStreamSt *pstStream)
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
 * @param[in] pstStream 码流结构体指针
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
int IMP_Encoder_ReleaseStream(ENC_CHN VeChn, IMPEncoderStreamSt *pstStream);

/**
 * IMP_Encoder_EnableIDR 强制使能IDR帧
 *
 * @fn int IMP_Encoder_EnableIDR( ENC_CHN VeChn, bool bEnableIDR )
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
 * @param[in] bEnableIDR 是否强制使能IDR帧
 *
 * @retval 零 成功
 * @retval 小于零 失败
 */
int IMP_Encoder_EnableIDR( ENC_CHN VeChn, bool bEnableIDR );

/**
 * IMP_Encoder_SetJpegSnapMode 设置JPEG抓拍通道的抓拍模式
 *
 * @fn int IMP_Encoder_SetJpegSnapMode(ENC_CHN VeChn, IMPEncoderJpegSnapModeEn enJpegSnapMode).
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
 * @param[in] enJpegSnapMode 通道抓拍模式
 *
 * @retval 零 成功
 * @retval 小于零 失败
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
int IMP_Encoder_SetJpegSnapMode(ENC_CHN VeChn, IMPEncoderJpegSnapModeEn enJpegSnapMode);

/**
 * IMP_Encoder_GetJpegSnapMode 获取JPEG抓拍通道的抓拍模式
 *
 * @fn int IMP_Encoder_GetJpegSnapMode(ENC_CHN VeChn, IMPEncoderJpegSnapModeEn *penJpegSnapMode).
 *
 * @param[in] VeChn 编码通道号,取值范围: [0, ENC_MAX_CHN_NUM)
 * @param[in] penJpegSnapMode 通道抓拍模式
 *
 * @retval 零 成功
 * @retval 小于零 失败
 *
 * @remarks 本接口必须在编码通道创建之后,编码通道销毁之前调用；
 * @remarks 本接口只对 JPEG 类型的编码通道有效。
 *
 */
int IMP_Encoder_GetJpegSnapMode(ENC_CHN VeChn, IMPEncoderJpegSnapModeEn *penJpegSnapMode);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_ENCODER_H__ */
