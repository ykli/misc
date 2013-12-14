#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>

#include <mach/camera.h>
#include <mach/keypad.h>
#include <mach/dmas.h>
#include <mach/uart.h>
#include <mach/gpio.h>

#include "devices.h"

void __init xxx_register_device(struct platform_device *dev, void *data)
{
	int ret;

	dev->dev.platform_data = data;

	ret = platform_device_register(dev);
	if (ret)
		dev_err(&dev->dev, "unable to register device: %d\n", ret);
}

static u64 ovisp_camera_dma_mask = ~(u64)0;
static struct resource ovisp_resource_camera[] = {
	[0] = {
		.start = ISP_BASE,
		.end = ISP_BASE + 0x80000,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_ISP,
		.end = INT_ISP,
		.flags = IORESOURCE_IRQ,
	},
};



struct platform_device ovisp_device_camera = {
	.name = "ovisp-camera",
	.id = -1,
	.dev = {
		.dma_mask = &ovisp_camera_dma_mask,
		.coherent_dma_mask = 0xffffffff,
	},
	.num_resources = ARRAY_SIZE(ovisp_resource_camera),
	.resource = ovisp_resource_camera,
};

void __init ovisp_set_camera_info(struct ovisp_camera_platform_data *info)
{
	xxx_register_device(&ovisp_device_camera, info);
}

