#ifndef __COMIP_V4L2_H__
#define __COMIP_V4L2_H__

#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>

/* External v4l2 format info. */
#define V4L2_I2C_REG_MAX		(150)
#define V4L2_I2C_ADDR_16BIT		(0x0002)
#define V4L2_I2C_DATA_16BIT		(0x0004)

struct v4l2_i2c_reg {
	unsigned short addr;
	unsigned short data;
};

struct v4l2_fmt_data {
	unsigned short hts;
	unsigned short vts;
	unsigned short flags;
	unsigned short sensor_vts_address1;
	unsigned short sensor_vts_address2;
	unsigned short mipi_clk; /* Mbps. */
	unsigned char slave_addr;
	unsigned char reg_num;
	unsigned char viv_section;
	unsigned char viv_addr;
	unsigned char viv_len;
	struct v4l2_i2c_reg reg[V4L2_I2C_REG_MAX];
};

struct v4l2_mbus_framefmt_ext {
	__u32			width;
	__u32			height;
	__u32			code;
	__u32			field;
	__u32			colorspace;
	__u32			reserved[155];
};

struct v4l2_flash_enable {
	unsigned short on;
	unsigned short mode;
};

struct v4l2_flash_duration {
	unsigned int redeye_off_duration;
	unsigned int redeye_on_duration;
	unsigned int snapshot_pre_duration;
	unsigned int flash_ramp_time;
};

#define v4l2_get_fmt_data(fmt)		((struct v4l2_fmt_data *)(&(fmt)->reserved[0]))

/* External control IDs. */
#define V4L2_CID_ISO				(V4L2_CID_PRIVACY + 1)
#define V4L2_CID_EFFECT				(V4L2_CID_PRIVACY + 2)
#define V4L2_CID_PHOTOMETRY			(V4L2_CID_PRIVACY + 3)
#define V4L2_CID_FLASH_MODE			(V4L2_CID_PRIVACY + 4)
#define V4L2_CID_EXT_FOCUS			(V4L2_CID_PRIVACY + 5)
#define V4L2_CID_SCENE				(V4L2_CID_PRIVACY + 6)
#define V4L2_CID_SET_SENSOR			(V4L2_CID_PRIVACY + 7)
#define V4L2_CID_FRAME_RATE			(V4L2_CID_PRIVACY + 8)
#define V4L2_CID_SET_FOCUS			(V4L2_CID_PRIVACY + 9)	//set on or off
#define V4L2_CID_AUTO_FOCUS_RESULT		(V4L2_CID_PRIVACY + 10)
#define V4L2_CID_SET_FOCUS_RECT			(V4L2_CID_PRIVACY + 11)
#define V4L2_CID_SET_METER_RECT			(V4L2_CID_PRIVACY + 12)
#define V4L2_CID_GET_CAPABILITY 		(V4L2_CID_PRIVACY + 13)
#define V4L2_CID_SET_METER_MODE			(V4L2_CID_PRIVACY + 14)
#define V4L2_CID_SET_ANTI_SHAKE_CAPTURE		(V4L2_CID_PRIVACY + 15)
#define V4L2_CID_SET_HDR_CAPTURE		(V4L2_CID_PRIVACY + 16)
#define V4L2_CID_SET_FLASH			(V4L2_CID_PRIVACY + 17)
#define V4L2_CID_GET_AECGC_STABLE		(V4L2_CID_PRIVACY + 18)
#define V4L2_CID_GET_BRIGHTNESS_LEVEL		(V4L2_CID_PRIVACY + 19)
#define V4L2_CID_GET_PREFLASH_DURATION		(V4L2_CID_PRIVACY + 20)
#define V4L2_CID_SET_VIV_MODE		(V4L2_CID_PRIVACY + 21) //add by wayne 0604
#define V4L2_CID_SET_VIV_WIN_POSITION		(V4L2_CID_PRIVACY + 22) //add by wayne 0604
#define  V4L2_CID_SET_NIGHT_MODE  (V4L2_CID_PRIVACY + 23) //add by wayne 0604


//capability
struct v4l2_privacy_cap
{
	unsigned char buf[8];
};

#define V4L2_PRIVACY_CAP_FOCUS_AUTO			0
#define V4L2_PRIVACY_CAP_FOCUS_INFINITY			1
#define V4L2_PRIVACY_CAP_FOCUS_MACRO			2
#define V4L2_PRIVACY_CAP_FOCUS_FIXED			3
#define V4L2_PRIVACY_CAP_FOCUS_EDOF			4
#define V4L2_PRIVACY_CAP_FOCUS_CONTINUOUS_VIDEO		5
#define V4L2_PRIVACY_CAP_FOCUS_CONTINUOUS_PICTURE	6
#define V4L2_PRIVACY_CAP_FOCUS_CONTINUOUS_AUTO		7
#define V4L2_PRIVACY_CAP_METER_DOT			8
#define V4L2_PRIVACY_CAP_METER_MATRIX			9
#define V4L2_PRIVACY_CAP_METER_CENTER			10
#define V4L2_PRIVACY_CAP_FACE_DETECT			11
#define V4L2_PRIVACY_CAP_ANTI_SHAKE_CAPTURE		12
#define V4L2_PRIVACY_CAP_HDR_CAPTURE			13
#define V4L2_PRIVACY_CAP_FLASH				14


#define CAP_BITSET(cap, bit)	\
		do	{	\
			if (bit < sizeof((cap).buf) * 8)	{	\
				(cap).buf[bit >> 3] |= (1 << (bit % 8));	\
			}	\
		}while(0);

#define CAP_IS_BITSET(cap, bit) ((bit >= sizeof((cap).buf)* 8) ? 0 : (((cap).buf[bit >> 3] & (1 << (bit % 8))) ? 1 : 0))

enum v4l2_scene_mode {
	SCENE_MODE_AUTO = 0,
	SCENE_MODE_ACTION,
	SCENE_MODE_PORTRAIT,
	SCENE_MODE_LANDSCAPE,
	SCENE_MODE_NIGHT,
	SCENE_MODE_NIGHT_PORTRAIT,
	SCENE_MODE_THEATRE,
	SCENE_MODE_BEACH,
	SCENE_MODE_SNOW,
	SCENE_MODE_SUNSET,
	SCENE_MODE_STEADYPHOTO,
	SCENE_MODE_FIREWORKS,
	SCENE_MODE_SPORTS,
	SCENE_MODE_PARTY,
	SCENE_MODE_CANDLELIGHT,
	SCENE_MODE_BARCODE,
	SCENE_MODE_NORMAL,
	SCENE_MODE_MAX,
};

enum v4l2_flash_mode {
	FLASH_MODE_OFF = 0,
	FLASH_MODE_AUTO,
	FLASH_MODE_ON,
	FLASH_MODE_RED_EYE,
	FLASH_MODE_TORCH,
	FLASH_MODE_MAX,
};

enum v4l2_night_mode {
	NIGHT_MODE_OFF = 0,
	NIGHT_MODE_ON,
	NIGHT_MODE_MAX,
};

enum v4l2_wb_mode {
	WHITE_BALANCE_AUTO = 0,
	WHITE_BALANCE_INCANDESCENT,
	WHITE_BALANCE_FLUORESCENT,
	WHITE_BALANCE_WARM_FLUORESCENT,
	WHITE_BALANCE_DAYLIGHT,
	WHITE_BALANCE_CLOUDY_DAYLIGHT,
	WHITE_BALANCE_TWILIGHT,
	WHITE_BALANCE_SHADE,
	WHITE_BALANCE_MAX,
};

enum v4l2_effect_mode {
	EFFECT_NONE = 0,
	EFFECT_MONO,
	EFFECT_NEGATIVE,
	EFFECT_SOLARIZE,
	EFFECT_SEPIA,
	EFFECT_POSTERIZE,
	EFFECT_WHITEBOARD,
	EFFECT_BLACKBOARD,
	EFFECT_AQUA,
	//add by wayne
	EFFECT_BLUEISH,
	EFFECT_WARM,
	EFFECT_GREENISH,
	EFFECT_OVEREXPOSURE,
	EFFECT_MAX,
};

enum v4l2_iso_mode {
	ISO_AUTO = 0,
	ISO_100,
	ISO_200,
	ISO_400,
	ISO_800,
	ISO_MAX,
};

enum v4l2_exposure_mode {
	EXPOSURE_L3 = 0,
	EXPOSURE_L2,
	EXPOSURE_L1,
	EXPOSURE_H0,
	EXPOSURE_H1,
	EXPOSURE_H2,
	EXPOSURE_H3,
	EXPOSURE_MAX,
};

enum v4l2_contrast_mode {
	CONTRAST_L3 = 0,
	CONTRAST_L2,
	CONTRAST_L1,
	CONTRAST_H0,
	CONTRAST_H1,
	CONTRAST_H2,
	CONTRAST_H3,
	CONTRAST_MAX,
};

enum v4l2_saturation_mode {
	SATURATION_L3 = 0,
	SATURATION_L2,
	SATURATION_L1,
	SATURATION_H0,
	SATURATION_H1,
	SATURATION_H2,
	SATURATION_H3,
	SATURATION_MAX,
};

enum v4l2_sharpness_mode {
	SHARPNESS_L2 = 0,
	SHARPNESS_L1,
	SHARPNESS_H0,
	SHARPNESS_H1,
	SHARPNESS_H2,
	SHARPNESS_MAX,
};

enum v4l2_brightness_mode {
	BRIGHTNESS_L3 = 0,
	BRIGHTNESS_L2,
	BRIGHTNESS_L1,
	BRIGHTNESS_H0,
	BRIGHTNESS_H1,
	BRIGHTNESS_H2,
	BRIGHTNESS_H3,
	BRIGHTNESS_MAX,
};

enum v4l2_flicker_mode {
	FLICKER_AUTO = 0,
	FLICKER_50Hz,
	FLICKER_60Hz,
	FLICKER_OFF,
	FLICKER_MAX,
};

enum v4l2_zoom_level {
	ZOOM_LEVEL_0 = 0,
	ZOOM_LEVEL_1,
	ZOOM_LEVEL_2,
	ZOOM_LEVEL_3,
	ZOOM_LEVEL_4,
	ZOOM_LEVEL_5,
	ZOOM_LEVEL_MAX,
};

enum v4l2_focus_mode {
	FOCUS_MODE_AUTO = 0,
	FOCUS_MODE_INFINITY,
	FOCUS_MODE_MACRO,
	FOCUS_MODE_FIXED,
	FOCUS_MODE_EDOF,
	FOCUS_MODE_CONTINUOUS_VIDEO,
	FOCUS_MODE_CONTINUOUS_PICTURE,
	FOCUS_MODE_CONTINUOUS_AUTO,
	FOCUS_MODE_MAX,
};

enum v4l2_meter_mode {
	METER_MODE_CENTER = 0,
	METER_MODE_DOT,
	METER_MODE_MATRIX,
	METER_MODE_MAX,
};

enum v4l2_focus_status{
    AUTO_FOCUS_OFF = 0,
    AUTO_FOCUS_ON,
};

enum v4l2_viv_sub_window{
    VIV_LL = 0,
    VIV_LR,
    VIV_UL,
    VIV_UR,
};
enum v4l2_viv_status{
    VIV_OFF = 0,
    VIV_ON,
};

enum v4l2_frame_rate {
	FRAME_RATE_AUTO = 0,
	FRAME_RATE_7    = 7,
	FRAME_RATE_15   = 15,
	FRAME_RATE_30   = 30,
	FRAME_RATE_60   = 60,
	FRAME_RATE_120  = 120,
	FRAME_RATE_MAX,
};

#endif /*__COMIP_V4L2_H__*/
