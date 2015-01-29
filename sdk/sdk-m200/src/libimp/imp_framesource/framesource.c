/*
 * Ingenic IMP framesource solution.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Tiger <lbh@ingenic.cn>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include <imp/imp_constraints.h>
#include <imp/imp_log.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_utils.h>

#include <system/module.h>
#include <system/group.h>
#include <system/device.h>
#include <system/vbm.h>

#define TAG "Framesource"

/* V4L2 extend */
#define V4L2_CID_FLOW_VIDEONUM			(V4L2_CID_PRIVATE_BASE + 1)
#define V4L2_CID_FLOW_CFG_VIDEO			(V4L2_CID_PRIVATE_BASE + 2)

typedef struct {
	int is_enabled;
	IMPFSChnAttr attr;
} FSChannel;

typedef struct {
	Device *device;
	pthread_t tid;
	int v4l2_fd;
	int time[NR_MAX_FS_GROUPS][NR_MAX_FS_CHN_IN_GROUP];
	int frame_count[NR_MAX_FS_GROUPS][NR_MAX_FS_CHN_IN_GROUP];
	IMPFSDevAttr attr;
	FSChannel channel[NR_MAX_FS_CHN_IN_GROUP];
} Framesource;

static inline int imppixfmt_to_v4l2pixfmt(IMPPixelFormat imp_pixfmt)
{
	int v4l2_pixfmt = 0;
	switch (imp_pixfmt) {
	case PIX_FMT_NV12: v4l2_pixfmt = V4L2_PIX_FMT_NV12; break;
	case PIX_FMT_YUYV422: v4l2_pixfmt = V4L2_PIX_FMT_YUYV; break;
	default: v4l2_pixfmt = -1; break;
	}
	return v4l2_pixfmt;
}

static inline int calc_pic_size(int width, int height, IMPPixelFormat imp_pixfmt)
{
	int bpp1 = 0, bpp2 = 1,size;

#define BPP(FMT, A, B) case FMT: bpp1 = A;bpp2 = B;break
	switch (imp_pixfmt) {
		BPP(PIX_FMT_NV12, 3, 2);
		BPP(PIX_FMT_YUYV422, 2, 1);
	default: break;
	}
#undef BPP
	size = width * height * bpp1 / bpp2;

	return size;
}

static Framesource *gFramesource = NULL;

Framesource *GetFramesource(void)
{
	return gFramesource;
}

void SetFramesource(Framesource *framesource)
{
	gFramesource = framesource;
}

static int get_cluster(int *cluster_idx, uint64_t *time, void *pri)
{
	Framesource *framesource = (Framesource *)pri;

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

	IMP_LOG_DBG(TAG, "get cluster:%d \n", buf.index);

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

	IMP_LOG_DBG(TAG, "release cluster:%d \n", buf.index);

	return 0;
}

void* frame_pooling(void *p)
{
	int ret = 0;
	Framesource *fs = (Framesource *)p;

	while (1) {
		IMP_LOG_DBG(TAG, "Tick\n");

		struct timeval tv = { 2, 0 };
		fd_set fds;

		FD_ZERO(&fds);
		FD_SET(fs->v4l2_fd, &fds);

		int ret = select(fs->v4l2_fd + 1, &fds, NULL, NULL, &tv);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "select() error: %s\n", strerror(errno));
			return NULL;
		} else if (ret == 0) {
			IMP_LOG_ERR(TAG, "select() timeout\n");
			continue;
		}

		if(!FD_ISSET(fs->v4l2_fd, &fds))
			continue;

		IMP_LOG_DBG(TAG, "select !\n");
		Group *group = fs->device->groups[0];
		group_tick(group);
	}

	return NULL;
}

static int framesource_init(Framesource *framesource)
{
	int ret, i;

	/* Init V4L2 Device */
	const char *dev_name = "/dev/video0";
	framesource->v4l2_fd = open(dev_name, O_RDWR);
	if (framesource->v4l2_fd < 0) {
		IMP_LOG_ERR(TAG, "open %s error: %s\n", dev_name, strerror(errno));
		return -1;
	}

	int input_num = 0;
	ret = ioctl(framesource->v4l2_fd, VIDIOC_S_INPUT, &input_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Unable to set video input device: %s\n",
					strerror(errno));
		goto close;
	}

	/* Init Channels. */
	int nr_chn_to_enable = 0; /* Ugly implement due to ugly V4L2. */
	for (i = 0; i < NR_MAX_FS_CHN_IN_GROUP; i++) {
		FSChannel *channel = &framesource->channel[i];
		if (channel->is_enabled)
			nr_chn_to_enable++;
	}

	struct v4l2_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id = V4L2_CID_FLOW_VIDEONUM;
	ctrl.value = nr_chn_to_enable;
	if (ioctl(framesource->v4l2_fd, VIDIOC_S_CTRL, &ctrl) < 0) {
		IMP_LOG_ERR(TAG, "Unable to set video ctrol: %s\n",
					strerror(errno));
		goto close;
	}

	for (i = 0; i < NR_MAX_FS_CHN_IN_GROUP; i++) {
		FSChannel *channel = &framesource->channel[i];
		if (!channel->is_enabled)
			continue;

		/* Choose one V4L2 channel. */
		memset(&ctrl, 0, sizeof(ctrl));
		ctrl.id = V4L2_CID_FLOW_CFG_VIDEO;
		ctrl.value = i;
		if (ioctl(framesource->v4l2_fd, VIDIOC_S_CTRL, &ctrl) < 0) {
			IMP_LOG_ERR(TAG, "Unable to set video ctrol: %s\n",
						strerror(errno));
			goto close;
		}

		/* Init V4L2 Channel. */
		struct v4l2_format fmt;
		memset(&fmt, 0, sizeof fmt);
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.width = channel->attr.picWidth;
		fmt.fmt.pix.height = channel->attr.picHeight;
		fmt.fmt.pix.pixelformat = imppixfmt_to_v4l2pixfmt(channel->attr.pixFmt);
		fmt.fmt.pix.field = V4L2_FIELD_ANY;

		if (ioctl(framesource->v4l2_fd, VIDIOC_S_FMT, &fmt) < 0) {
			IMP_LOG_ERR(TAG, "Unable to set video format(chn%d): %s\n",
						i, strerror(errno));
			goto close;
		}
	}

	if (framesource->attr.inFrmRate != 0) {
		struct v4l2_streamparm v4l2_streamparm;
		v4l2_streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		v4l2_streamparm.parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
		v4l2_streamparm.parm.capture.capturemode = V4L2_MODE_HIGHQUALITY;
		v4l2_streamparm.parm.capture.timeperframe.numerator = 1;
		v4l2_streamparm.parm.capture.timeperframe.denominator = framesource->attr.inFrmRate;
		ioctl(framesource->v4l2_fd, VIDIOC_S_PARM, &v4l2_streamparm);
	}

	/* Init V4L2 videobuffer, USERPTR support only. */
	struct v4l2_requestbuffers req;
	memset(&req, 0, sizeof req);
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;
	req.count = framesource->attr.nrVBs;
	if (ioctl(framesource->v4l2_fd, VIDIOC_REQBUFS, &req) < 0) {
		IMP_LOG_ERR(TAG, "VIDIOC_REQBUFS error: %s\n", strerror(errno));
		goto close;
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

	return 0;
close:
	close(framesource->v4l2_fd);
	return -1;
}

static void framesource_exit(Framesource *framesource)
{
	int ret;


	close(framesource->v4l2_fd);

	ret = VBMExit();
	if (ret < 0)
		IMP_LOG_ERR(TAG, "VBMExit() failed, ret=%d", ret);
}

static Framesource *framesource_create_device(void)
{
	int i, j;
	Device *dev;
	Framesource *framesource;

	dev = alloc_device("Framesource", sizeof(Framesource));
	if (dev == NULL) {
		IMP_LOG_ERR(TAG, "alloc_device() error\n");
		return NULL;
	}

	dev->dev_id = DEV_ID_FS;
	dev->nr_groups = NR_MAX_FS_GROUPS;

	framesource = device_pri(dev);
	framesource->device = dev;
	for(i = 0; i < NR_MAX_FS_GROUPS; i++)
		for(j = 0; j < NR_MAX_FS_CHN_IN_GROUP; j++) {
			framesource->time[i][j] = 0;
			framesource->frame_count[i][j] = 0;
		}

	return framesource;
}

static void free_framesource(Framesource *framesource)
{
	Device *dev = framesource->device;
	free_device(dev);
}

static int on_framesource_group_data_update(Group *group, IMPFrameInfo *frame)
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
		struct timeval tv;
		gettimeofday(&tv, NULL);

		if(tv.tv_sec != framesource->time[0][channel_id]) {
			framesource->time[0][channel_id] = tv.tv_sec;
			framesource->frame_count[0][channel_id] = 0;
		} else {
			framesource->frame_count[0][channel_id]++;
		}

		if(framesource->frame_count[0][channel_id] >= framesource->channel[channel_id].attr.outFrmRate) {
			VBMReleaseFrame(frames[i]);
			return 0;
		}

		IMP_LOG_DBG(TAG, "%s:%d channel_id=%d frames[i]=%p frame_count[0][%d] = %d, outFrmRate = %d\n", __func__, __LINE__, channel_id, frames[i], channel_id, framesource->frame_count[0][channel_id], framesource->channel[channel_id].attr.outFrmRate);

		group->for_channel_data[channel_id] = frames[i];
	}

	return 0;
}

static int framesource_create_group(int group_index, Framesource *framesource)
{
	Device *dev;
	char grp_name[MAX_MODULE_NAME_LEN];

	dev = framesource->device;

	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "Invalid group num%d\n", group_index);
		return -1;
	}

	if (group_index > dev->nr_groups - 1) {
		IMP_LOG_ERR(TAG, "Invalid group num%d\n", group_index);
		return -1;
	}

	sprintf(grp_name, "%s-%d", dev->name, group_index);

	Group *grp = create_group(dev->dev_id, group_index, grp_name,
							  on_framesource_group_data_update);
	grp->device = dev;
	grp->nr_channels = NR_MAX_FS_CHN_IN_GROUP;
	dev->groups[group_index] = grp;

	return 0;
}

static int framesource_destroy_group(int group_index)
{
	Framesource *framesource = GetFramesource();
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

	destroy_group(grp, DEV_ID_FS); /* TODO return value. */
	dev->groups[group_index] = NULL;

	return 0;
}

/* APIs */
int IMP_FrameSource_SetDevAttr(IMPFSDevAttr *dev_attr)
{
	int ret;
	Framesource *framesource;

	if (dev_attr == NULL) {
		IMP_LOG_ERR(TAG, "%s(): dev_attr is NULL\n");
		return -1;
	}

	framesource = framesource_create_device();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	ret = framesource_create_group(0, framesource);
	if (ret < 0) {
		free_framesource(framesource);
		framesource = NULL;
	}

	framesource->attr = *dev_attr;

	SetFramesource(framesource);

	return 0;
}

int IMP_FrameSource_GetDevAttr(IMPFSDevAttr *dev_attr)
{
	if (dev_attr == NULL) {
		IMP_LOG_ERR(TAG, "%s(): dev_attr is NULL\n");
		return -1;
	}

	Framesource *framesource = GetFramesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	*dev_attr = framesource->attr;

	return 0;
}

int IMP_FrameSource_SetChnAttr(uint32_t chn_num, IMPFSChnAttr *chn_attr)
{
	if (chn_attr == NULL) {
		IMP_LOG_ERR(TAG, "%s(): chn_attr is NULL\n");
		return -1;
	}

	Framesource *framesource = GetFramesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	if (chn_num > (NR_MAX_FS_CHN_IN_GROUP - 1)) {
		IMP_LOG_ERR(TAG, "%s(): Invalid chn_num %d\n", chn_num);
		return -1;
	}

	FSChannel *channel = &framesource->channel[chn_num];

	channel->attr = *chn_attr;

	return 0;
}

int IMP_FrameSource_GetChnAttr(uint32_t chn_num, IMPFSChnAttr *chn_attr)
{
	if (chn_attr == NULL) {
		IMP_LOG_ERR(TAG, "%s(): chn_attr is NULL\n");
		return -1;
	}

	Framesource *framesource = GetFramesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	if (chn_num > (NR_MAX_FS_CHN_IN_GROUP - 1)) {
		IMP_LOG_ERR(TAG, "%s(): Invalid chn_num %d\n", chn_num);
		return -1;
	}

	FSChannel *channel = &framesource->channel[chn_num];

	*chn_attr = channel->attr;

	return 0;
}

int IMP_FrameSource_EnableChn(uint32_t chn_num)
{
	Framesource *framesource = GetFramesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	if (chn_num > (NR_MAX_FS_CHN_IN_GROUP - 1)) {
		IMP_LOG_ERR(TAG, "%s(): Invalid chn_num %d\n", chn_num);
		return -1;
	}

	FSChannel *channel = &framesource->channel[chn_num];

	channel->is_enabled = 1;

	return 0;
}

int IMP_FrameSource_DisableChn(uint32_t chn_num)
{
	Framesource *framesource = GetFramesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	if (chn_num > (NR_MAX_FS_CHN_IN_GROUP - 1)) {
		IMP_LOG_ERR(TAG, "%s(): Invalid chn_num %d\n", chn_num);
		return -1;
	}

	FSChannel *channel = &framesource->channel[chn_num];

	channel->is_enabled = 0;

	return 0;
}

int IMP_FrameSource_EnableDev(void)
{
	Framesource *framesource = GetFramesource();
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

int IMP_FrameSource_DisableDev(void)
{
	Framesource *framesource = GetFramesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	framesource_exit(framesource);

	if (framesource) {
		framesource_destroy_group(0);
		free_framesource(framesource);
		framesource = NULL;
	}

	return 0;
}

int IMP_FrameSource_StreamOn(void)
{
	Framesource *framesource = GetFramesource();
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

	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(framesource->v4l2_fd, VIDIOC_STREAMON, &type) < 0) {
		IMP_LOG_ERR(TAG, "VIDIOC_STREAMON failed: %s", strerror(errno));
		return -1;
	}

	/* Init the tick thread. */
	ret = pthread_create(&framesource->tid, NULL,
						 frame_pooling, framesource);
	if (ret) {
		IMP_LOG_ERR(TAG, "thread create error\n");
		return -1;
	}

	return 0;
}

int IMP_FrameSource_StreamOff(void)
{
	Framesource *framesource = GetFramesource();
	if (framesource == NULL) {
		IMP_LOG_ERR(TAG, "%s(): FrameSource is invalid,"
					"maybe system was not inited yet.\n");
		return -1;
	}

	pthread_cancel(framesource->tid);

	int ret = VBMFlushFrames();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "%s(): VBMFlushFrames() failed, ret=%d", ret);
		return -1;
	}


	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(framesource->v4l2_fd, VIDIOC_STREAMOFF, &type) < 0) {
		IMP_LOG_ERR(TAG, "VIDIOC_STREAMOFF failed: %s", strerror(errno));
		return -1;
	}

	return 0;
}
