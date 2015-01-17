/*
 * IMP common venc data structure and macro header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Justin <pengtao.kang@ingenic.com>
 */

#ifndef __IMP_COMM_VENC_H__
#define __IMP_COMM_VENC_H__

/**
 * @file
 * venc data type interface.
 */

#include "imp_common.h"
#include "imp_errno.h"

#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/**
 * @def IMP_ERR_VENC_INVALID_DEVID
 * invlalid device ID
 */
#define IMP_ERR_VENC_INVALID_DEVID     IMP_DEF_ERR(ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_DEVID)

/**
 * @def IMP_ERR_VENC_INVALID_CHNID
 * invlalid channel ID
 */
#define IMP_ERR_VENC_INVALID_CHNID     IMP_DEF_ERR(ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_INVALID_CHNID)

/**
 * @def IMP_ERR_VENC_ILLEGAL_PARAM
 * at lease one parameter is illagal ,eg, an illegal enumeration value
 */
#define IMP_ERR_VENC_ILLEGAL_PARAM     IMP_DEF_ERR(ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)

/**
 * @def IMP_ERR_VENC_EXIST
 * channel exists
 */
#define IMP_ERR_VENC_EXIST             IMP_DEF_ERR(ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_EXIST)

/**
 * @def IMP_ERR_VENC_UNEXIST
 * channel exists
 */
#define IMP_ERR_VENC_UNEXIST           IMP_DEF_ERR(ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_UNEXIST)

/**
 * @def IMP_ERR_VENC_NULL_PTR
 * using a NULL point
 */
#define IMP_ERR_VENC_NULL_PTR          IMP_DEF_ERR(ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)

/**
 * @def IMP_ERR_VENC_NOT_CONFIG
 * try to enable or initialize system,device or channel, before configing attribute
 */
#define IMP_ERR_VENC_NOT_CONFIG        IMP_DEF_ERR(ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_CONFIG)

/**
 * @def IMP_ERR_VENC_NOT_SUPPORT
 * operation is not supported by NOW
 */
#define IMP_ERR_VENC_NOT_SUPPORT       IMP_DEF_ERR(ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SUPPORT)

/**
 * @def IMP_ERR_VENC_NOT_PERM
 * operation is not permitted ,eg, try to change stati attribute
 */
#define IMP_ERR_VENC_NOT_PERM          IMP_DEF_ERR(ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_PERM)

/**
 * @def IMP_ERR_VENC_NOMEM
 * failure caused by malloc memory
 */
#define IMP_ERR_VENC_NOMEM             IMP_DEF_ERR(ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)

/**
 * @def IMP_ERR_VENC_NOBUF
 * failure caused by malloc buffer
 */
#define IMP_ERR_VENC_NOBUF             IMP_DEF_ERR(ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_NOBUF)

/**
 * @def IMP_ERR_VENC_BUF_EMPTY
 * no data in buffer
 */
#define IMP_ERR_VENC_BUF_EMPTY         IMP_DEF_ERR(ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_EMPTY)

/**
 * @def IMP_ERR_VENC_BUF_FULL
 * no buffer for new data
 */
#define IMP_ERR_VENC_BUF_FULL          IMP_DEF_ERR(ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_BUF_FULL)

/**
 * @def IMP_ERR_VENC_SYS_NOTREADY
 * system is not ready,had not initialed or loaded
 */
#define IMP_ERR_VENC_SYS_NOTREADY      IMP_DEF_ERR(ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_SYS_NOTREADY)

/**
 * @def IMP_ERR_VENC_BUSY
 * system is busy
 */
#define IMP_ERR_VENC_BUSY              IMP_DEF_ERR(ID_VENC, EN_ERR_LEVEL_ERROR, EN_ERR_BUSY)

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
typedef enum ENUM_IMP_H264E_NALU_TYPE_E
{
     H264E_NALU_PSLICE = 1, /**< PSLICE 类型 */
     H264E_NALU_ISLICE = 5, /**< ISLICE 类型 */
     H264E_NALU_SEI    = 6, /**< SEI 类型 */
     H264E_NALU_SPS    = 7, /**< SPS 类型*/
     H264E_NALU_PPS    = 8, /**< PPS 类型 */
     H264E_NALU_BUTT
} H264E_NALU_TYPE_E;

/**
 * 定义获取的H264码流属于何种跳帧参考模式下的参考帧
 */
typedef enum ENUM_IMP_H264E_REFSLICE_TYPE_E
{
     H264E_REFSLICE_FOR_1X = 1, /**< 1倍跳帧参考时的参考帧 */
     H264E_REFSLICE_FOR_2X = 2, /**< 2倍跳帧参考时的参考帧 */
     H264E_REFSLICE_FOR_4X = 5, /**< 4倍跳帧参考时的参考帧 */
     H264E_REFSLICE_FOR_BUTT    /**< 非参考帧 */
} H264E_REFSLICE_TYPE_E;

/**
 * 定义JPEG码流的PACK类型
 */
typedef enum ENUM_IMP_JPEGE_PACK_TYPE_E
{
     JPEGE_PACK_ECS = 5, /**< ECS 类型 */
     JPEGE_PACK_APP = 6, /**< APP 类型 */
     JPEGE_PACK_VDO = 7, /**< VDO 类型 */
     JPEGE_PACK_PIC = 8, /**< PIC 类型 */
     JPEGE_PACK_BUTT
} JPEGE_PACK_TYPE_E;

/**
 * 定义编码码流类型
 */
typedef union UNION_IMP_VENC_DATA_TYPE_U
{
    H264E_NALU_TYPE_E    enH264EType; /**< H264E NALU 码流包类型 */
    JPEGE_PACK_TYPE_E    enJPEGEType; /**< JPEGE 码流包类型 */
}VENC_DATA_TYPE_U;

/**
 * 定义帧码流包结构体
 */
typedef struct IMP_VENC_PACK_S
{
    uint32_t	u32PhyAddr[2];      /**< 码流包物理地址 */
    uint8_t		*pu8Addr[2];        /**< 码流包虚拟地址 */
    uint32_t	u32Len[2];          /**< 码流包长度 */

    uint64_t	u64PTS;             /**< 时间戳，单位us */
    bool		bFieldEnd;          /**< 场结束标识 */
    bool		bFrameEnd;			/**< 帧结束标识 */

    VENC_DATA_TYPE_U  DataType;		/**< 码流类型 */
}VENC_PACK_S;

/**
 * 定义H264高级跳帧参考帧类型
 */
typedef enum ENUM_IMP_H264E_REF_TYPE_E
{
    BASE_IDRSLICE = 0,                  /**< 基本IDR帧或I帧 */
    BASE_PSLICE_REFBYBASE,              /**< 基本P帧，被其他基本P帧参考 */
    BASE_PSLICE_REFBYENHANCE,           /**< 基本P帧，被其他增强P帧参考 */
    ENHANCE_PSLICE_REFBYENHANCE,        /**< 增强P帧，被其他增强P帧参考 */
    ENHANCE_PSLICE_NOTFORREF,           /**< 增强P帧，不被参考 */
    ENHANCE_PSLICE_BUTT
} H264E_REF_TYPE_E;

/**
 * 定义H264协议码流特帧信息
 */
typedef struct IMP_VENC_STREAM_INFO_H264_S
{
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

    H264E_REFSLICE_TYPE_E enRefSliceType;	/**< 编码当前帧属于何种跳帧参考模式下的参考帧 */
    H264E_REF_TYPE_E      enRefType;	/**< 编码当前帧属于何种高级跳帧参考模式下的参考帧 */
    uint32_t	u32UpdateAttrCnt;		/**< 更新属性的次数 */
}VENC_STREAM_INFO_H264_S;

/**
 * 定义JPEG协议码流特征信息
 */
typedef struct IMP_VENC_STREAM_INFO_JPEG_S
{
    uint32_t	u32PicBytesNum;     /**< 一帧jpeg码流大小,以字节(byte)为单位 */
    uint32_t	u32UpdateAttrCnt;	/**< 更新属性的次数 */
}VENC_STREAM_INFO_JPEG_S;

/**
 * 定义帧码流类型结构体
 */
typedef struct IMP_VENC_STREAM_S
{
    VENC_PACK_S	*pstPack;       /**< 帧码流包结构 */
    uint32_t	u32PackCount;   /**< 一帧码流的所有包的个数 */
    uint32_t	u32Seq;         /**< 码流序列号，按帧获取帧序号，按包获取包序号 */

    union
    {
        VENC_STREAM_INFO_H264_S stH264Info;   /**< H264码流包特征信息 */
        VENC_STREAM_INFO_JPEG_S stJpegInfo;   /**< JPEG码流包特征信息 */
    };
}VENC_STREAM_S;

/**
 * 定义H264编码属性结构体
 */
typedef struct IMP_VENC_ATTR_H264_S
{
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
}VENC_ATTR_H264_S;

/**
 * 定义 JPEG 抓拍属性结构体
 */
typedef struct IMP_VENC_ATTR_JPEG_S
{
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

}VENC_ATTR_JPEG_S;

/**
 * 定义编码器属性结构体
 */
typedef struct IMP_VENC_ATTR_S
{
    PAYLOAD_TYPE_E	enType;      /**< 编码协议类型 */
    union
    {
        VENC_ATTR_H264_S  stAttrH264e;     /**< H264编码属性结构体 */
        VENC_ATTR_JPEG_S  stAttrJpeg;      /**< JPEG 抓拍属性结构体 */
    };
}VENC_ATTR_S;

/**
 * 定义编码通道属性结构体
 */
typedef struct IMP_VENC_CHN_ATTR_S
{
    VENC_ATTR_S		stVeAttr;		/**< 编码器属性结构体 */
    VENC_RC_ATTR_S	stRcAttr;		/**< 码率控制器属性结构体 */
}VENC_CHN_ATTR_S;

/**
 * 定义编码通道的状态结构体
 */
typedef struct IMP_VENC_CHN_STAT_S
{
    bool		bRegistered;			/**< 注册到通道组标志，取值范围:{TRUE, FALSE}，
										  TRUE:注册，FALSE:未注册 */
    uint32_t	u32LeftPics;			/**< 待编码的图像数 */
    uint32_t	u32LeftStreamBytes;		/**< 码流buffer剩余的byte数 */
    uint32_t	u32LeftStreamFrames;	/**< 码流buffer剩余的帧数 */
    uint32_t	u32CurPacks;			/**< 当前帧的码流包个数 */
    uint32_t	u32LeftRecvPics;		/**< 剩余待接收的帧数,在用户设置，IMP_VENC_StartRecvPicEx
										  后有效 */
    uint32_t	u32LeftEncPics;			/**< 剩余待编码的帧数,在用户设置，IMP_VENC_StartRecvPicEx
										  后有效 */
}VENC_CHN_STAT_S;

/**
 * H264编码能力属性结构体
 */
typedef struct IMP_VENC_H264_CAPABILITY_S
{
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
}VENC_H264_CAPABILITY_S;

/**
 * JPEG编码能力属性结构体
 */
typedef struct IMP_VENC_JPEG_CAPABILITY_S
{
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
}VENC_JPEG_CAPABILITY_S;

/**
 * 编码能力属性结构体
 */
typedef struct IMP_VENC_CAPABILITY_S
{
    PAYLOAD_TYPE_E enType;	 /**< 编码协议类型 */
    union
    {
        VENC_H264_CAPABILITY_S  stH264Cap; /**< H264编码能力属性结构体 */
        VENC_JPEG_CAPABILITY_S  stJpegCap; /**< JPEG编码能力属性结构体 */
    };
}VENC_CAPABILITY_S;

/**
 * 定义H.264协议编码通道SLICE分割结构体
 */
typedef struct IMP_VENC_PARAM_H264_SLICE_SPLIT_S
{
    bool		bSplitEnable;      /**< slice分割使能, TRUE:enable, FALSE:diable, 默认值:FALSE */
    uint32_t	u32SplitMode;      /**< slice分割模式，0:按bit数分割, 1:按宏块行分割, >=2:无意义 */
    uint32_t	u32SliceSize;      /**< 当u32SplitMode=0，表示每个slice的bit数，最小值为128,
									 最大值为min(0xffff，u32PicSize/2)，其中u32PicSize=PicWidth*
									 PicHeight*3/2；
									 当u32SplitMode=1，表示每个slice占的宏块行数，最小值为1，
									 最大值为(图像高+15)/16 */
} VENC_PARAM_H264_SLICE_SPLIT_S;

/**
 * 定义H.264协议编码通道帧间预测结构体
 */
typedef struct IMP_VENC_PARAM_H264_INTER_PRED_S
{
    uint32_t	u32HWSize;      /**< 水平搜索窗大小 */
    uint32_t	u32VWSize;      /**< 垂直搜索窗大小 */

    bool	bInter16x16PredEn;  /**< 16x16 帧间预测使能开关,默认使能 */
    bool	bInter16x8PredEn;   /**< 16x8 帧间预测使能开关,默认使能 */
    bool	bInter8x16PredEn;   /**< 8x16 帧间预测使能开关,默认使能 */
    bool	bInter8x8PredEn;    /**< 8x8 帧间预测使能开关,默认使能 */
    bool	bExtedgeEn;         /**< 当搜索遇见图像边界时,超出了图像范围,是否进行补搜的使
								  能开关,默认使能 */
} VENC_PARAM_H264_INTER_PRED_S;

/**
 * 定义 H.264 协议编码通道帧内预测结构体
 */
typedef struct IMP_VENC_PARAM_H264_INTRA_PRED_S
{
    bool	bIntra16x16PredEn;  /**< 16x16 帧内预测使能,默认使能，0:不使能，1:使能 */
    bool	bIntraNxNPredEn;    /**< NxN 帧内预测使能,默认使能，0:不使能，1:使能 */
    uint32_t constrained_intra_pred_flag; /**< 默认为 0，取值范围:0 或 1 */
    bool	bIpcmEn; /**< IPCM 预测使能，取值范围:0 或 1 */
}VENC_PARAM_H264_INTRA_PRED_S;

/**
 * 定义 H.264 协议编码通道变换、量化结构体
 */
typedef struct IMP_VENC_PARAM_H264E_TRANS_S
{
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
}VENC_PARAM_H264_TRANS_S;

/**
 * 定义 H.264 协议编码通道熵编码结构体
 */
typedef struct IMP_VENC_PARAM_H264E_ENTROPY_S
{
    uint32_t	u32EntropyEncModeI;         /**< I 帧熵编码模式,0:cavlc, 1:cabac,Baseline
											   不支持 cabac */
    uint32_t	u32EntropyEncModeP;         /**< P 帧熵编码模式,0:cavlc, 1:cabac,Baseline
											   不支持 cabac */
    uint32_t	cabac_stuff_en;             /**< 具体含义请参见 H.264 协议,系统默认为 0 */
    uint32_t	Cabac_init_idc;             /**< 取值范围[0, 2], 默认值 0 */
}VENC_PARAM_H264_ENTROPY_S;

/**
 * 定义H.264协议编码通道Poc结构体
 */
typedef struct IMP_VENC_PARAM_H264_POC_S
{
    uint32_t	pic_order_cnt_type; /**< 取值范围[0, 2],默认值2,具体含义请参见H.264协议 */

}VENC_PARAM_H264_POC_S;

/**
 * 定义H.264协议编码通道Dblk结构体
 */
typedef struct IMP_VENC_PARAM_H264E_DBLK_S
{
    uint32_t	disable_deblocking_filter_idc;   /**< 取值范围[0,2],默认值0,具体含义请参见H.264协议 */
    int			slice_alpha_c0_offset_div2;      /**< 取值范围[-6,6],默认值0,具体含义请参见
												   H.264协议 */
    int			slice_beta_offset_div2;          /**< 取值范围[-6,6],默认值0,具体含义请参见
												   H.264协议 */
}VENC_PARAM_H264_DBLK_S;

/**
 * 定义 H.264 协议编码通道 Vui 结构体
 */
typedef struct IMP_VENC_PARAM_H264E_VUI_S
{
    int		timing_info_present_flag;    /**< 具体含义请参见H.264协议,系统默认为0,取值范围:0或1 */
    int		num_units_in_tick;           /**< 具体含义请参见H.264协议,系统默认为1,取值范围:大于0 */
    int		time_scale;                  /**< 具体含义请参见H.264协议,系统默认为60,取值范围:大于0 */
    int		fixed_frame_rate_flag;       /**< 具体含义请参见H.264协议,系统默认为1,取值范围:0或1 */
}VENC_PARAM_H264_VUI_S;

/**
 * 定义JPEG协议编码通道高级参数结构体
 */
typedef struct IMP_VENC_PARAM_JPEG_S
{
    uint32_t	u32Qfactor;     /**< 具体含义请参见RFC2435协议,系统默认为90,取值范围:[1,99] */

    uint8_t		u8YQt[64];      /**< Y 量化表,取值范围:[1, 255] */
    uint8_t		u8CbQt[64];     /**< Cb 量化表,取值范围:[1, 255] */
    uint8_t		u8CrQt[64];     /**< Cb 量化表,取值范围:[1, 255] */

    uint32_t	u32MCUPerECS;   /**< 每个 ECS 中包含多少个 MCU,系统默认为 0,表示不划分Ecs,
									u32MCUPerECS:[0, (picwidth+15)>>4 x (picheight+15)>>4 x 2] */
} VENC_PARAM_JPEG_S;

/**
 * 定义编码感兴趣区域信息
 */
typedef struct IMP_VENC_ROI_CFG_S
{
    uint32_t	u32Index;   /**< ROI区域的索引,系统支持的索引范围为[0,7],不支持超出这个范围的索引 */
    bool		bEnable;    /**< 是否使能这个 ROI 区域 */
    bool		bAbsQp;     /**< ROI 区域 QP 模式,HI_FALSE:相对QP,HI_TURE:绝对QP */
    int			s32Qp;      /**< QP值,当QP模式为HI_FALSE时,s32Qp为QP偏移,s32Qp范围[-51,51],当QP
							  模式为HI_TRUE时,s32Qp为宏块QP值,s32Qp范围[0,51] */
    RECT_S		stRect;     /**< ROI区域,s32X、s32Y、u32Width、u32Height必须是16对齐 */
}VENC_ROI_CFG_S;

/**
 * 开启或关闭一个 GROUP 的彩转灰功能
 */
typedef struct IMP_GROUP_COLOR2GREY_S
{
    bool	bColor2Grey;    /**< 开启或关闭一个GROUP的彩转灰功能 */
}GROUP_COLOR2GREY_S;

/**
 * 彩转灰功能的全局配置参数
 */
typedef struct IMP_GROUP_COLOR2GREY_CONF_S
{
    bool		bEnable;		/**< 所有 GROUP 是否开启彩转灰功能的全局开关 */
    uint32_t	u32MaxWidth;	/**< 要开启彩转灰功能的GROUP的最大宽度。一个GROUP 的宽度就是
								  注册到其中的通道的宽度 */
    uint32_t	u32MaxHeight;	/**< 要开启彩转灰功能的GROUP的最大高度。一个GROUP 的高度就是
								  注册到其中的通道的高度*/
}GROUP_COLOR2GREY_CONF_S;

/**
 * 定义通道组截取(Crop)参数
 */
typedef struct IMP_GROUP_CROP_CFG_S
{
    bool bEnable;           /**< 是否进行裁剪,取值范围:[HI_FALSE, HI_TRUE],HI_TRUE:使能裁剪,
								HI_FALSE:不使能裁剪 */
    RECT_S  stRect;         /**< 裁剪的区域,stRect.s32X:必须16像素对齐,stRect.s32Y:无约束,
							  stRect.u32Width,s32Rect.u32Height,满足对应编码通道的宽高约束 */
}GROUP_CROP_CFG_S;

/**
 * 定义帧率设置结构体
 */
typedef struct IMP_GROUP_FRAME_RATE_S
{
    int		s32ViFrmRate;       /**< 通道组的输入帧率 */
    int		s32VpssFrmRate;     /**< 通道组的输出帧率 */
} GROUP_FRAME_RATE_S;

/**
 * 定义 H.264 编码的高级跳帧参考参数
 */
typedef struct IMP_VENC_ATTR_H264_REF_PARAM_S
{
    uint32_t	u32Base;		/**< base 层的周期 */
    uint32_t	u32Enhance;		/**< enhance 层的周期 */
    bool		bEnablePred;	/**< 代表base层的帧是否被base层其他帧用作参考;当为HI_FALSE时,
								  base层的所有帧都参考IDR帧 */
} VENC_ATTR_H264_REF_PARAM_S;

/**
 * 定义 H.264 编码的跳帧参考模式
 */
typedef enum ENUM_IMP_VENC_ATTR_H264_REF_MODE_E
{
    H264E_REF_MODE_1X = 1,	/**< 正常参考模式 */
    H264E_REF_MODE_2X = 2,	/**< 间隔 2 的跳帧参考模式 */
    H264E_REF_MODE_4X = 5,	/**< 间隔 4 的跳帧参考模式 */
    H264E_REF_MODE_BUTT,
}VENC_ATTR_H264_REF_MODE_E;

/**
 * JPEG 编码通道的抓拍模式
 */
typedef enum ENUM_IMP_VENC_JPEG_SNAP_MODE_E
{
    JPEG_SNAP_ALL   = 0,    /**< 全部抓拍模式,JPEG 通道的默认抓拍模式 */
    JPEG_SNAP_FLASH = 1,    /**< 闪光灯抓拍模式,JPEG 通道配合前端闪光灯时使用的抓拍模式 */
    JPEG_SNAP_BUTT,

}VENC_JPEG_SNAP_MODE_E;

/**
 * 接收图像参数
 */
typedef struct ENUM_IMP_VENC_RECV_PIC_PARAM_S
{
    int s32RecvPicNum;       /**< 编码通道连续接收并编码的帧数 */
} VENC_RECV_PIC_PARAM_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_COMM_VENC_H__ */
