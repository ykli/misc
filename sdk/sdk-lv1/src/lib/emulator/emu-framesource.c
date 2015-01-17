/*
 * Ingenic IMP emulation.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <imp_log.h>
#include <imp_sys.h>
#include <imp_utils.h>
#include <system/constraints.h>
#include <system/module.h>
#include <system/group.h>
#include <system/device.h>

#define TAG "emu-Framesource"
#define TMP_NR_VBS 3
#define TMP_FRAME_SIZE 4096

typedef struct {
	Device *device;

	pthread_t tid;

	FrameInfo *v4l2_vbs;
	int nr_vbs;
} Framesource;

static Framesource *gFramesource = NULL;

Framesource *get_framesource(void)
{
	return gFramesource;
}

void* frame_pooling_thread(void *p)
{
	Framesource *fs = (Framesource *)p;

	while (1) { /* Should be modified!! */
		void *tmp;

		IMP_LOG_DBG(TAG, "thread run\n");

		//select();
		usleep(500000);
		/* Ugly implement start... */
		Group *group = fs->device->groups[0];
		Module *module = group->module;

		module->Update(module, &tmp);
	}
}

static int framesource_init(Framesource *framesource)
{
	/* Framesource initial work here. */
	int ret, i;

	framesource->nr_vbs = TMP_NR_VBS;
	framesource->v4l2_vbs = malloc(framesource->nr_vbs * sizeof(FrameInfo));

	for (i = 0; i < framesource->nr_vbs; i++) {
		FrameInfo *vb_info = &framesource->v4l2_vbs[i];
		vb_info->virAddr = (uint32_t)malloc(TMP_FRAME_SIZE);
		IMP_LOG_DBG(TAG, "vb%d: 0x%08x\n", i, vb_info->virAddr);
	}

	ret = pthread_create(&framesource->tid, NULL,
						 frame_pooling_thread, framesource);
	if (ret) {
		IMP_LOG_ERR(TAG, "thread create error\n");
		return -1;
	}

	return 0;
}

static void framesource_exit(Framesource *framesource)
{
	/* Exit work here. */
}

static Framesource *alloc_framesource(void)
{
	Framesource *framesource;
	Device *dev = alloc_device("Framesource", sizeof(Framesource));

	if (dev == NULL) {
		IMP_LOG_ERR(TAG, "alloc_device() error\n");
		return NULL;
	}
	dev->nr_groups = NR_MAX_FS_GROUPS;

	framesource = device_pri(dev);
	framesource->device = dev;

	return framesource;
}

static void free_framesource(Framesource *framesource)
{
	Device *dev = framesource->device;
	free_device(dev);
}

static int on_framesource_group_data_update(Group *group, FrameInfo *frame)
{
	Device *dev = get_device_of_group(group);
	Framesource *framesource = (Framesource *)device_pri(dev);
#if 0
	static int frame_i = 0;

	FrameInfo *cur_frame = &framesource->v4l2_vbs[frame_i++ % framesource->nr_vbs];
	cur_frame->timeStamp = frame_i * 100;

	FrameInfo *cur_frame2 = &framesource->v4l2_vbs[frame_i++ % framesource->nr_vbs];
	cur_frame2->timeStamp = frame_i * 100;
	
	group->for_channel_data[0] = cur_frame;
	group->for_channel_data[1] = cur_frame2;

	IMP_LOG_DBG(TAG, "[%s][%s] update\n",
				framesource->device->name, group->module->name);
#else
	FrameInfo *cur_frame[max_chh_macro];
	VbmGetFrames(nr_channel_enabled, cur_frame);
	group->for_channel_data[0] = cur_frame[0];
	group->for_channel_data[1] = cur_frame[1];
#endif
	/* V4L2 DQBUF here. Then get one frame. */


	return 0;
}

static int framesource_create_group(int group_index)
{
	Framesource *framesource = get_framesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "Invalid group num%d\n", group_index);
		return -1;
	}

	Device *dev = framesource->device;

	if (group_index > NR_MAX_FS_GROUPS - 1) {
		IMP_LOG_ERR(TAG, "Invalid group num%d\n", group_index);
		return -1;
	}

	char grp_name[MAX_MODULE_NAME_LEN];
	sprintf(grp_name, "%s-%d", dev->name, group_index);

	Group *grp = create_group(DEV_ID_EMU_FS, group_index, grp_name,
							  on_framesource_group_data_update);
	grp->device = dev;
	grp->nr_channels = NR_MAX_FS_CHN_IN_GROUP;
	dev->groups[group_index] = grp;

	return 0;
}

static int framesource_destroy_group(int group_index)
{
	Framesource *framesource = get_framesource();
	Device *dev = framesource->device;

	if (group_index > NR_MAX_FS_GROUPS - 1) {
		IMP_LOG_ERR(TAG, "Invalid group num%d\n\n", group_index);
		return -1;
	}

	Group *grp = dev->groups[group_index];

	if (grp == NULL) {
		IMP_LOG_WARN(TAG, "group-%d has not been created\n", group_index);
		return -1;
	}

	destroy_group(grp, DEV_ID_EMU_FS); /* TODO return value. */
	dev->groups[group_index] = NULL;

	return 0;
}

int EmuFrameSourceInit(void)
{
	int ret;

	gFramesource = alloc_framesource();
	if (gFramesource == NULL)
		return -1;

	ret = framesource_init(gFramesource);
	if (ret < 0)
		goto free;

	framesource_create_group(0);

	return ret;
free:
	free_framesource(gFramesource);
	gFramesource = NULL;

	return -1;
}

int EmuFrameSourceExit(void)
{
	if (gFramesource) {
		framesource_destroy_group(0);
		framesource_exit(gFramesource);
		free_framesource(gFramesource);
		gFramesource = NULL;
	}

	return 0;
}
