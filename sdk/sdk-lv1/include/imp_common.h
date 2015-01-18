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
	int pool_idx;

	int width;
	int hight;
	int pixfmt;
	int size;

	uint32_t phyAddr;
	uint32_t virAddr;

	uint64_t timeStamp;
} FrameInfo;

typedef enum {
	PIX_FMT_YUV420P,   /**< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples) */
	PIX_FMT_YUYV422,   /**< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr */
	PIX_FMT_UYVY422,   /**< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1 */
	PIX_FMT_YUV422P,   /**< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples) */
	PIX_FMT_YUV444P,   /**< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples) */
	PIX_FMT_YUV410P,   /**< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples) */
	PIX_FMT_YUV411P,   /**< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples) */
	PIX_FMT_GRAY8,     /**<	   Y	    ,  8bpp */
	PIX_FMT_MONOWHITE, /**<	   Y	    ,  1bpp, 0 is white, 1 is black, in each byte pixels are ordered from the msb to the lsb */
	PIX_FMT_MONOBLACK, /**<	   Y	    ,  1bpp, 0 is black, 1 is white, in each byte pixels are ordered from the msb to the lsb */

	PIX_FMT_NV12,      /**< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V) */
	PIX_FMT_NV21,      /**< as above, but U and V bytes are swapped */

	PIX_FMT_RGB24,     /**< packed RGB 8:8:8, 24bpp, RGBRGB... */
	PIX_FMT_BGR24,     /**< packed RGB 8:8:8, 24bpp, BGRBGR... */

	PIX_FMT_ARGB,      /**< packed ARGB 8:8:8:8, 32bpp, ARGBARGB... */
	PIX_FMT_RGBA,	   /**< packed RGBA 8:8:8:8, 32bpp, RGBARGBA... */
	PIX_FMT_ABGR,	   /**< packed ABGR 8:8:8:8, 32bpp, ABGRABGR... */
	PIX_FMT_BGRA,	   /**< packed BGRA 8:8:8:8, 32bpp, BGRABGRA... */

	PIX_FMT_RGB565BE,  /**< packed RGB 5:6:5, 16bpp, (msb)	  5R 6G 5B(lsb), big-endian */
	PIX_FMT_RGB565LE,  /**< packed RGB 5:6:5, 16bpp, (msb)	  5R 6G 5B(lsb), little-endian */
	PIX_FMT_RGB555BE,  /**< packed RGB 5:5:5, 16bpp, (msb)1A 5R 5G 5B(lsb), big-endian, most significant bit to 0 */
	PIX_FMT_RGB555LE,  /**< packed RGB 5:5:5, 16bpp, (msb)1A 5R 5G 5B(lsb), little-endian, most significant bit to 0 */

	PIX_FMT_BGR565BE,  /**< packed BGR 5:6:5, 16bpp, (msb)	 5B 6G 5R(lsb), big-endian */
	PIX_FMT_BGR565LE,  /**< packed BGR 5:6:5, 16bpp, (msb)	 5B 6G 5R(lsb), little-endian */
	PIX_FMT_BGR555BE,  /**< packed BGR 5:5:5, 16bpp, (msb)1A 5B 5G 5R(lsb), big-endian, most significant bit to 1 */
	PIX_FMT_BGR555LE,  /**< packed BGR 5:5:5, 16bpp, (msb)1A 5B 5G 5R(lsb), little-endian, most significant bit to 1 */

	PIX_FMT_0RGB,      /**< packed RGB 8:8:8, 32bpp, 0RGB0RGB... */
	PIX_FMT_RGB0,	   /**< packed RGB 8:8:8, 32bpp, RGB0RGB0... */
	PIX_FMT_0BGR,	   /**< packed BGR 8:8:8, 32bpp, 0BGR0BGR... */
	PIX_FMT_BGR0,	   /**< packed BGR 8:8:8, 32bpp, BGR0BGR0... */

	PIX_FMT_BAYER_BGGR8,    ///< bayer, BGBG..(odd line), GRGR..(even line), 8-bit samples */
	PIX_FMT_BAYER_RGGB8,    ///< bayer, RGRG..(odd line), GBGB..(even line), 8-bit samples */
	PIX_FMT_BAYER_GBRG8,    ///< bayer, GBGB..(odd line), RGRG..(even line), 8-bit samples */
	PIX_FMT_BAYER_GRBG8,    ///< bayer, GRGR..(odd line), BGBG..(even line), 8-bit samples */

	PIX_FMT_NB,        /**< number of pixel formats, DO NOT USE THIS if you want to link with shared libav* because the number of formats might differ between versions */
} IMP_PixelFormat;

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
