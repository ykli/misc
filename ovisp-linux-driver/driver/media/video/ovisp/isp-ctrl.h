#ifndef __ISP_CTRL_H__
#define __ISP_CTRL_H__

#include "ovisp-isp.h"

extern int isp_set_exposure(struct isp_device *isp, int val);
extern int isp_set_iso(struct isp_device *isp, int val);
extern int isp_set_contrast(struct isp_device *isp, int val);
extern int isp_set_saturation(struct isp_device *isp, int val);
extern int isp_set_scene(struct isp_device *isp, int val);
extern int isp_set_effect(struct isp_device *isp, int val);
extern int isp_set_white_balance(struct isp_device *isp, int val);
extern int isp_set_brightness(struct isp_device *isp, int val);
extern int isp_set_sharpness(struct isp_device *isp, int val);
extern int isp_set_flicker(struct isp_device *isp, int val);
extern int isp_set_hflip(struct isp_device *isp, int val);
extern int isp_set_vflip(struct isp_device *isp, int val);

#endif/*__ISP_CTRL_H__*/
