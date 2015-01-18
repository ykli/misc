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
  int is_enabled;
  IMP_FS_ChnAttr attr;
} FSChannel;

typedef struct {
	Device *device;

	pthread_t tid;

	FrameInfo *v4l2_vbs;
	int nr_vbs;

  int v4l2_fd;
  FSChannel channel[NR_MAX_FS_CHN_IN_GROUP];
} Framesource;

static Framesource *gFramesource = NULL;

Framesource *get_framesource(void)
{
	return gFramesource;
}

static int get_cluster(int *cluster_idx, uint64_t *time, void *pri)
{
  Framesource *framesource = (Framesource *)pri;
  VBMCluster *cluster;

  struct v4l2_buffer buf;
  memset(&buf, 0, sizeof buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_USERPTR;
  if (ioctl(framesource->v4l2_fd, VIDIOC_DQBUF, &buf) < 0) {
    IMP_LOG_ERR(TAG, "VIDIOC_DQBUF error: %s\n", strerror(errno));
    return -1;
  }

  *cluster_idx = buf.index;
  *time = (uint64_t)((int64_t)buf.timestamp.tv_sec * 1000000 + (int64_t)buf.timestamp.tv_usec);

  return 0;
}

static int release_cluster(VBMCluster *cluster, void *pri)
{
  Framesource *framesource = (Framesource *)pri;

  struct v4l2_buffer buf;
  memset(&buf, 0, sizeof buf);

  buf.index = cluster->index;
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_USERPTR; /* USERPTR as fixed implement. */
  if (cluster->paddr)
    buf.m.userptr = (long unsigned int)cluster->paddr;
  else
    buf.m.userptr = (long unsigned int)cluster->vaddr;
  buf.length = cluster->length;

  if (ioctl(framesource->v4l2_fd, VIDIOC_QBUF, &buf) < 0) {
    IMP_LOG_ERR(TAG, "VIDIOC_QBUF error: %s\n", strerror(errno));
    return -1;
  }

  return 0;
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

	IMP_LOG_DBG(TAG, "[%s][%s] update\n",
				framesource->device->name, group->module->name);

	FrameInfo *cur_frame[max_chh_macro];
	FrameInfo *frames[NR_MAX_FS_CHN_IN_GROUP];
	int nr_frames, ret;
	ret = VBMGetFrames(frames, &nr_frames);
	if (ret < 0) {
	  /* Error log*/
	  return -1;
	}

	if (nr_frames != nr_channel_enabled)
	  return -1;

	int i;
	for (i = 0; i < nr_frames; i++) {
	  int channel_id = frames[i]->pool_idx;
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

int IMP_EmuFrameSource_SetChnAttr(uint32_t chn_num, IMP_FS_ChnAttr *chn_attr)
{
  if (chn_attr == NULL) {
    IMP_LOG_ERR(TAG, "%s(): chn_attr is NULL\n");
    return -1;
  }

  if (chn_num > (NR_MAX_FS_CHN_IN_GROUP - 1)) {
    IMP_LOG_ERR(TAG, "%s(): Invalid chn_num %d\n", chn_num);
    return -1;
  }

  Framesource *Framesource = get_framesource();
  if (Framesource == NULL) {
    IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
		"maybe system was not inited yet.\n");
    return -1;
  }

  FSChannel *channel = &Framesource->channel[chn_num];

  channel->attr = *chn_attr;

  return 0;
}

int IMP_EmuFrameSource_GetChnAttr(uint32_t chn_num, IMP_FS_ChnAttr *chn_attr)
{
  if (chn_attr == NULL) {
    IMP_LOG_ERR(TAG, "%s(): chn_attr is NULL\n");
    return -1;
  }

  if (chn_num > (NR_MAX_FS_CHN_IN_GROUP - 1)) {
    IMP_LOG_ERR(TAG, "%s(): Invalid chn_num %d\n", chn_num);
    return -1;
  }

  Framesource *Framesource = get_framesource();
  if (Framesource == NULL) {
    IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
		"maybe system was not inited yet.\n");
    return -1;
  }

  FSChannel *channel = &Framesource->channel[chn_num];

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

  Framesource *Framesource = get_framesource();
  if (Framesource == NULL) {
    IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
		"maybe system was not inited yet.\n");
    return -1;
  }

  FSChannel *channel = &Framesource->channel[chn_num];

  channel->is_enabled = 1;

  return 0;
}

int IMP_EmuFrameSource_DisableChn(uint32_t chn_num)
{
  if (chn_num > (NR_MAX_FS_CHN_IN_GROUP - 1)) {
    IMP_LOG_ERR(TAG, "%s(): Invalid chn_num %d\n", chn_num);
    return -1;
  }

  Framesource *Framesource = get_framesource();
  if (Framesource == NULL) {
    IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
		"maybe system was not inited yet.\n");
    return -1;
  }

  FSChannel *channel = &Framesource->channel[chn_num];

  channel->is_enabled = 0;

  return 0;
}

int IMP_EmuFrameSource_EnableDev(void)
{
  if (chn_num > (NR_MAX_FS_CHN_IN_GROUP - 1)) {
    IMP_LOG_ERR(TAG, "%s(): Invalid chn_num %d\n", chn_num);
    return -1;
  }

  Framesource *Framesource = get_framesource();
  if (Framesource == NULL) {
    IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
		"maybe system was not inited yet.\n");
    return -1;
  }

  

  return 0;
}

int IMP_EmuFrameSource_DisableDev(void)
{
  if (chn_num > (NR_MAX_FS_CHN_IN_GROUP - 1)) {
    IMP_LOG_ERR(TAG, "%s(): Invalid chn_num %d\n", chn_num);
    return -1;
  }

  Framesource *Framesource = get_framesource();
  if (Framesource == NULL) {
    IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
		"maybe system was not inited yet.\n");
    return -1;
  }

  return 0;
}

int IMP_EmuFrameSource_StreamOn(void)
{
}

int IMP_EmuFrameSource_StreamOff(void)
{
}
