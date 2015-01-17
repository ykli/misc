/**
 * IMP common data structure header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#ifndef __IMP_COMMON_H__
#define __IMP_COMMON_H__

/**
 * @file
 * Common data structures of SDK-level1.
 */

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#define VERSION_NAME_MAXLEN 64
typedef struct IMP_VERSION
{
	char aVersion[VERSION_NAME_MAXLEN];
} IMP_VERSION_t;

typedef int VENC_CHN;
typedef int VENC_GRP;

/**
 * Modules enumeration definitions.
 */
#define EMULATION
typedef enum {
	DEV_ID_VB,		/**< Video buffer */
	DEV_ID_SYS,		/**< Sys control */
	DEV_ID_VENC,		/**< Video Encoder */
	DEV_ID_VPSS,		/**< Video Process Sub-System */
	DEV_ID_VIU,		/**< Video Input */
	DEV_ID_ISP,		/**< ISP */
#ifdef EMULATION
	DEV_ID_EMU_SYS,		/**< System */
	DEV_ID_EMU_FS,		/**< FrameSource */
	DEV_ID_EMU_ENC,		/**< Encoder */
#endif
	NR_MAX_DEVICES,
} DeviceID;

/**
 * Channel data sturcture.
 */
typedef struct {
    /**
     * Device ID
     */
	DeviceID	devID;
    /**
     * Device ID
     */
	int			grpID;
    /**
     * Channel ID
     */
	int			chnID;
} IMPChannel;

typedef struct {
	int index;

	int width;
	int hight;
	int pixfmt;
	int size;

	uint32_t phyAddr;
	uint32_t virAddr;

	uint32_t timeStamp;
} FrameInfo;

/**
 * 编码协议类型,直接从RTP/RTSP协议中拷贝过来的
 */
typedef enum
{
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
}PAYLOAD_TYPE_E;

/**
 * 矩阵属性结构体
 */
typedef struct IMP_RECT_S
{
	int			s32X;		/**< 起点横坐标值 */
	int			s32Y;		/**< 起点纵坐标值 */
	uint32_t	u32Width;	/**< 宽度 */
	uint32_t	u32Height;	/**< 高度 */
}RECT_S;

/**
 * @typedef struct IMP_ISP_image_Attr_t
 *
 * 图片的属性
 */

typedef struct IMP_ISP_Image_Attr_t
{
	int frame_rate; /**< sensor帧率 */
} IMP_ISP_Image_Attr_t;

/**
 * @typedef struct IMP_VI_Stat_t
 *
 * VI通道的状态
 */

typedef struct IMP_VI_Stat_t
{
	int enable; /**< 通道是否使能 */
	int interrupt_count; /**< 中断计数 */
	int frame_rate; /**< 平均帧率 */
	int vb_fail_count; /**< vb失败计数 */
	int picture_width; /**< 图片宽度 */
	int picture_height; /**< 图片高度 */
} IMP_VI_Stat_t;

/**
 * @typedef struct IMP_VI_Frame_t
 *
 * VI通道的缓存帧
 */

typedef struct IMP_VI_Frame_t
{
	int picture_width; /**< 图片宽度 */
	int picture_height; /**< 图片高度 */
	int picture_type; /**< 图片格式 */
	int picture_time; /**< 图片的时间戳 */
	char *picture_data; /**< 图片数据 */
} IMP_VI_Frame_t;

/**
 * @typedef struct IMP_VI_Chn_Attr_t
 *
 * VI通道的属性
 */
typedef struct IMP_VI_Chn_Attr_t
{
	int picture_width; /**< 图片宽度 */
	int picture_height; /**< 图片高度 */
	int picture_type; /**< 图片格式 */
	int frame_rate; /**< 输出帧率 */
} IMP_VI_Chn_Attr_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_COMMON_H__ */
