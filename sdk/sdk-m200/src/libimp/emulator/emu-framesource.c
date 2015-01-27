/*
 * Ingenic IMP emulation FrameSource.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <imp/imp_constraints.h>
#include <imp/imp_log.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_utils.h>

#include <system/system.h>
#include <system/module.h>
#include <system/group.h>
#include <system/device.h>
#include <system/vbm.h>
#include <emulator/imp_emu_framesource.h>

#define TAG "emu-Framesource"

typedef struct {
	int is_enabled;
	IMPFSChnAttr attr;
} FSChannel;

typedef struct {
	Device *device;

	pthread_t tid;

	IMPFSDevAttr attr;
	FSChannel channel[NR_MAX_FS_CHN_IN_GROUP];
} Framesource;

typedef struct {
	int index;
	uint32_t vaddr;
	int length;
	struct timeval timestamp;
} EmuV4L2Buffer;

static Framesource *gFramesource = NULL;
static EmuV4L2Buffer g_emu_vb[NR_MAX_CLUSTERS];
static int nr_emu_vb = 0;

Framesource *get_framesource(void)
{
	return gFramesource;
}

static int v4l2_emu_qbuf(EmuV4L2Buffer *vb)
{
	int index = vb->index;

	g_emu_vb[index].index = vb->index;
	g_emu_vb[index].vaddr = vb->vaddr;
	g_emu_vb[index].length = vb->length;

	return 0;
}

static int v4l2_emu_dqbuf(EmuV4L2Buffer *vb)
{
	static int who = 0;

	if (!nr_emu_vb) {
		IMP_LOG_ERR(TAG, "Emu buffer hasn't inited yet\n");
		return -1;
	}

	int index = who++ % nr_emu_vb;

	vb->index = g_emu_vb[index].index;
	vb->vaddr = g_emu_vb[index].vaddr;
	vb->length = g_emu_vb[index].length;
	gettimeofday(&vb->timestamp, NULL);

	/* Draw sth. Haha. */
	int i;
	for (i = 0; i < vb->length; i++) {
		uint8_t *addr = (uint8_t *)vb->vaddr;
		if (((i % 128) < 64) && ((index % 2) == 0))
			*addr = 0xff;
		else
			*addr = 0;
	}

	return 0;
}

static int get_cluster(int *cluster_idx, uint64_t *time, void *pri)
{
	EmuV4L2Buffer buf;
	memset(&buf, 0, sizeof buf);

	v4l2_emu_dqbuf(&buf);

	*cluster_idx = buf.index;
	*time = (uint64_t)((int64_t)buf.timestamp.tv_sec * 1000000 + (int64_t)buf.timestamp.tv_usec);

	IMP_LOG_DBG(TAG, "get cluster:%d \n", buf.index);

	return 0;
}

static int release_cluster(VBMCluster *cluster, void *pri)
{
	EmuV4L2Buffer buf;
	memset(&buf, 0, sizeof buf);

	buf.index = cluster->index;
	buf.vaddr = cluster->vaddr;
	buf.length = cluster->length;
	v4l2_emu_qbuf(&buf);

	IMP_LOG_DBG(TAG, "release cluster:%d \n", buf.index);

	return 0;
}

void* frame_pooling_thread(void *p)
{
	Framesource *fs = (Framesource *)p;

	while (1) {
		IMP_LOG_DBG(TAG, "Tick\n");

		Group *group = fs->device->groups[0];
		group_tick(group);
		usleep(1000000 / fs->attr.inFrmRate);
	}

	return NULL;
}

static int framesource_init(Framesource *framesource)
{
	int ret, i;

	/* Init Channels. */
	int nr_chn_to_enable = 0;
	for (i = 0; i < NR_MAX_FS_CHN_IN_GROUP; i++) {
		FSChannel *channel = &framesource->channel[i];
		if (channel->is_enabled)
			nr_chn_to_enable++;
	}

	/* Init VBM. */
	VBMConfig vbm_config;
	vbm_config.nr_pools = nr_chn_to_enable;
	vbm_config.nr_cluster = framesource->attr.nrVBs;
	for (i = 0; i < vbm_config.nr_pools; i++) {
		FSChannel *channel = &framesource->channel[i];
		vbm_config.pool_buffer_size[i] =
			calc_pic_size(channel->attr.picWidth, channel->attr.picHeight, channel->attr.pixFmt);
	}

	VBMInterface vbm_interface;
	vbm_interface.pri = framesource;
	vbm_interface.GetCluster = get_cluster;
	vbm_interface.ReleaseCluster = release_cluster;

	ret = VBMInit(&vbm_config, &vbm_interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "VBMInit() failed, ret=%d", ret);
		goto close;
	}

	nr_emu_vb = framesource->attr.nrVBs;

	return 0;
close:
	return -1;
}

static void framesource_exit(Framesource *framesource)
{
	int ret;

	ret = VBMExit();
	if (ret < 0)
		IMP_LOG_ERR(TAG, "VBMExit() failed, ret=%d", ret);

	nr_emu_vb = 0;
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
	dev->dev_id = DEV_ID_EMU_FS;

	framesource = device_pri(dev);
	framesource->device = dev;

	return framesource;
}

static void free_framesource(Framesource *framesource)
{
	Device *dev = framesource->device;
	free_device(dev);
}

static int on_framesource_group_data_update(Group *group, IMPFrameInfo *d)
{
	Device *dev = get_device_of_group(group);
	Framesource *framesource = (Framesource *)device_pri(dev);

	IMP_LOG_DBG(TAG, "[%s][%s] update\n",
				framesource->device->name, group->module->name);

	IMPFrameInfo *frames[NR_MAX_FS_CHN_IN_GROUP];
	int nr_frames, ret;
	ret = VBMGetFrames(frames, &nr_frames);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "VBMGetFrames() error:%d\n", ret);
		return -1;
	}

	int i;
	for (i = 0; i < nr_frames; i++) {
		int channel_id = frames[i]->pool_idx;
//		IMP_LOG_ERR(TAG, "%s:%d channel_id=%d frames[i]=%p\n", __func__, __LINE__, channel_id, frames[i]);
		group->for_channel_data[channel_id] = frames[i];
	}

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

	ret = framesource_create_group(0);
	if (ret < 0) {
		free_framesource(gFramesource);
		gFramesource = NULL;
	}

	return ret;
}

int EmuFrameSourceExit(void)
{
	if (gFramesource) {
		framesource_destroy_group(0);
		free_framesource(gFramesource);
		gFramesource = NULL;
	}

	return 0;
}

/* APIs */
int IMP_EmuFrameSource_SetDevAttr(IMPFSDevAttr *dev_attr)
{
	if (dev_attr == NULL) {
		IMP_LOG_ERR(TAG, "%s(): dev_attr is NULL\n");
		return -1;
	}

	Framesource *framesource = get_framesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	framesource->attr = *dev_attr;

	return 0;
}

int IMP_EmuFrameSource_GetDevAttr(IMPFSDevAttr *dev_attr)
{
	if (dev_attr == NULL) {
		IMP_LOG_ERR(TAG, "%s(): dev_attr is NULL\n");
		return -1;
	}

	Framesource *framesource = get_framesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	if (framesource->attr.inFrmRate == 0) {
		IMP_LOG_ERR(TAG, "%s(): dev_attr was not set yet\n");
		return -1;
	} else {
		*dev_attr = framesource->attr;
	}

	return 0;
}

int IMP_EmuFrameSource_SetChnAttr(uint32_t chn_num, IMPFSChnAttr *chn_attr)
{
	if (chn_attr == NULL) {
		IMP_LOG_ERR(TAG, "%s(): chn_attr is NULL\n");
		return -1;
	}

	if (chn_num > (NR_MAX_FS_CHN_IN_GROUP - 1)) {
		IMP_LOG_ERR(TAG, "%s(): Invalid chn_num %d\n", chn_num);
		return -1;
	}

	Framesource *framesource = get_framesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	FSChannel *channel = &framesource->channel[chn_num];

	channel->attr = *chn_attr;

	return 0;
}

int IMP_EmuFrameSource_GetChnAttr(uint32_t chn_num, IMPFSChnAttr *chn_attr)
{
	if (chn_attr == NULL) {
		IMP_LOG_ERR(TAG, "%s(): chn_attr is NULL\n");
		return -1;
	}

	if (chn_num > (NR_MAX_FS_CHN_IN_GROUP - 1)) {
		IMP_LOG_ERR(TAG, "%s(): Invalid chn_num %d\n", chn_num);
		return -1;
	}

	Framesource *framesource = get_framesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	FSChannel *channel = &framesource->channel[chn_num];

	if (channel->attr.picWidth == 0) {
		IMP_LOG_ERR(TAG, "%s(): chn_attr was not set yetNULL\n");
		return -1;
	} else {
		*chn_attr = channel->attr;
	}

	return 0;
}

int IMP_EmuFrameSource_EnableChn(uint32_t chn_num)
{
	if (chn_num > (NR_MAX_FS_CHN_IN_GROUP - 1)) {
		IMP_LOG_ERR(TAG, "%s(): Invalid chn_num %d\n", chn_num);
		return -1;
	}

	Framesource *framesource = get_framesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	FSChannel *channel = &framesource->channel[chn_num];

	channel->is_enabled = 1;

	return 0;
}

int IMP_EmuFrameSource_DisableChn(uint32_t chn_num)
{
	if (chn_num > (NR_MAX_FS_CHN_IN_GROUP - 1)) {
		IMP_LOG_ERR(TAG, "%s(): Invalid chn_num %d\n", chn_num);
		return -1;
	}

	Framesource *framesource = get_framesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	FSChannel *channel = &framesource->channel[chn_num];

	channel->is_enabled = 0;

	return 0;
}

int IMP_EmuFrameSource_EnableDev(void)
{
	Framesource *framesource = get_framesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	int ret = framesource_init(framesource);
	if (ret < 0)
		return -1;

	return 0;
}

int IMP_EmuFrameSource_DisableDev(void)
{
	Framesource *framesource = get_framesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	framesource_exit(framesource);

	return 0;
}

int IMP_EmuFrameSource_StreamOn(void)
{
	Framesource *framesource = get_framesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	int ret = VBMFillFrames();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "%s(): VBMFillFrames() failed, ret=%d", ret);
		return -1;
	}

	/* Init the tick thread. */
	ret = pthread_create(&framesource->tid, NULL,
						 frame_pooling_thread, framesource);
	if (ret) {
		IMP_LOG_ERR(TAG, "thread create error\n");
		return -1;
	}

	return 0;
}

int IMP_EmuFrameSource_StreamOff(void)
{
	Framesource *framesource = get_framesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	pthread_cancel(framesource->tid);
	pthread_join(framesource->tid, NULL);

	int ret = VBMFlushFrames();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "%s(): VBMFlushFrames() failed, ret=%d", ret);
		return -1;
	}


	return 0;
}
