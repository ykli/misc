#include "ovisp-isp.h"
#include "ovisp-csi.h"
#include "isp-i2c.h"
#include "isp-ctrl.h"
#include "isp-regs.h"
#include <linux/delay.h>
#include "isp-firmware_array.h"
#include "ovisp-video.h"
#include "ovisp-debugtool.h"

#define OVISP_ISP_DEBUG
#ifdef OVISP_ISP_DEBUG
#define ISP_PRINT(fmt, args...) printk(KERN_ERR "[ISP]" fmt, ##args)
#else
#define ISP_PRINT(fmt, args...) 
#endif

/* Timeouts. */
#define ISP_BOOT_TIMEOUT	(3000) /* ms. */
#define ISP_I2C_TIMEOUT		(3000) /* ms. */
#define ISP_ZOOM_TIMEOUT	(3000) /* ms. */
#define ISP_FORMAT_TIMEOUT	(3000) /* ms. */
#define ISP_CAPTURE_TIMEOUT	(8000) /* ms. */
#define ISP_OFFLINE_TIMEOUT	(8000) /* ms. */
#define iSP_BRACKET_TIMEOUT      (1000) /* ms */

/* Clock flags. */
#define ISP_CLK_MAIN		(0x00000001)
#define ISP_CLK_CSI		(0x00000002)
#define ISP_CLK_DEV		(0x00000004)
#define ISP_CLK_ALL		(0xffffffff)

/* CCLK divider. */
#define ISP_CCLK_DIVIDER	(0x04)

//static int isp_full_hdr_process(struct isp_device *isp, unsigned long short_addr,unsigned long long_addr, unsigned long target_addr);
static int isp_update_buffer(struct isp_device *isp, struct isp_buffer *buf);
static int isp_get_zoom_ratio(struct isp_device *isp, int zoom);

struct isp_clk_info {
	const char* name;
	unsigned long rate;
	unsigned long flags;
};

static struct isp_clk_info isp_clks[ISP_CLK_NUM] = {
	{"isp_cphy_cfg_clk",	26000000,  ISP_CLK_CSI},
	{"isp_axi_clk", 	156000000,  ISP_CLK_CSI},
	{"isp_p_sclk",		156000000,  ISP_CLK_MAIN | ISP_CLK_DEV},
	{"isp_sclk2",		156000000,  ISP_CLK_MAIN},
	{"isp_hclk",		200000000, ISP_CLK_MAIN},
};

static DEFINE_SPINLOCK(isp_suspend_lock);

static int isp_int_mask(struct isp_device *isp, unsigned char mask)
{
	unsigned long flags;

	spin_lock_irqsave(&isp->lock, flags);
	if (mask) {
		isp->intr &= ~mask;
		isp_reg_writeb(isp, isp->intr, REG_ISP_INT_EN);
	}
	spin_unlock_irqrestore(&isp->lock, flags);

	return 0;
}

static int isp_int_unmask(struct isp_device *isp, unsigned char mask)
{
	unsigned long flags;

	spin_lock_irqsave(&isp->lock, flags);
	if (mask) {
		isp->intr |= mask;
		isp_reg_writeb(isp, isp->intr, REG_ISP_INT_EN);
	}
	spin_unlock_irqrestore(&isp->lock, flags);

	return 0;
}

static unsigned char isp_int_state(struct isp_device *isp)
{
	return isp_reg_readb(isp, REG_ISP_INT_STAT);
}

static int isp_mac_int_mask(struct isp_device *isp, unsigned short mask)
{
	unsigned char mask_l = mask & 0x00ff;
	unsigned char mask_h = (mask >> 8) & 0x00ff;
	unsigned long flags;

	spin_lock_irqsave(&isp->lock, flags);
	if (mask_l) {
		isp->mac_intr_l &= ~mask_l;
		isp_reg_writeb(isp, isp->mac_intr_l, REG_ISP_MAC_INT_EN_L);
	}
	if (mask_h) {
		isp->mac_intr_h &= ~mask_h;
		isp_reg_writeb(isp, isp->mac_intr_h, REG_ISP_MAC_INT_EN_H);
	}
	spin_unlock_irqrestore(&isp->lock, flags);

	return 0;
}

static int isp_mac_int_unmask(struct isp_device *isp, unsigned short mask)
{
	unsigned char mask_l = mask & 0x00ff;
	unsigned char mask_h = (mask >> 8) & 0x00ff;
	unsigned long flags;

	spin_lock_irqsave(&isp->lock, flags);
	if (mask_l) {
		isp->mac_intr_l |= mask_l;
		isp_reg_writeb(isp, isp->mac_intr_l, REG_ISP_MAC_INT_EN_L);
	}
	if (mask_h) {
		isp->mac_intr_h |= mask_h;
		isp_reg_writeb(isp, isp->mac_intr_h, REG_ISP_MAC_INT_EN_H);
	}
	spin_unlock_irqrestore(&isp->lock, flags);

	return 0;
}

static unsigned short isp_mac_int_state(struct isp_device *isp)
{
	unsigned short state_l;
	unsigned short state_h;

	state_l = isp_reg_readb(isp, REG_ISP_MAC_INT_STAT_L);
	state_h = isp_reg_readb(isp, REG_ISP_MAC_INT_STAT_H);

	return (state_h << 8) | state_l;
}

static int isp_wait_cmd_done(struct isp_device *isp, unsigned long timeout)
{
	unsigned long tm;
	int ret = 0;

	tm = wait_for_completion_timeout(&isp->completion,
					msecs_to_jiffies(timeout));
	if (!tm && !isp->completion.done) {
		ret = -ETIMEDOUT;
	}

	return ret;
}


static int isp_send_cmd(struct isp_device *isp, unsigned char id,
				unsigned long timeout)
{
	int ret;

	INIT_COMPLETION(isp->completion);

	isp_int_unmask(isp, MASK_INT_CMDSET);
	isp_reg_writeb(isp, id, COMMAND_REG0);

	/* Wait for command set done interrupt. */
	ret = isp_wait_cmd_done(isp, timeout);

	isp_int_mask(isp, MASK_INT_CMDSET);

	return ret;
}

static int isp_set_address(struct isp_device *isp,
			unsigned int id, unsigned int addr)
{
	unsigned int reg = id ? REG_BASE_ADDR1 : REG_BASE_ADDR0;

	isp_reg_writeb(isp, (addr >> 24) & 0xff, reg);
	isp_reg_writeb(isp, (addr >> 16) & 0xff, reg + 1);
	isp_reg_writeb(isp, (addr >> 8) & 0xff, reg + 2);
	isp_reg_writeb(isp, (addr >> 0) & 0xff, reg + 3);

	return 0;
}



static int isp_calc_zoom(struct isp_device *isp)
{
	//function [ crop_w crop_h crop_x crop_y ratio_d ratio_dcw downscaleFlag dcwFlag] = GetScale (Wi, Wo, Hi, Ho)
	int crop_w = 0;
	int crop_h = 0;
	int crop_x = 0;
	int crop_y = 0;
	int downscale_w;
	int downscale_h;
	int dcw_w;
	int dcw_h;
	int Wi;
	int Wo;
	int Hi;
	int Ho;
	int ratio_h;
	int ratio_w;
	int ratio_gross;
	int ratio;
	int ratio_dcw;
	int dratio = 0;;
	int ratio_d = 0;;
	int ratio_up = 0;
	int dcwFlag =0;
	int downscaleFlag=0;
	int w_dcw;
	int h_dcw;
	int i;
	int j;
	int zoom_0;
	int zoom_ratio;
	int crop_width;
	int crop_height;
	int in_width;
	int in_height;
	int out_height;
	int out_width;
	int final_crop_width;
	int final_crop_height;
	int crop_x0 = 0;
	int crop_y0 = 0;
	int ret = 1;
	int t1;
	int t2;
	
	out_width = isp->parm.out_width;
	out_height = isp->parm.out_height;
	in_width = isp->parm.in_width;
	in_height = isp->parm.in_height;
	printk("in_width:%d;in_height:%d;out_width:%d;out_height:%d\n",in_width,in_height,out_width,out_height);
	zoom_0 = isp_get_zoom_ratio(isp, ZOOM_LEVEL_0);
	zoom_ratio = isp_get_zoom_ratio(isp, isp->parm.zoom);
	printk("zoom_0:%x;zoom_ratio:%x\n",zoom_0,zoom_ratio);
	crop_width = (zoom_0 * in_width)/zoom_ratio;
	crop_height = (zoom_0 * in_height)/zoom_ratio;

	if(((crop_width*1000)/crop_height) <= ((out_width*1000)/out_height)){
		final_crop_width = crop_width;
		final_crop_width = final_crop_width&0xfffe;
		final_crop_height = (final_crop_width * out_height)/out_width;
		final_crop_height = final_crop_height&0xfffe;
	}else{
		final_crop_height = crop_height;
		final_crop_height = final_crop_height&0xfffe;
		final_crop_width = (final_crop_height * out_width)/out_height;
		final_crop_width = final_crop_width&0xfffe;
	}
	crop_x0 = (in_width - final_crop_width)/2;
	crop_y0 = (in_height - final_crop_height)/2;

	Wo = isp->parm.out_width;
	Ho = isp->parm.out_height;
	Wi = final_crop_width;
        Hi = final_crop_height;
	printk("Wo %d;Ho %d;Wi %d;Hi %d\n",Wo,Ho,Wi,Hi);

	if(final_crop_width> isp->parm.out_width) {
		crop_w =crop_h =crop_x =crop_y =downscale_w =downscale_h =dcw_w =dcw_h = 0;
		ratio_h = (Hi*1000/Ho);
		ratio_w = (Wi*1000/Wo);
		ratio_gross = (ratio_h>= ratio_w) ? ratio_w:ratio_h;
		ratio = ratio_gross/1000;
		for (i=0;i<4;i++)
			if((1<<i)<=ratio && (1<<(i+1))>ratio)		
				break;
		if(i==4) i--;
		dcw_w = dcw_h = i;
		ratio_dcw = i;
		if(dcw_w == 0)
			dcwFlag = 0;
		else
			dcwFlag = 1;
		

		h_dcw = (1<<i)*Ho;
		w_dcw = (1<<i)*Wo;

		downscale_w = (256*w_dcw + Wi)/(2*Wi);
		downscale_h = (256*h_dcw + Hi)/(2*Hi);
		dratio = (downscale_w>=downscale_h)?downscale_w:downscale_h;
		if(dratio == 128)
			downscaleFlag = 0;
		else {
			downscaleFlag = 1;
			dratio += 1;
		}
		
		crop_w = (256*w_dcw + dratio)/(2*dratio);
		crop_h = (256*h_dcw + dratio)/(2*dratio);
		crop_w = crop_w&0xfffe;
		crop_h = crop_h&0xfffe;

	//update by wayne
		for(j=-3;j<=3;j++) {
			crop_w = (256*w_dcw + (dratio+j))/(2*(dratio+j));
			crop_h = (256*h_dcw + (dratio+j))/(2*(dratio+j));
			crop_w = crop_w&0xfffe;
			crop_h = crop_h&0xfffe;
				
			for(i=0;i<=4;i+=2) {	
				t1 = (crop_w+i)*(dratio+j)/128;
				t2 = (crop_h+i)*(dratio+j)/128;
			if((t1&0xfffe) == t1 && t1 >= w_dcw && (t2&0xfffe) == t2 && t2 >= h_dcw && (dratio+j)>=64 &&(dratio+j)<=128 && (crop_w +i)<= Wi && (crop_h+i)<= Hi)
				{
				ret = 0;
				break;
				}
			
			}		
			if(ret == 0)
				break;
		
	}
		if(j==4) j--;
		if(i==6) i = i-2;
		printk("i = %d,j = %d\n",i,j);
		crop_w += i;
		crop_h += i;
		dratio += j;
		//end
		crop_x = (Wi-crop_w)/2;
		crop_y = (Hi-crop_h)/2;

		ratio_d = dratio;
		}
	else {
		ratio_up= ((final_crop_height* 0x100)/isp->parm.out_height);
		crop_w =  final_crop_width;
		crop_h = final_crop_height;
	}

	isp->parm.ratio_up = ratio_up;
	isp->parm.ratio_d = ratio_d;
	isp->parm.ratio_dcw = ratio_dcw;
	isp->parm.crop_width = crop_w;
	isp->parm.crop_height = crop_h;
	
	isp->parm.crop_x = crop_x + crop_x0;
	isp->parm.crop_y = crop_y + crop_y0;
	isp->parm.dcwFlag = dcwFlag;
	isp->parm.dowscaleFlag = downscaleFlag;
	
printk("---------------------------------------------------------------------------\n");
printk("isp->parm.ratio_d= %d,isp->parm.ratio_dcw = %d\n",isp->parm.ratio_d,isp->parm.ratio_dcw);
printk("isp->parm.crop_width= %d,isp->parm.crop_height = %d\n",isp->parm.crop_width,isp->parm.crop_height);
printk("isp->parm.crop_x= %d,isp->parm.crop_y = %d\n",isp->parm.crop_x,isp->parm.crop_y);
printk("isp->parm.dcwFlag= %d,isp->parm.dowscaleFlag = %d\n",isp->parm.dcwFlag,isp->parm.dowscaleFlag);
printk("isp->parm.ratio_up= %d\n",isp->parm.ratio_up);
printk("---------------------------------------------------------------------------\n");
	
}


static int isp_set_parameters(struct isp_device *isp)
{
	struct ovisp_camera_client *client = isp->client;
	struct isp_parm *iparm = &isp->parm;
	struct ovisp_camera_dev *camera_dev;
	camera_dev = (struct ovisp_camera_dev *)(isp->data);
	unsigned char iformat;
	unsigned char oformat;
	u8 func_control;
	u32 snap_paddr;

	if(isp->hdr_mode==1) 
		snap_paddr = camera_dev->offline.paddr;
	else
		snap_paddr = isp->buf_start.paddr;

	if (iparm->in_format == V4L2_MBUS_FMT_YUYV8_2X8)
		iformat = IFORMAT_YUV422;
	else
		iformat = IFORMAT_RAW10;

	if (iparm->out_format == V4L2_PIX_FMT_NV21)
		oformat = OFORMAT_YUV420;
	else
		oformat = OFORMAT_YUV422;

	if (!isp->bypass)
		iformat |= ISP_PROCESS;

	//this is for capture raw data for image tuning
	#ifdef OVISP_DEBUGTOOL_ENABLE
	if (isp->snapshot) {
		u8 raw_flag = 0;
		raw_flag = ovisp_debugtool_get_flag(OVISP_DEBUGTOOL_GETRAW_FILENAME);
		if ('1' == raw_flag) {
			oformat = OFORMAT_RAW10;
			iformat &= (~ISP_PROCESS);
		}
	}
	#endif

	
	if (client->flags & CAMERA_CLIENT_IF_MIPI)
		iformat |= SENSOR_PRIMARY_MIPI;

	if(isp->snapshot){
		isp_reg_writeb(isp, (snap_paddr>>24)&0xff, ISP_BASE_ADDR_LEFT);
		isp_reg_writeb(isp, (snap_paddr>>16)&0xff, ISP_BASE_ADDR_LEFT+1);
		isp_reg_writeb(isp, (snap_paddr>>8)&0xff, ISP_BASE_ADDR_LEFT+2);
		isp_reg_writeb(isp, (snap_paddr>>0)&0xff, ISP_BASE_ADDR_LEFT+3);
	}

	isp_reg_writeb(isp, iformat, ISP_INPUT_FORMAT);
	isp_reg_writeb(isp, oformat, ISP_OUTPUT_FORMAT);

	isp_reg_writeb(isp, (iparm->in_width >> 8) & 0xff,
				SENSOR_OUTPUT_WIDTH);
	isp_reg_writeb(isp, iparm->in_width & 0xff,
				SENSOR_OUTPUT_WIDTH + 1);
	isp_reg_writeb(isp, (iparm->in_height >> 8) & 0xff,
				SENSOR_OUTPUT_HEIGHT);
	isp_reg_writeb(isp, iparm->in_height & 0xff,
				SENSOR_OUTPUT_HEIGHT + 1);

	isp_reg_writeb(isp, (iparm->in_width >> 8) & 0xff,
				ISP_INPUT_WIDTH);
	isp_reg_writeb(isp, iparm->in_width & 0xff,
				ISP_INPUT_WIDTH + 1);
	isp_reg_writeb(isp, (iparm->in_height >> 8) & 0xff,
				ISP_INPUT_HEIGHT);
	isp_reg_writeb(isp, iparm->in_height & 0xff,
				ISP_INPUT_HEIGHT + 1);

	isp_reg_writeb(isp, 0x00, ISP_INPUT_H_START);
	isp_reg_writeb(isp, 0x00, ISP_INPUT_H_START + 1);
	isp_reg_writeb(isp, 0x00, ISP_INPUT_V_START);
	isp_reg_writeb(isp, 0x00, ISP_INPUT_V_START + 1);

	isp_reg_writeb(isp, 0x00, ISP_INPUT_H_SENSOR_START);
	isp_reg_writeb(isp, 0x00, ISP_INPUT_H_SENSOR_START + 1);
	isp_reg_writeb(isp, 0x00, ISP_INPUT_V_SENSOR_START);
	isp_reg_writeb(isp, 0x00, ISP_INPUT_V_SENSOR_START + 1);

	isp_reg_writeb(isp, (iparm->out_width >> 8) & 0xff,
				ISP_OUTPUT_WIDTH);
	isp_reg_writeb(isp, iparm->out_width & 0xff,
				ISP_OUTPUT_WIDTH + 1);
	isp_reg_writeb(isp, (iparm->out_height >> 8) & 0xff,
				ISP_OUTPUT_HEIGHT);
	isp_reg_writeb(isp, iparm->out_height & 0xff,
				ISP_OUTPUT_HEIGHT + 1);

	isp_reg_writeb(isp, 0x00, ISP_INPUT_H_START_3D);
	isp_reg_writeb(isp, 0x00, ISP_INPUT_H_START_3D + 1);
	isp_reg_writeb(isp, 0x00, ISP_INPUT_V_START_3D);
	isp_reg_writeb(isp, 0x00, ISP_INPUT_V_START_3D + 1);
	isp_reg_writeb(isp, 0x00, ISP_INPUT_H_SENSOR_START_3D);
	isp_reg_writeb(isp, 0x00, ISP_INPUT_H_SENSOR_START_3D + 1);
	isp_reg_writeb(isp, 0x00, ISP_INPUT_V_SENSOR_START_3D);
	isp_reg_writeb(isp, 0x00, ISP_INPUT_V_SENSOR_START_3D + 1);

	isp_reg_writeb(isp, (iparm->out_width >> 8) & 0xff,
				MAC_MEMORY_WIDTH);
	isp_reg_writeb(isp, iparm->out_width & 0xff,
				MAC_MEMORY_WIDTH + 1);
	
	isp_reg_writeb(isp, 0x00, ISP_INPUT_MODE);

	if(isp->hdr_mode!=1||!isp->snapshot) {		
		isp_calc_zoom(isp);

		func_control = 0;
		if((iparm->crop_width != iparm->in_width) || (iparm->crop_height != iparm->in_height))
			func_control |= 1;
		if(iparm->crop_width > iparm->out_width){
			func_control |= (1<<1);
		
			isp_reg_writeb(isp, func_control, ISP_FUNCTION_CTRL);			
				isp_reg_writeb(isp, iparm->ratio_dcw, ISP_SCALE_DOWN_H_RATIO1);
				isp_reg_writeb(isp, iparm->ratio_dcw, ISP_SCALE_DOWN_V_RATIO1);	
				isp_reg_writeb(isp,  iparm->ratio_d, ISP_SCALE_DOWN_H_RATIO2);
				isp_reg_writeb(isp,   iparm->ratio_d, ISP_SCALE_DOWN_V_RATIO2);

			isp_reg_writeb(isp, 0x00, ISP_SCALE_UP_H_RATIO);
			isp_reg_writeb(isp, 0x00, ISP_SCALE_UP_H_RATIO + 1);
			isp_reg_writeb(isp, 0x00, ISP_SCALE_UP_V_RATIO);
			isp_reg_writeb(isp, 0x00, ISP_SCALE_UP_V_RATIO + 1);
			
		}else if(iparm->crop_width < iparm->out_width){
			func_control |= (2<<1);

			isp_reg_writeb(isp, func_control, ISP_FUNCTION_CTRL);
			isp_reg_writeb(isp, 0x00, ISP_SCALE_DOWN_H_RATIO1);
			isp_reg_writeb(isp, 0x00, ISP_SCALE_DOWN_H_RATIO2);
			isp_reg_writeb(isp, 0x00, ISP_SCALE_DOWN_V_RATIO1);
			isp_reg_writeb(isp, 0x00, ISP_SCALE_DOWN_V_RATIO2);
			isp_reg_writeb(isp, ((iparm->ratio_up) >> 8)&0xff, ISP_SCALE_UP_H_RATIO);
			isp_reg_writeb(isp, (iparm->ratio_up)&0xff, ISP_SCALE_UP_H_RATIO + 1);
			isp_reg_writeb(isp, ((iparm->ratio_up) >> 8)&0xff, ISP_SCALE_UP_V_RATIO);
			isp_reg_writeb(isp, (iparm->ratio_up)&0xff, ISP_SCALE_UP_V_RATIO + 1);

		}else{
			isp_reg_writeb(isp, func_control, ISP_FUNCTION_CTRL);
		}

	}
	else {

			isp_reg_writeb(isp, 0x00, ISP_INPUT_MODE);
			isp_reg_writeb(isp, 0x00, ISP_FUNCTION_CTRL);
			isp_reg_writeb(isp, 0x00, ISP_SCALE_DOWN_H_RATIO1);
			isp_reg_writeb(isp, 0x80, ISP_SCALE_DOWN_H_RATIO2);
			isp_reg_writeb(isp, 0x00, ISP_SCALE_DOWN_V_RATIO1);
			isp_reg_writeb(isp, 0x80, ISP_SCALE_DOWN_V_RATIO2);
			isp_reg_writeb(isp, 0x00, ISP_SCALE_UP_H_RATIO);
			isp_reg_writeb(isp, 0x00, ISP_SCALE_UP_H_RATIO + 1);
			isp_reg_writeb(isp, 0x00, ISP_SCALE_UP_V_RATIO);
			isp_reg_writeb(isp, 0x00, ISP_SCALE_UP_V_RATIO + 1);
	}

	
	isp_reg_writeb(isp, (iparm->crop_x >> 8) & 0xff,
				ISP_YUV_CROP_H_START);
	isp_reg_writeb(isp, iparm->crop_x & 0xff,
				ISP_YUV_CROP_H_START + 1);
	isp_reg_writeb(isp, (iparm->crop_y >> 8) & 0xff,
				ISP_YUV_CROP_V_START);
	isp_reg_writeb(isp, iparm->crop_y & 0xff,
				ISP_YUV_CROP_V_START + 1);
	isp_reg_writeb(isp, (iparm->crop_width >> 8) & 0xff,
				ISP_YUV_CROP_WIDTH);
	isp_reg_writeb(isp, iparm->crop_width & 0xff,
				ISP_YUV_CROP_WIDTH + 1);
	isp_reg_writeb(isp, (iparm->crop_height >> 8) & 0xff,
				ISP_YUV_CROP_HEIGHT);
	isp_reg_writeb(isp, iparm->crop_height & 0xff,
				ISP_YUV_CROP_HEIGHT + 1);

	isp_reg_writeb(isp, 0x00, ISP_MAX_GAIN);
	isp_reg_writeb(isp, 0xff, ISP_MAX_GAIN + 1);
	isp_reg_writeb(isp, 0x00, ISP_MIN_GAIN);
	isp_reg_writeb(isp, 0x10, ISP_MIN_GAIN + 1);

	isp_reg_writeb(isp, (isp->fmt_data.vts - 0x20) >> 8, ISP_MAX_EXPOSURE);
	isp_reg_writeb(isp, (isp->fmt_data.vts - 0x20) & 0xff, ISP_MAX_EXPOSURE + 1);
	isp_reg_writeb(isp, 0x00, ISP_MIN_EXPOSURE);
	isp_reg_writeb(isp, 0x01, ISP_MIN_EXPOSURE + 1);
	isp_reg_writeb(isp, isp->fmt_data.vts >> 8, ISP_VTS);
	isp_reg_writeb(isp, isp->fmt_data.vts & 0xff, ISP_VTS + 1);

	if (!isp->snapshot) {
		isp_reg_writeb(isp, 0x01, ISP_EXPOSURE_RATIO);
		isp_reg_writeb(isp, 0x73, ISP_EXPOSURE_RATIO + 1);
		
		/* this shold be modified if the width*height is different*/
		isp_reg_writeb(isp, 0x0f, ISP_BANDING_60HZ);
		isp_reg_writeb(isp, 0x50, ISP_BANDING_60HZ + 1);
		isp_reg_writeb(isp, 0x12, ISP_BANDING_50HZ);
		isp_reg_writeb(isp, 0x60, ISP_BANDING_50HZ + 1);
	}
	else
	{	
		if(isp->hdr_mode==1) {
			isp_reg_writeb(isp, 0x00, ISP_EXPOSURE_RATIO);
			isp_reg_writeb(isp, 0x1d, ISP_EXPOSURE_RATIO + 1);
			isp_reg_writeb(isp, 0x08, BRACKET_RATIO1);
			isp_reg_writeb(isp, 0x00, BRACKET_RATIO1 + 1);
		}
			else {
			isp_reg_writeb(isp, 0x00, ISP_EXPOSURE_RATIO);
			isp_reg_writeb(isp, 0xB0, ISP_EXPOSURE_RATIO + 1);
		}

		/* this shold be modified if the width*height is different*/
		isp_reg_writeb(isp, 0x0a, ISP_BANDING_60HZ);
		isp_reg_writeb(isp, 0x96, ISP_BANDING_60HZ + 1);
		isp_reg_writeb(isp, 0x0c, ISP_BANDING_50HZ);
		isp_reg_writeb(isp, 0xb5, ISP_BANDING_50HZ + 1);
	}

	return 0;
}

static int isp_i2c_config(struct isp_device *isp)
{
	unsigned char val;

	if (isp->pdata->flags & CAMERA_I2C_FAST_SPEED)
		val = I2C_SPEED_200;
	else
		val = I2C_SPEED_100;

	isp_reg_writeb(isp, val, REG_SCCB_MAST1_SPEED);

	return 0;
}

static int isp_i2c_xfer_cmd(struct isp_device *isp, struct isp_i2c_cmd *cmd)
{
	unsigned char val = 0;

	isp_reg_writew(isp, cmd->reg, COMMAND_BUFFER);
	if (!(cmd->flags & I2C_CMD_READ)) {
		if (cmd->flags & I2C_CMD_DATA_16BIT) {
			isp_reg_writew(isp, cmd->data, COMMAND_BUFFER + 2);
		} else {
			isp_reg_writeb(isp, cmd->data & 0xff, COMMAND_BUFFER + 2);
			isp_reg_writeb(isp, 0xff, COMMAND_BUFFER + 3);
		}
	}

	val |= SELECT_I2C_PRIMARY;
	if (!(cmd->flags & I2C_CMD_READ))
		val |= SELECT_I2C_WRITE;
	if (cmd->flags & I2C_CMD_ADDR_16BIT)
		val |= SELECT_I2C_16BIT_ADDR;
	if (cmd->flags & I2C_CMD_DATA_16BIT)
		val |= SELECT_I2C_16BIT_DATA;

	isp_reg_writeb(isp, val, COMMAND_REG1);
	isp_reg_writeb(isp, cmd->addr, COMMAND_REG2);
	isp_reg_writeb(isp, 0x01, COMMAND_REG3);

	/* Wait for command set done interrupt. */
	if (isp_send_cmd(isp, CMD_I2C_GRP_WR, ISP_I2C_TIMEOUT)) {
		printk(KERN_ERR "Failed to wait i2c set done (%02x)!\n",
			isp_reg_readb(isp, REG_ISP_INT_EN));
		return -ETIME;
	}

	if ((CMD_SET_SUCCESS != isp_reg_readb(isp, COMMAND_RESULT))
		|| (CMD_I2C_GRP_WR != isp_reg_readb(isp, COMMAND_FINISHED))) {
		printk(KERN_ERR "Failed to write sequeue to I2C (%02x:%02x)\n",
			isp_reg_readb(isp, COMMAND_RESULT),
			isp_reg_readb(isp, COMMAND_FINISHED));
		return -EINVAL;
	}

	if (cmd->flags & I2C_CMD_READ) {
		if (cmd->flags & I2C_CMD_DATA_16BIT)
			cmd->data = isp_reg_readw(isp, COMMAND_BUFFER + 2);
		else
			cmd->data = isp_reg_readb(isp, COMMAND_BUFFER + 3);
	}

	return 0;
}

static int isp_i2c_fill_buffer(struct isp_device *isp)
{
	struct v4l2_fmt_data *data = &isp->fmt_data;
	unsigned char val = 0;
	unsigned char i;
	
	for (i = 0; i < data->reg_num; i++) {
		isp_reg_writew(isp, data->reg[i].addr, COMMAND_BUFFER + i * 4);
		if (data->flags & I2C_CMD_DATA_16BIT) {
			isp_reg_writew(isp, data->reg[i].data,
						COMMAND_BUFFER + i * 4 + 2);
		} else {
			isp_reg_writeb(isp, data->reg[i].data & 0xff,
						COMMAND_BUFFER + i * 4 + 2);
			isp_reg_writeb(isp, 0xff, COMMAND_BUFFER + i * 4 + 3);
		}
	}

	if (data->reg_num) {
		val |= SELECT_I2C_PRIMARY | SELECT_I2C_WRITE;
		if (data->flags & V4L2_I2C_ADDR_16BIT)
			val |= SELECT_I2C_16BIT_ADDR;
		if (data->flags & V4L2_I2C_DATA_16BIT)
			val |= SELECT_I2C_16BIT_DATA;

		isp_reg_writeb(isp, val, COMMAND_REG1);
		isp_reg_writeb(isp, data->slave_addr << 1, COMMAND_REG2);
		isp_reg_writeb(isp, data->reg_num, COMMAND_REG3);
	} else {
		isp_reg_writeb(isp, 0x00, COMMAND_REG1);
		isp_reg_writeb(isp, 0x00, COMMAND_REG2);
		isp_reg_writeb(isp, 0x00, COMMAND_REG3);
	}
	if(!isp->bypass&&!isp->debug.status&&!isp->snapshot) {
		isp_reg_writeb(isp, 0x00, COMMAND_REG1);
		isp_reg_writeb(isp, 0x00, COMMAND_REG2);
		isp_reg_writeb(isp, 0x00, COMMAND_REG3);

	}

	return 0;
}

static int isp_set_format(struct isp_device *isp)
{	
	isp_int_unmask(isp, MASK_INT_CMDSET);
	isp_i2c_fill_buffer(isp);
	isp_reg_writeb(isp, ISP_CCLK_DIVIDER, COMMAND_REG4);

#endif
	 if(isp->first_init){
		isp->first_init = 0;
		isp_reg_writeb(isp, 0x80, COMMAND_REG5);
	}
	else if(isp->snapshot){
		isp_reg_writeb(isp, 0x00, COMMAND_REG5);
	}
	else {
		isp_reg_writeb(isp, 0x00, COMMAND_REG5);
	}
	/* Wait for command set done interrupt. */
	if (isp_send_cmd(isp, CMD_SET_FORMAT, ISP_FORMAT_TIMEOUT)) {
		printk(KERN_ERR "Failed to wait format set done!\n");
		return -ETIME;
	}

	if ((CMD_SET_SUCCESS != isp_reg_readb(isp, COMMAND_RESULT))
		|| (CMD_SET_FORMAT != isp_reg_readb(isp, COMMAND_FINISHED))) {
		printk(KERN_ERR "Failed to set format (%02x:%02x)\n",
			isp_reg_readb(isp, COMMAND_RESULT),
			isp_reg_readb(isp, COMMAND_FINISHED));
		return -EINVAL;
	}

	return 0;
}

static int isp_set_capture(struct isp_device *isp)
{	
	struct ovisp_camera_dev *camera_dev;
	camera_dev = (struct ovisp_camera_dev *)(isp->data);
	isp_i2c_fill_buffer(isp);
	isp_reg_writeb(isp, ISP_CCLK_DIVIDER, COMMAND_REG4);

	isp_reg_writeb(isp, 0x40, COMMAND_REG5);

	/* Wait for command set done interrupt. */
	if (isp_send_cmd(isp, CMD_CAPTURE, ISP_CAPTURE_TIMEOUT)) {
		printk(KERN_ERR "Failed to wait capture set done!\n");
		return -ETIME;
	}

	if ((CMD_SET_SUCCESS != isp_reg_readb(isp, COMMAND_RESULT))
		|| (CMD_CAPTURE != isp_reg_readb(isp, COMMAND_FINISHED))) {
		printk(KERN_ERR "Failed to set capture (%02x:%02x)\n",
			isp_reg_readb(isp, COMMAND_RESULT),
			isp_reg_readb(isp, COMMAND_FINISHED));
		return -EINVAL;
	}
	return 0;
}

static int isp_get_zoom_ratio(struct isp_device *isp, int zoom)
{
	int zoom_ratio;

	switch (zoom) {
	case ZOOM_LEVEL_5:
		zoom_ratio = 0x350;
		break;
	case ZOOM_LEVEL_4:
		zoom_ratio = 0x300;
		break;
	case ZOOM_LEVEL_3:
		zoom_ratio = 0x250;
		break;
	case ZOOM_LEVEL_2:
		zoom_ratio = 0x200;
		break;
	case ZOOM_LEVEL_1:
		zoom_ratio = 0x140;
		break;
	case ZOOM_LEVEL_0:
	default:
		zoom_ratio = 0x100;
		break;
	}

	return zoom_ratio;	
}

static int isp_set_zoom(struct isp_device *isp, int zoom)
{
	int zoom_ratio;

	zoom_ratio = isp_get_zoom_ratio(isp, zoom);

	isp_reg_writeb(isp, 0x01, COMMAND_REG1);
	isp_reg_writeb(isp, zoom_ratio >> 8, COMMAND_REG2);
	isp_reg_writeb(isp, zoom_ratio & 0xFF, COMMAND_REG3);
	isp_reg_writeb(isp, zoom_ratio >> 8, COMMAND_REG4);
	isp_reg_writeb(isp, zoom_ratio & 0xFF, COMMAND_REG5);

	/* Wait for command set done interrupt. */
	if (isp_send_cmd(isp, CMD_ZOOM_IN_MODE, ISP_ZOOM_TIMEOUT)) {
		printk(KERN_ERR "Failed to wait zoom set done!\n");
		return -ETIME;
	}

	if ((CMD_SET_SUCCESS != isp_reg_readb(isp, COMMAND_RESULT))
		|| (CMD_ZOOM_IN_MODE != isp_reg_readb(isp, COMMAND_FINISHED))) {
		printk(KERN_ERR "Failed to set zoom (%02x:%02x)\n",
			isp_reg_readb(isp, COMMAND_RESULT),
			isp_reg_readb(isp, COMMAND_FINISHED));
		return -EINVAL;
	}

	return 0;
}

static int isp_boot(struct isp_device *isp)
{
	unsigned char val;

	if (isp->boot)
		return 0;

	/* Mask all interrupts. */
	isp_int_mask(isp, 0xff);
	isp_mac_int_mask(isp, 0xffff);

	/* Reset ISP.  */
	isp_reg_writeb(isp, DO_SOFT_RST, REG_ISP_SOFT_RST);

	/* Enable interrupt (only set_cmd_done interrupt). */
	isp_int_unmask(isp, MASK_INT_CMDSET);

	isp_reg_writeb(isp, DO_SOFTWARE_STAND_BY, REG_ISP_SOFT_STANDBY);

	/* Enable the clk used by mcu. */
	isp_reg_writeb(isp, 0xf1, REG_ISP_CLK_USED_BY_MCU);

	/* Download firmware to ram of mcu. */
	#ifdef OVISP_DEBUGTOOL_ENABLE
	ovisp_debugtool_load_firmware(OVISP_DEBUGTOOL_FIRMWARE_FILENAME, (u8*)(isp->base + FIRMWARE_BASE), isp_firmware, ARRAY_SIZE(isp_firmware));
	#else
	memcpy((unsigned char *)(isp->base + FIRMWARE_BASE),
			isp_firmware,
			ARRAY_SIZE(isp_firmware));
	#endif

	/* MCU initialize. */
	isp_reg_writeb(isp, 0xf0, REG_ISP_CLK_USED_BY_MCU);

	/* Wait for command set done interrupt. */
	if (isp_wait_cmd_done(isp, ISP_BOOT_TIMEOUT)) {
		printk(KERN_ERR "MCU not respond when init ISP!\n");
		return -ETIME;
	}

	val = isp_reg_readb(isp, COMMAND_FINISHED);
	if (val != CMD_FIRMWARE_DOWNLOAD) {
		printk(KERN_ERR "Failed to download isp firmware (%02x)\n",
			val);
		return -EINVAL;
	}

	isp_reg_writeb(isp, DO_SOFTWARE_STAND_BY, REG_ISP_SOFT_STANDBY);

	isp_i2c_config(isp);

	isp_reg_writeb(isp, 0x01, 0x63002);// 1: 2lane, 2: 4lane; MIPI1: 0x63007
	isp_reg_writeb(isp, ISP_CCLK_DIVIDER, 0x63023);
	isp_reg_writeb(isp, 0x04, 0x63b35);//uyvy -> yuyv
//	isp_reg_writeb(isp, 0x08, 0x63927);
//	isp_reg_writeb(isp, 0x00, 0x63108);

	return 0;
}

static int isp_irq_notify(struct isp_device *isp, unsigned int status)
{
	int ret = 0;

	if (isp->irq_notify)
		ret = isp->irq_notify(status, isp->data);

	return ret;
}

static irqreturn_t isp_irq(int this_irq, void *dev_id)
{
	struct isp_device *isp = dev_id;
	unsigned char irq_status;
	unsigned short mac_irq_status = 0;
	unsigned int notify = 0;

	irq_status = isp_int_state(isp);
	if (irq_status & MASK_INT_MAC)
		mac_irq_status = isp_mac_int_state(isp);

	/* Command set done interrupt. */
	if (irq_status & MASK_INT_CMDSET)
		complete(&isp->completion);

	/* Drop. */
	if (mac_irq_status & (MASK_INT_DROP0 | MASK_INT_DROP1)){
		notify |= ISP_NOTIFY_DROP_FRAME;
		if(mac_irq_status & MASK_INT_DROP0){
			isp->pp_buf = true;
		}
		if(mac_irq_status & MASK_INT_DROP1){
			isp->pp_buf = false;
		}
		if(!isp->snapshot)
			isp_update_buffer(isp, &isp->buf_start);
	}
	/* Done. */
	if (mac_irq_status & (MASK_INT_WRITE_DONE0 | MASK_INT_WRITE_DONE1)){
		notify |= ISP_NOTIFY_DATA_DONE;
		if (mac_irq_status & MASK_INT_WRITE_DONE0)
			isp->pp_buf = false;
		else if (mac_irq_status & MASK_INT_WRITE_DONE1)
			isp->pp_buf = true;
	}
	/* FIFO overflow */
	if (mac_irq_status & (MASK_INT_OVERFLOW0 | MASK_INT_OVERFLOW1))
		notify |= ISP_NOTIFY_OVERFLOW;
	/* Frame start. */
	if (mac_irq_status & MASK_INT_FRAME_START)
		notify |= ISP_NOTIFY_DATA_START;

	isp_irq_notify(isp, notify);

	return IRQ_HANDLED;
}

static int isp_mipi_init(struct isp_device *isp)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(isp_mipi_regs_init); i++) {
		isp_reg_writeb(isp, isp_mipi_regs_init[i].value,
					isp_mipi_regs_init[i].reg);
	}

	#ifdef OVISP_DEBUGTOOL_ENABLE
	ovisp_debugtool_load_isp_setting(isp, OVISP_DEBUGTOOL_ISPSETTING_FILENAME);
	#endif

	return 0;
}

static int isp_dvp_init(struct isp_device *isp)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(isp_dvp_regs_init); i++) {
		isp_reg_writeb(isp, isp_dvp_regs_init[i].value,
					isp_dvp_regs_init[i].reg);
	}

	return 0;
}

static int isp_int_init(struct isp_device *isp)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&isp->lock, flags);

	isp->intr = 0;
	isp->mac_intr_l = 0;
	isp->mac_intr_h = 0;
	isp_reg_writeb(isp, 0x00, REG_ISP_INT_EN);
	isp_reg_writeb(isp, 0x00, REG_ISP_MAC_INT_EN_L);
	isp_reg_writeb(isp, 0x00, REG_ISP_MAC_INT_EN_H);

	spin_unlock_irqrestore(&isp->lock, flags);

	ret = request_irq(isp->irq, isp_irq, IRQF_SHARED,
			  "isp", isp);

	return ret;
}

static int isp_i2c_init(struct isp_device *isp)
{
	struct ovisp_i2c_platform_data *pdata = isp->i2c.pdata;
	int ret;

	if (isp->pdata->flags & CAMERA_USE_ISP_I2C) {
		ovisp_mfp_config(MFP_PIN_GPIO85, MFP_GPIO85_ISP_I2C_SCL);
		ovisp_mfp_config(MFP_PIN_GPIO86, MFP_GPIO86_ISP_I2C_SDA);
	} else {
		ovisp_mfp_config(MFP_PIN_GPIO85, MFP_GPIO85_I2C3_SCL);
		ovisp_mfp_config(MFP_PIN_GPIO86, MFP_GPIO86_I2C3_SDA);
	}

	if (isp->pdata->flags & CAMERA_USE_ISP_I2C) {
		isp->i2c.xfer_cmd = isp_i2c_xfer_cmd;
		ret = isp_i2c_register(isp);
		if (ret)
			return ret;
	} else {
		pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
		if (!pdata)
			return -ENOMEM;

		if (isp->pdata->flags & CAMERA_I2C_PIO_MODE)
			pdata->use_pio = 1;

		if (isp->pdata->flags & CAMERA_I2C_HIGH_SPEED)
			pdata->flags = OVISP_I2C_HIGH_SPEED_MODE;
		else if (isp->pdata->flags & CAMERA_I2C_FAST_SPEED)
			pdata->flags = OVISP_I2C_FAST_MODE;
		else
			pdata->flags = OVISP_I2C_STANDARD_MODE;

		ovisp_set_i2c3_info(pdata);
	}

	return 0;
}

static int isp_i2c_release(struct isp_device *isp)
{
	if (isp->pdata->flags & CAMERA_USE_ISP_I2C) {
		isp_i2c_unregister(isp);
	} else {
		platform_device_register(to_platform_device(isp->dev));
		kfree(isp->i2c.pdata);
	}

	return 0;
}

static int isp_mfp_init(struct isp_device *isp)
{

	return 0;
}

static int isp_clk_init(struct isp_device *isp)
{
	int i;

	for (i = 0; i < ISP_CLK_NUM; i++) {
		isp->clk[i] = clk_get(isp->dev, isp_clks[i].name);
		if (IS_ERR(isp->clk[i])) {
			printk(KERN_ERR "Failed to get %s clock %ld\n",
				isp_clks[i].name, PTR_ERR(isp->clk[i]));
			return PTR_ERR(isp->clk[i]);
		}

		clk_set_rate(isp->clk[i], isp_clks[i].rate);
		isp->clk_enable[i] = 0;
	}

	return 0;
}

static int isp_clk_release(struct isp_device *isp)
{
	int i;

	for (i = 0; i < ISP_CLK_NUM; i++)
		clk_put(isp->clk[i]);

	return 0;
}

static int isp_clk_enable(struct isp_device *isp, unsigned int type)
{
	int i;

	for (i = 0; i < ISP_CLK_NUM; i++) {
		if (!isp->clk_enable[i] && (isp_clks[i].flags & type)) {
			clk_enable(isp->clk[i]);
			isp->clk_enable[i] = 1;
		}
	}

	return 0;
}

static int isp_clk_disable(struct isp_device *isp, unsigned int type)
{
	int i;

	for (i = 0; i < ISP_CLK_NUM; i++) {
		if (isp->clk_enable[i] && (isp_clks[i].flags & type)) {
			clk_disable(isp->clk[i]);
			isp->clk_enable[i] = 0;
		}
	}

	return 0;
}


static int isp_powerdown(void)
{
	
	return 0;
}

static int isp_powerup(void)
{

	return 0;
}

static int isp_init(struct isp_device *isp, void *data)
{
	isp->boot = 0;
	isp->poweron = 1;
	isp->snapshot = 0;
	isp->bypass = 0;
	isp->running = 0;
	isp->format_active = 0;
	isp->bracket_end = 0;
	memset(&isp->parm, 0, sizeof(isp->parm));

	isp_mfp_init(isp);

	return 0;
}

static int isp_release(struct isp_device *isp, void *data)
{
	isp->boot = 0;
	isp->poweron = 0;
	isp->snapshot = 0;
	isp->bypass = 0;
	isp->running = 0;
	isp->format_active = 0;

	return 0;
}

static int isp_open(struct isp_device *isp, struct isp_prop *prop)
{
	struct ovisp_camera_client *client = &isp->pdata->client[prop->index];
	int ret = 0;

	if (!isp->poweron)
		return -ENODEV;

	isp_powerup();
	isp_clk_enable(isp, ISP_CLK_MAIN);

	if (!isp->boot) {
		ret = isp_boot(isp);
		if (ret)
			return ret;

		isp->boot = 1;
	}

	if (client->flags & CAMERA_CLIENT_IF_MIPI) {
		isp_clk_enable(isp, ISP_CLK_CSI);
		ret = isp_mipi_init(isp);
	} else if (client->flags & CAMERA_CLIENT_IF_DVP) {
		ret = isp_dvp_init(isp);
	}

	isp->debug.status = 1;
	isp->input = prop->index;
	isp->bypass = prop->bypass;
	isp->snapshot = 0;
	isp->format_active = 0;
	memset(&isp->fmt_data, 0, sizeof(isp->fmt_data));

	isp->pp_buf = true;
	isp->first_init = 1;

	memset(&isp->debug, 0, sizeof(isp->debug));
	return ret;
}

static int isp_close(struct isp_device *isp, struct isp_prop *prop)
{
	struct ovisp_camera_client *client = &isp->pdata->client[prop->index];

	/* wait for mac wirte ram finish */
	msleep(80);

	if (!isp->poweron)
		return -ENODEV;

	if (client->flags & CAMERA_CLIENT_IF_MIPI) {
		csi_phy_stop(prop->index);
		isp_clk_disable(isp, ISP_CLK_CSI);
	}

	isp_reg_writeb(isp, DO_SOFT_RST, REG_ISP_SOFT_RST);
	isp_reg_writeb(isp, isp_reg_readb(isp, 0x6301b) | 0x02, 0x6301b);
	isp_reg_writeb(isp, 0x00, REG_ISP_INT_EN);
	isp_reg_writeb(isp, 0x00, REG_ISP_MAC_INT_EN_H);
	isp_reg_writeb(isp, 0x00, REG_ISP_MAC_INT_EN_L);

	isp_powerdown();
	isp_clk_disable(isp, ISP_CLK_MAIN);
	isp->input = -1;

	return 0;
}

static int isp_config(struct isp_device *isp, void *data)
{
	return 0;
}

static int isp_suspend(struct isp_device *isp, void *data)
{
	return 0;
}

static int isp_resume(struct isp_device *isp, void *data)
{
	return 0;
}

static int isp_mclk_on(struct isp_device *isp, int index)
{
	ovisp_mfp_config(MFP_PIN_GPIO72, MFP_GPIO72_ISP_SCLK0);
	isp_clk_enable(isp, ISP_CLK_DEV);

	return 0;
}

static int isp_mclk_off(struct isp_device *isp, int index)
{
	isp_clk_disable(isp, ISP_CLK_DEV);
	ovisp_mfp_config(MFP_PIN_GPIO72, MFP_PIN_MODE_GPIO);

	return 0;
}

static int isp_update_buffer(struct isp_device *isp, struct isp_buffer *buf)
{
	if(isp->pp_buf){
		isp_set_address(isp, 0, buf->paddr);
		isp_reg_writeb(isp, 0x01, REG_BASE_ADDR_READY);
		isp->pp_buf = false;
	}else{
		isp_set_address(isp, 1, buf->paddr);
		isp_reg_writeb(isp, 0x02, REG_BASE_ADDR_READY);
		isp->pp_buf = true;
	}

	return 0;
}

static int isp_offline_process(struct isp_device *isp,  unsigned long  source_addr, unsigned long  target_addr)
{
	struct isp_parm *iparm = &isp->parm;
	unsigned char iformat;
	unsigned char oformat;
	unsigned long uv_addr;
	uv_addr = target_addr + iparm->out_width*iparm->out_height;

	iformat = IFORMAT_RAW10;

	if (iparm->out_format == V4L2_PIX_FMT_NV21)
		oformat = OFFLINE_OFORMAT_YUV420;
	else
		oformat = OFFLINE_OFORMAT_YUV422;

	isp_reg_writeb(isp, iformat, (ISP_INPUT_FORMAT));
	isp_reg_writeb(isp, oformat, (ISP_OUTPUT_FORMAT));

	isp_reg_writeb(isp, (iparm->out_width >> 8) & 0xff, (ISP_OFFLINE_INPUT_WIDTH));
	isp_reg_writeb(isp, iparm->out_width & 0xff, (ISP_OFFLINE_INPUT_WIDTH + 1));
	isp_reg_writeb(isp, (iparm->out_height >> 8) & 0xff, (ISP_OFFLINE_INPUT_HEIGHT));
	isp_reg_writeb(isp, iparm->out_height & 0xff, (ISP_OFFLINE_INPUT_HEIGHT + 1));

	isp_reg_writeb(isp, (iparm->out_width >> 8) & 0xff, (ISP_OFFLINE_MAC_READ_MEM_WIDTH));
	isp_reg_writeb(isp, iparm->out_width & 0xff, (ISP_OFFLINE_MAC_READ_MEM_WIDTH + 1));

	isp_reg_writeb(isp, (iparm->out_width >> 8) & 0xff, (ISP_OFFLINE_OUTPUT_WIDTH));
	isp_reg_writeb(isp, iparm->out_width & 0xff, (ISP_OFFLINE_OUTPUT_WIDTH + 1));
	isp_reg_writeb(isp, (iparm->out_height >> 8) & 0xff, (ISP_OFFLINE_OUTPUT_HEIGHT));
	isp_reg_writeb(isp, iparm->out_height & 0xff, (ISP_OFFLINE_OUTPUT_HEIGHT + 1));

	isp_reg_writeb(isp, (iparm->out_width >> 8) & 0xff, (ISP_OFFLINE_MAC_WRITE_MEM_WIDTH));
	isp_reg_writeb(isp, iparm->out_width & 0xff, (ISP_OFFLINE_MAC_WRITE_MEM_WIDTH + 1));

	isp_reg_writeb(isp, ((u32)source_addr >>24) & 0xff, (ISP_OFFLINE_INPUT_BASE_ADDR));
	isp_reg_writeb(isp, ((u32)source_addr>>16) & 0xff, (ISP_OFFLINE_INPUT_BASE_ADDR + 1));
	isp_reg_writeb(isp, ((u32)source_addr>>8) & 0xff, (ISP_OFFLINE_INPUT_BASE_ADDR + 2));
	isp_reg_writeb(isp, (u32)source_addr & 0xff, (ISP_OFFLINE_INPUT_BASE_ADDR + 3));

	isp_reg_writeb(isp, ((u32)target_addr >>24) & 0xff, (ISP_OFFLINE_OUTPUT_BASE_ADDR));
	isp_reg_writeb(isp, ((u32)target_addr>>16) & 0xff, (ISP_OFFLINE_OUTPUT_BASE_ADDR + 1));
	isp_reg_writeb(isp, ((u32)target_addr>>8) & 0xff, (ISP_OFFLINE_OUTPUT_BASE_ADDR + 2));
	isp_reg_writeb(isp, (u32)target_addr & 0xff, (ISP_OFFLINE_OUTPUT_BASE_ADDR + 3));

	isp_reg_writeb(isp, ((u32)uv_addr >>24)  & 0xff, (ISP_OFFLINE_OUTPUT_BASE_ADDR_UV + 3));
	isp_reg_writeb(isp, ((u32)uv_addr >>16) & 0xff, (ISP_OFFLINE_OUTPUT_BASE_ADDR_UV + 3));
	isp_reg_writeb(isp, ((u32)uv_addr >>8) & 0xff, (ISP_OFFLINE_OUTPUT_BASE_ADDR_UV + 3));
	isp_reg_writeb(isp, (u32)uv_addr & 0xff, (ISP_OFFLINE_OUTPUT_BASE_ADDR_UV + 3));

	

	isp_reg_writeb(isp, 0x00, (ISP_OFFLINE_FUNCTION_CONTROL));

	isp_reg_writeb(isp, 0x00, (ISP_OFFLINE_SCALE_DOWN_H_TATIO_1));
	isp_reg_writeb(isp, 0x80, (ISP_OFFLINE_SCALE_DOWN_H_TATIO_2));
	isp_reg_writeb(isp, 0x00, (ISP_OFFLINE_SCALE_DOWN_V_TATIO_1));
	isp_reg_writeb(isp, 0x80, (ISP_OFFLINE_SCALE_DOWN_V_TATIO_2));

	isp_reg_writeb(isp, 0x01, (ISP_OFFLINE_SCALE_UP_H_TATIO));
	isp_reg_writeb(isp, 0x00, (ISP_OFFLINE_SCALE_UP_H_TATIO + 1));
	isp_reg_writeb(isp, 0x01, (ISP_OFFLINE_SCALE_UP_V_TATIO));
	isp_reg_writeb(isp, 0x00, (ISP_OFFLINE_SCALE_UP_V_TATIO + 1));

	isp_reg_writeb(isp, 0x00, (ISP_OFFLINE_CROP_H_START));
	isp_reg_writeb(isp, 0x00, (ISP_OFFLINE_CROP_H_START + 1));
	isp_reg_writeb(isp, 0x00, (ISP_OFFLINE_CROP_V_START));
	isp_reg_writeb(isp, 0x00, (ISP_OFFLINE_CROP_V_START + 1));


	isp_reg_writeb(isp, (iparm->out_width >> 8) & 0xff, (ISP_OFFLINE_CROP_WIDTH));
	isp_reg_writeb(isp, iparm->out_width  & 0xff, (ISP_OFFLINE_CROP_WIDTH + 1));
	isp_reg_writeb(isp, ((iparm->out_height >> 8) & 0xff), (ISP_OFFLINE_CROP_HEIGHT));
	isp_reg_writeb(isp, (iparm->out_height & 0xff), (ISP_OFFLINE_CROP_HEIGHT + 1));
#if 0
	isp_reg_writeb(isp, ((iparm->out_width >> 8) & 0xff), (ISP_OFFLINE_MAC_WRITE_UV_WIDTH));
	isp_reg_writeb(isp, (iparm->out_width & 0xff), (ISP_OFFLINE_MAC_WRITE_UV_WIDTH + 1));
#endif

	
	/* Wait for command set done interrupt. */
	printk("*********before offline \n");

	isp_reg_writeb(isp, 0x40, COMMAND_REG5);
	//isp_reg_writeb(isp, 0x03, REG_BASE_ADDR_READY);
	if (isp_send_cmd(isp, CMD_OFFLINE_PROCESS, ISP_OFFLINE_TIMEOUT)) {
		printk(KERN_ERR "Failed to wait offline set done!\n");
		return -ETIME;
	}
	if ((CMD_SET_SUCCESS != isp_reg_readb(isp, COMMAND_RESULT))
	        || (CMD_OFFLINE_PROCESS != isp_reg_readb(isp, COMMAND_FINISHED))) {
		printk(KERN_ERR "Failed to set offline process (%02x:%02x)\n",
		       isp_reg_readb(isp, COMMAND_RESULT),
		       isp_reg_readb(isp, COMMAND_FINISHED));
		return -EINVAL;
	}
	printk("*********after offline \n");
	return 0;
}



static int isp_start_capture(struct isp_device *isp, struct isp_capture *cap)
{	

	int ret = 0;
	struct ovisp_camera_dev *camera_dev;
	camera_dev = (struct ovisp_camera_dev *)(isp->data);
	isp->snapshot = cap->snapshot;
	isp->client = cap->client;
	
	isp->buf_start = cap->buf;

	
	if (isp->format_active){
		isp_set_parameters(isp);

		if (!isp->snapshot)
			ret = isp_set_format(isp);
		else
			ret = isp_set_capture(isp);
		if (ret)
			return ret;
	}
	if(isp->bypass&&isp->snapshot&&!isp->format_active) {
		isp_set_parameters(isp);
		ret = isp_set_capture(isp);

	}
		isp_int_mask(isp, 0xff);
		isp_mac_int_mask(isp, 0xffff);
		isp_int_state(isp);
		isp_mac_int_state(isp);
		isp_int_unmask(isp, MASK_INT_MAC);
		isp_mac_int_unmask(isp, MASK_INT_FRAME_START |
					MASK_INT_WRITE_DONE0 | MASK_INT_WRITE_DONE1 |
					MASK_INT_OVERFLOW0 | MASK_INT_OVERFLOW1 |
					MASK_INT_DROP0 | MASK_INT_DROP1);
	isp->running = 1;
	/* call the update buffer in drop int */
	
	isp->hdr_mode = 0;
	isp->bracket_end = 0;
	isp->debug.status = isp->snapshot;
	return 0;
}

static int isp_stop_capture(struct isp_device *isp, void *data)
{
	isp->running = 0;
	isp_int_mask(isp, 0xff);
	isp_mac_int_mask(isp, 0xffff);
	isp_reg_writeb(isp, 0x00, REG_BASE_ADDR_READY);
	isp_int_state(isp);
	isp_mac_int_state(isp);

	return 0;
}

static int isp_enable_capture(struct isp_device *isp, struct isp_buffer *buf)
{
	printk("[ISP] %s\n", __func__);
	isp_int_state(isp);
	isp_mac_int_state(isp);
	isp->buf_start = *buf;
	isp_mac_int_unmask(isp, (MASK_INT_DROP0 | MASK_INT_DROP1));

	return 0;
}

static int isp_disable_capture(struct isp_device *isp, void *data)
{
	isp_reg_writeb(isp, 0x00, REG_BASE_ADDR_READY);
	printk("[ISP] %s\n", __func__);

	return 0;
}

static int isp_check_fmt(struct isp_device *isp, struct isp_format *f)
{
	if (isp->bypass)
		return 0;

	switch (f->fourcc) {
		case V4L2_PIX_FMT_YUYV:
			break;
		case V4L2_PIX_FMT_NV12:
		/* Now, we don't support yuv420sp. */
		default:
			return -EINVAL;
	}

	return 0;
}

static int isp_try_fmt(struct isp_device *isp, struct isp_format *f)
{
	int ret;

	ret = isp_check_fmt(isp, f);
	if (ret)
		return ret;

	return 0;
}

static int isp_pre_fmt(struct isp_device *isp, struct isp_format *f)
{
	int ret = 0;

	if (!isp->bypass && (isp->fmt_data.mipi_clk != f->fmt_data->mipi_clk)) {
		printk("~~~~~~~~~~~restart phy!!!\n");
		printk("f->fmt_data->mipi_clk = %d\n",f->fmt_data->mipi_clk);
		ret = csi_phy_start(0, f->fmt_data->mipi_clk);
		if (!ret)
			isp->fmt_data.mipi_clk = f->fmt_data->mipi_clk;
	}

	return ret;
}

static int isp_s_fmt(struct isp_device *isp, struct isp_format *f)
{
	struct isp_parm *iparm = &isp->parm;
	int in_width, in_height;

	if (isp->bypass) {
		in_width = f->width;
		in_height = f->height;
	} else {
		in_width = f->dev_width;
		in_height = f->dev_height;
	}

	/* check if need set format to isp. */
	isp->format_active = 0;
	if ((isp->fmt_data.vts != f->fmt_data->vts)
		|| (isp->fmt_data.hts != f->fmt_data->hts))
		isp->format_active = 1;

	if ((iparm->in_width != in_width)
		|| (iparm->in_height != in_height)
		|| (iparm->out_width != f->width)
		|| (iparm->out_height != f->height)
		|| (iparm->out_format != f->fourcc)
		|| (iparm->in_format != f->code))
		isp->format_active = 1;

	/* Save the parameters. */
	iparm->in_width = in_width;
	iparm->in_height = in_height;
	iparm->in_format = f->code;
	iparm->out_width = f->width;
	iparm->out_height = f->height;
	iparm->out_format = f->fourcc;
	iparm->crop_width = iparm->in_width;
	iparm->crop_height = iparm->in_height;
	iparm->crop_x = 0;
	iparm->crop_y = 0;
//	printk("[%s] f->fmt_data.mipi_clk = %d\n",__func__,f->fmt_data.mipi_clk);
	memcpy(&isp->fmt_data, f->fmt_data, sizeof(isp->fmt_data));
	printk("[%s]isp->fmt_data.vts = %x;isp->fmt_data.mipi_clk = %d\n",__func__,isp->fmt_data.vts,isp->fmt_data.mipi_clk);
	return 0;
}

static int isp_s_ctrl(struct isp_device *isp, struct v4l2_control *ctrl)
{
	int ret = 0;
	struct isp_parm *iparm = &isp->parm;

	switch (ctrl->id) {
	case V4L2_CID_DO_WHITE_BALANCE:
		ISP_PRINT("set white_balance %d\n", ctrl->value);
		ret = isp_set_white_balance(isp, ctrl->value);
		if (!ret)
			iparm->white_balance = ctrl->value;
		break;
	case V4L2_CID_BRIGHTNESS:
		ISP_PRINT("set brightness %d\n", ctrl->value);
		ret = isp_set_brightness(isp, ctrl->value);
		if (!ret)
			iparm->brightness = ctrl->value;
		break;
	case V4L2_CID_EXPOSURE:
		ISP_PRINT("set exposure %d\n", ctrl->value);
		ret = isp_set_exposure(isp, ctrl->value);
		if (!ret)
			iparm->exposure = ctrl->value;
		break;
	case V4L2_CID_CONTRAST:
		ISP_PRINT("set contrast %d\n", ctrl->value);
		ret = isp_set_contrast(isp, ctrl->value);
		if (!ret)
			iparm->contrast = ctrl->value;
		break;
	case V4L2_CID_SATURATION:
		ISP_PRINT("set saturation %d\n", ctrl->value);
		ret = isp_set_saturation(isp, ctrl->value);
		if (!ret)
			iparm->saturation = ctrl->value;
		break;
	case V4L2_CID_SHARPNESS:
		ISP_PRINT("set sharpness %d\n", ctrl->value);
		ret = isp_set_sharpness(isp, ctrl->value);
		if (!ret)
			iparm->sharpness = ctrl->value;
		break;
	case V4L2_CID_POWER_LINE_FREQUENCY:
		ISP_PRINT("set flicker %d\n", ctrl->value);
		ret = isp_set_flicker(isp, ctrl->value);
		if (!ret)
			iparm->flicker = ctrl->value;
		break;
	case V4L2_CID_FOCUS_AUTO:
		break;
	case V4L2_CID_FOCUS_RELATIVE:
		break;
	case V4L2_CID_FOCUS_ABSOLUTE:
		break;
	case V4L2_CID_ZOOM_RELATIVE:
		ISP_PRINT("set zoom %d\n", ctrl->value);
		if (isp->running)
			ret = isp_set_zoom(isp, ctrl->value);
		if (!ret)
			iparm->zoom = ctrl->value;
		break;
	case V4L2_CID_HFLIP:
		ISP_PRINT("set hflip %d\n", ctrl->value);
		ret = isp_set_hflip(isp, ctrl->value);
		if (!ret)
			iparm->hflip = ctrl->value;
		break;
	case V4L2_CID_VFLIP:
		ISP_PRINT("set vflip %d\n", ctrl->value);
		ret = isp_set_vflip(isp, ctrl->value);
		if (!ret)
			iparm->vflip = ctrl->value;
		break;
	/* Private. */
	case V4L2_CID_ISO:
		ISP_PRINT("set iso %d\n", ctrl->value);
		ret = isp_set_iso(isp, ctrl->value);
		if (!ret)
			iparm->iso = ctrl->value;
		break;
	case V4L2_CID_EFFECT:
		ISP_PRINT("set effect %d\n", ctrl->value);
		ret = isp_set_effect(isp, ctrl->value);
		if (!ret)
			iparm->effects = ctrl->value;
		break;
	case V4L2_CID_FLASH_MODE:
		break;
	case V4L2_CID_SCENE:
		ISP_PRINT("set scene %d\n", ctrl->value);
		ret = isp_set_scene(isp, ctrl->value);
		if (!ret)
			iparm->scene_mode = ctrl->value;
		break;
	case V4L2_CID_FRAME_RATE:
		iparm->frame_rate = ctrl->value;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int isp_g_ctrl(struct isp_device *isp, struct v4l2_control *ctrl)
{
	int ret = 0;
	struct isp_parm *iparm = &isp->parm;

	switch (ctrl->id) {
	case V4L2_CID_DO_WHITE_BALANCE:
		ctrl->value = iparm->white_balance;
		break;
	case V4L2_CID_BRIGHTNESS:
		ctrl->value = iparm->brightness;
		break;
	case V4L2_CID_EXPOSURE:
		ctrl->value = iparm->exposure;
		break;
	case V4L2_CID_CONTRAST:
		ctrl->value = iparm->contrast;
		break;
	case V4L2_CID_SATURATION:
		ctrl->value = iparm->saturation;
		break;
	case V4L2_CID_SHARPNESS:
		ctrl->value = iparm->sharpness;
		break;
	case V4L2_CID_POWER_LINE_FREQUENCY:
		ctrl->value = iparm->flicker;
		break;
	case V4L2_CID_FOCUS_AUTO:
		break;
	case V4L2_CID_FOCUS_RELATIVE:
		break;
	case V4L2_CID_FOCUS_ABSOLUTE:
		break;
	case V4L2_CID_ZOOM_RELATIVE:
		ctrl->value = iparm->zoom;
		break;
	case V4L2_CID_HFLIP:
		ctrl->value = iparm->hflip;
		break;
	case V4L2_CID_VFLIP:
		ctrl->value = iparm->vflip;
		break;
	/* Private. */
	case V4L2_CID_ISO:
		ctrl->value = iparm->iso;
		break;
	case V4L2_CID_EFFECT:
		ctrl->value = iparm->effects;
		break;
	case V4L2_CID_FLASH_MODE:
		break;
	case V4L2_CID_SCENE:
		ctrl->value = iparm->scene_mode;
		break;
	case V4L2_CID_FRAME_RATE:
		ctrl->value = iparm->frame_rate;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int isp_s_parm(struct isp_device *isp, struct v4l2_streamparm *parm)
{
	return 0;
}

static int isp_g_parm(struct isp_device *isp, struct v4l2_streamparm *parm)
{
	return 0;
}

static struct isp_ops isp_ops = {
	.init = isp_init,
	.release = isp_release,
	.open = isp_open,
	.close = isp_close,
	.config = isp_config,
	.suspend = isp_suspend,
	.resume = isp_resume,
	.mclk_on = isp_mclk_on,
	.mclk_off = isp_mclk_off,
	.offline_process = isp_offline_process,
	.start_capture = isp_start_capture,
	.stop_capture = isp_stop_capture,
	.enable_capture = isp_enable_capture,
	.disable_capture = isp_disable_capture,
	.update_buffer = isp_update_buffer,
	.check_fmt = isp_check_fmt,
	.try_fmt = isp_try_fmt,
	.pre_fmt = isp_pre_fmt,
	.s_fmt = isp_s_fmt,
	.s_ctrl = isp_s_ctrl,
	.g_ctrl = isp_g_ctrl,
	.s_parm = isp_s_parm,
	.g_parm = isp_g_parm,
};

int isp_device_init(struct isp_device* isp)
{
	struct resource *res = isp->res;
	int ret = 0;

	spin_lock_init(&isp->lock);
	init_completion(&isp->completion);

	isp->base = ioremap(res->start, res->end - res->start + 1);
	if (!isp->base) {
		printk(KERN_ERR "Unable to ioremap registers.n");
		ret = -ENXIO;
		goto exit;
	}

	ret = isp_int_init(isp);
	if (ret)
		goto io_unmap;

	ret = isp_i2c_init(isp);
	if (ret)
		goto irq_free;

	ret = isp_clk_init(isp);
	if (ret)
		goto i2c_release;

	csi_phy_init();

	isp->ops = &isp_ops;

	return 0;

i2c_release:
	isp_i2c_release(isp);
irq_free:
	free_irq(isp->irq, isp);
io_unmap:
	iounmap(isp->base);
exit:
	return ret;
}
EXPORT_SYMBOL(isp_device_init);

int isp_device_release(struct isp_device* isp)
{
	if (!isp)
		return -EINVAL;

	csi_phy_release();
	isp_clk_release(isp);
	isp_i2c_release(isp);
	free_irq(isp->irq, isp);
	iounmap(isp->base);

	return 0;
}
EXPORT_SYMBOL(isp_device_release);

