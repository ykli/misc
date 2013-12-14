/* arch/arm/mach-xxx/board-xxx.c
**
** This software is licensed under the terms of the GNU General Public
** License version 2, as published by the Free Software Foundation, and
** may be copied, distributed, and modified under those terms.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** Copyright (c) 2010-2019 Omnivision Corp.
**
**
** CHANGE HISTORY:
**
** Version	Date		Author		Description
** 1.0.0	2012-03-06	Wayne	        created
**
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/input.h>
#include <linux/input/ft5x06_ts.h>
#include <linux/fb.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include <linux/spi/inno_if2xx.h>
#include <linux/spi/sms.h>
#include <linux/nfc/pn65n.h>
#include <linux/mfd/lp8788.h>
#include <linux/mfd/lc1100h.h>
#include <linux/wlan_plat.h>
#include <linux/i2c/taos_common.h>
#include <linux/mpu.h>
#include <linux/bmc.h>

#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/flash.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>

#include <media/ov5647.h>
#include <media/ov8825.h>
#include <media/front_camera.h>

#include <mach/timer.h>
#include <mach/mfp.h>
#include <mach/gpio.h>
#include <mach/i2c.h>
#include <mach/spi.h>
#include <mach/switch.h>
#include <mach/mmc.h>
#include <mach/usb.h>
#include <mach/uart.h>
#include <mach/keypad.h>
#include <mach/camera.h>
#include <mach/suspend.h>
#include <mach/hardware.h>


#include "generic.h"
#include "board-xxx.h"


#if defined(CONFIG_VIDEO_OVISP)
#if defined(CONFIG_VIDEO_OV5647)
static int ov5647_pwdn = mfp_to_gpio(OV5647_POWERDOWN_PIN);
static int ov5647_rst = mfp_to_gpio(OV5647_RESET_PIN);
static int ov5647_power(int onoff)
{
	printk("############## ov5647 power : %d################\n", onoff);

	if (onoff) {		
		;
	} else {
		;
	}

	return 0;
}

static int ov5647_reset(void)
{
	printk("############## ov5647 reset################\n");

	return 0;
}

static struct i2c_board_info ov5647_board_info = {
	.type = "ov5647",
	.addr = 0x36,
};
#endif


#if defined(CONFIG_VIDEO_front_camera)
static int front_camera_pwdn = mfp_to_gpio(front_camera_POWERDOWN_PIN);
static int front_camera_power(int onoff)
{
	printk("############## front_camera power : %d################\n", onoff);

	if (onoff) {
		;
	} else {
		;
	}

	return 0;
}

static int front_camera_reset(void)
{
	printk("############## front_camera reset################\n");
	return 0;
}

static struct i2c_board_info front_camera_board_info = {
	.type = "front_camera",
	.addr = 0x31,
};
#endif

static struct ovisp_camera_client ovisp_camera_clients[] = {
#if defined(CONFIG_VIDEO_OV5647)
	{
		.board_info = &ov5647_board_info,
		.flags = CAMERA_CLIENT_IF_MIPI,
		.mclk_rate = 26000000,
		.max_video_width = 1280,
		.max_video_height = 960,
		.power = ov5647_power,
		.reset = ov5647_reset,
	},
#endif

#if defined(CONFIG_VIDEO_front_camera)
	{
		.board_info = &front_camera_board_info,
		.flags = CAMERA_CLIENT_IF_DVP
				| CAMERA_CLIENT_CLK_EXT
				| CAMERA_CLIENT_ISP_BYPASS,
		.mclk_parent_name = "pll1_mclk",
		.mclk_name = "clkout1_clk",
		.mclk_rate = 24000000,
		.power = front_camera_power,
		.reset = front_camera_reset,
	},
#endif
};

static struct ovisp_camera_platform_data ovisp_camera_info = {
	.i2c_adapter_id = 3,
	.flags = CAMERA_USE_ISP_I2C | CAMERA_USE_HIGH_BYTE
			| CAMERA_I2C_PIO_MODE | CAMERA_I2C_STANDARD_SPEED,
	.client = ovisp_camera_clients,
	.client_num = ARRAY_SIZE(ovisp_camera_clients),
};

static void __init ovisp_init_camera(void)
{
	ovisp_set_camera_info(&ovisp_camera_info);
}
#else
static void inline ovisp_init_camera(void)
{
};
#endif



static void __init board_xxx_init(void)
{
	
	ovisp_init_camera();
	
}



