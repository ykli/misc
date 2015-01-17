/*
 * Ingenic IMP emulation.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <stdio.h>

#include <imp_log.h>
#include <imp_sys.h>
#include <imp_utils.h>
#include <system/constraints.h>
#include <system/module.h>
#include <system/group.h>
#include <system/device.h>

#define TAG "emu-Encoder"

typedef struct {
	Device *device;
	/* Other private data */
} Encoder;

static Encoder *gEncoder = NULL;

Encoder *get_encoder(void)
{
	return gEncoder;
}

static int encoder_init(Encoder *encoder)
{
	/* Encoder initial work here. */

	return 0;
}

static void encoder_exit(Encoder *encoder)
{
	/* Exit work here. */
}

static Encoder *alloc_encoder(void)
{
	Encoder *encoder;
	Device *dev = alloc_device("Encoder", sizeof(Encoder));

	if (dev == NULL) {
		IMP_LOG_ERR(TAG, "alloc_device() error\n");
		return NULL;
	}
	dev->nr_groups = NR_MAX_ENC_GROUPS;

	encoder = device_pri(dev);
	encoder->device = dev;

	return encoder;
}

static void free_encoder(Encoder *encoder)
{
	Device *dev = encoder->device;
	free_device(dev);
}

static int on_encoder_group_data_update(Group *group, FrameInfo *frame)
{
	Device *dev = get_device_of_group(group);
	Encoder *encoder = (Encoder *)device_pri(dev);

	IMP_LOG_DBG(TAG, "[%s][%s] update addr=0x%08x, timestamp=%d\n",
				encoder->device->name, group->module->name,
				frame->virAddr, frame->timeStamp);
	
	return 0;
}

int EmuEncoderInit(void)
{
	int ret;

	gEncoder = alloc_encoder();
	if (gEncoder == NULL)
		return -1;

	ret = encoder_init(gEncoder);
	if (ret < 0)
		goto free;

	return ret;
free:
	free_encoder(gEncoder);
	gEncoder = NULL;

	return -1;
}

int EmuEncoderExit(void)
{
	if (gEncoder) {
		encoder_exit(gEncoder);
		free_encoder(gEncoder);
		gEncoder = NULL;
	}

	return 0;
}

int IMP_EmuEncoder_CreateGroup(int group_index)
{
	Encoder *encoder = get_encoder();
	if (encoder == NULL) {
		IMP_LOG_ERR(TAG, "Invalid group num%d\n", group_index);
		return -1;
	}

	Device *dev = encoder->device;

	if (group_index > NR_MAX_ENC_GROUPS - 1) {
		IMP_LOG_ERR(TAG, "Invalid group num%d\n", group_index);
		return -1;
	}

	char grp_name[MAX_MODULE_NAME_LEN];
	sprintf(grp_name, "%s-%d", dev->name, group_index);

	Group *grp = create_group(DEV_ID_EMU_ENC, group_index, grp_name,
							  on_encoder_group_data_update);
	grp->device = dev;
	grp->nr_channels = NR_MAX_ENC_CHN_IN_GROUP;
	dev->groups[group_index] = grp;

	return 0;
}

int IMP_EmuEncoder_DestroyGroup(int group_index)
{
	Encoder *encoder = get_encoder();
	Device *dev = encoder->device;

	if (group_index > NR_MAX_ENC_GROUPS - 1) {
		IMP_LOG_ERR(TAG, "Invalid group num%d\n\n", group_index);
		return -1;
	}

	Group *grp = dev->groups[group_index];

	if (grp == NULL) {
		IMP_LOG_WARN(TAG, "group-%d has not been created\n", group_index);
		return -1;
	}

	destroy_group(grp, DEV_ID_EMU_ENC); /* TODO return value. */
	dev->groups[group_index] = NULL;

	return 0;
}
