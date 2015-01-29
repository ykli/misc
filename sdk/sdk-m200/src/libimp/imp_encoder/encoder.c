/*
 * IMP Encoder implement.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#include <imp/imp_constraints.h>
#include <imp/imp_log.h>
#include <imp/imp_system.h>
#include <imp/imp_utils.h>
#include <imp/imp_encoder.h>

#include <system/system.h>
#include <system/module.h>
#include <system/group.h>
#include <system/device.h>

#include <x264.h>

#define TAG "Encoder"

#define NR_MAX_STREAM_BUFFER 4
#define MAX_STREAM_BUFFER_SIZE (2 * 1024 * 1024)


typedef struct {
	uint32_t		seq;
	int				in_use;
	uint32_t		packCount;
	IMPEncoderPack	pack[0];
} FrameStream;

typedef struct {
	x264_param_t		x264Param;
	x264_t				*x264Handler;
	x264_picture_t		xPicIn;
	x264_picture_t		*xPicOut;
	x264_nal_t			*nals;
	int					pts;
} X264Data;

typedef struct {
	int reserved;
} JpegData;

typedef struct {
	int index;
	IMPEncoderCHNAttr attr;

	int recv_pic;
	int idr_request;

	sem_t wr_sem;
	sem_t rd_sem;

	int nr_stream_buffer;
	FrameStream **frame_stream;	/* Cyclic Buffer */
	int cur_rd_frmstrm_idx;		/* Ugly name */
	int cur_wr_frmstrm_idx;

	X264Data x264;
	JpegData jpeg;
} EncChannel;

typedef struct {
	int nr_enc_chn;
	EncChannel *channel[NR_MAX_ENC_CHN_IN_GROUP];
} EncGroup;

typedef struct {
	Device *device;

	EncGroup encGroup[NR_MAX_ENC_GROUPS];
} Encoder;

static EncChannel g_EncChannel[NR_MAX_ENC_CHN];
static Encoder *gEncoder = NULL;

static Encoder *get_encoder(void)
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
	dev->dev_id = DEV_ID_EMU_ENC;

	encoder = device_pri(dev);
	encoder->device = dev;

	return encoder;
}

static void free_encoder(Encoder *encoder)
{
	Device *dev = encoder->device;
	free_device(dev);
}

static int do_channel_process(EncChannel *enc_chn, IMPFrameInfo *frame)
{
#if 0
	int ret;
	int buf_index = enc_chn->cur_wrbuf_idx % enc_chn->nr_stream_buffer;
	EmuStreamBuffer *buffer = &enc_chn->streamBuffer[buf_index];

	ret = sem_trywait(&enc_chn->wr_sem);
	if (ret < 0) {
		IMP_LOG_DBG(TAG, "Encoder chn%d is full", enc_chn->index);
		return -1;
	}
	buffer->length = (rand() % MAX_STREAM_BUFFER_SIZE) | 1;
	buffer->timestamp = frame->timeStamp;
	memcpy((void *)buffer->buf, (void *)frame->virAddr, buffer->length);

	enc_chn->cur_wrbuf_idx++;

	usleep(10000);		/* It seems that we are working. */

	sem_post(&enc_chn->rd_sem);
#endif
	return 0;
}

static int on_encoder_group_data_update(Group *group, IMPFrameInfo *frame)
{
	Device *dev = get_device_of_group(group);
	Encoder *encoder = (Encoder *)device_pri(dev);

	IMP_LOG_DBG(TAG, "[%s][%s] update addr=0x%08x, timestamp=%llu\n",
				encoder->device->name, group->module->name,
				frame->virAddr, frame->timeStamp);

	EncGroup *enc_group = &encoder->encGroup[group->group_index];
	int i;
	for (i = 0; i < enc_group->nr_enc_chn; i++) {
		EncChannel *enc_channel = enc_group->channel[i];
		if (enc_channel->recv_pic)
			do_channel_process(enc_channel, frame);
	}

	return 0;
}

int EncoderInit(void)
{
	int ret, i;

	gEncoder = alloc_encoder();
	if (gEncoder == NULL)
		return -1;

	ret = encoder_init(gEncoder);
	if (ret < 0)
		goto free;

	/* g_EncChannel create instance */
	memset(g_EncChannel, 0, NR_MAX_ENC_CHN * sizeof(EncChannel));
	for (i = 0; i < NR_MAX_ENC_CHN; i++) {
		g_EncChannel[i].index = -1;	/* It's a uninitial sate*/
		g_EncChannel[i].nr_stream_buffer = NR_MAX_STREAM_BUFFER;
	}

	return ret;
free:
	free_encoder(gEncoder);
	gEncoder = NULL;

	return -1;
}

int EncoderExit(void)
{
	if (gEncoder) {
		encoder_exit(gEncoder);
		free_encoder(gEncoder);
		gEncoder = NULL;
	}

	return 0;
}

int IMP_Encoder_CreateGroup(int encGroup)
{
	Encoder *encoder = get_encoder();
	if (encoder == NULL) {
		IMP_LOG_ERR(TAG, "Invalid group num%d\n", encGroup);
		return -1;
	}

	Device *dev = encoder->device;

	if (encGroup > NR_MAX_ENC_GROUPS - 1) {
		IMP_LOG_ERR(TAG, "Invalid group num%d\n", encGroup);
		return -1;
	}

	char grp_name[MAX_MODULE_NAME_LEN];
	sprintf(grp_name, "%s-%d", dev->name, encGroup);

	Group *grp = create_group(DEV_ID_ENC, encGroup, grp_name,
							  on_encoder_group_data_update);
	grp->device = dev;
	grp->nr_channels = 0;		/* Encoder doesn't have sub-module. */
	dev->groups[encGroup] = grp;

	return 0;
}

int IMP_Encoder_DestroyGroup(int encGroup)
{
	printf("%s\n", __func__);

	return 0;
}

static int encoder_channel_init_x264(EncChannel *enc_chn)
{
	IMPEncoderCHNAttr *chn_attr = &enc_chn->attr;
	x264_t *x264_handler;
	x264_param_t x264_param;

	x264_param_default_preset(&x264_param, "ultrafast", "zerolatency");

	x264_param.i_width = chn_attr->encAttr.attrH264.picWidth;
	x264_param.i_height = chn_attr->encAttr.attrH264.picHeight;
	x264_param.i_bframe = 0; /* B frame is NOT support */
	x264_param.i_csp = X264_CSP_NV12; /* Fixed input format */
	x264_param.b_annexb = 1;
	x264_param.b_cabac = 1;
	x264_param.i_threads = 2;

	switch (chn_attr->rcAttr.rcMode) {
	case ENC_RC_MODE_H264VBR:
		x264_param.i_keyint_max = chn_attr->rcAttr.attrH264Vbr.maxGop;
		x264_param.i_fps_num = chn_attr->rcAttr.attrH264Vbr.inFrmRate;
		x264_param.i_fps_den = 1;
		x264_param.rc.i_rc_method = X264_RC_CRF;
		x264_param.rc.i_qp_max = chn_attr->rcAttr.attrH264Vbr.maxQp;
		x264_param.rc.i_qp_min = chn_attr->rcAttr.attrH264Vbr.minQp;
		x264_param.rc.i_vbv_max_bitrate =
			chn_attr->rcAttr.attrH264Vbr.maxBitRate;
		x264_param.rc.i_vbv_buffer_size =
			x264_param.rc.i_vbv_max_bitrate / 2;
		break;
	case ENC_RC_MODE_H264FIXQP:
	default:
		IMP_LOG_ERR(TAG, "H264 RC Mode %d not support yet\n",
					chn_attr->rcAttr.rcMode);
		return -1;
	}

	x264_handler = x264_encoder_open(&x264_param);
	if (x264_handler == NULL) {
		IMP_LOG_ERR(TAG, "Encoder open error\n");
		return -1;
	}

	x264_picture_t *x264_pic = &enc_chn->x264.xPicIn;
	memset(x264_pic, 0, sizeof(x264_picture_t));

	/* Init x264 picture */
	x264_pic->i_type = X264_TYPE_AUTO;
	x264_pic->i_qpplus1 = 0;
	x264_pic->img.i_csp = x264_param.i_csp;
	x264_pic->img.i_plane = 2;

	x264_pic->img.i_stride[0] = x264_param.i_width;
	x264_pic->img.i_stride[1] = x264_param.i_width;
	x264_pic->param = NULL;

	enc_chn->x264.x264Handler = x264_handler;
	enc_chn->x264.x264Param = x264_param;
	enc_chn->x264.pts = 0;

	return 0;
}

static int encoder_channel_exit_x264(EncChannel *enc_chn)
{
	if (enc_chn->x264.x264Handler == NULL)
		return 0;

	x264_encoder_close(enc_chn->x264.x264Handler);
	enc_chn->x264.x264Handler = NULL;

	if (enc_chn->x264.xPicOut) {
		free(enc_chn->x264.xPicOut);
		enc_chn->x264.xPicOut = NULL;
	}

	return 0;
}

static int encoder_channel_init_jpeg(EncChannel *enc_chn)
{
	IMP_LOG_ERR(TAG, "JPEG is not support yet\n");
	return -1;
}

static int encoder_channel_exit_jpeg(EncChannel *enc_chn)
{
	return 0;
}

static int encoder_channel_init(EncChannel *enc_chn)
{
	int ret = -1;

	switch (enc_chn->attr.encAttr.enType) {
	case PT_H264:
		ret = encoder_channel_init_x264(enc_chn);
		break;
	case PT_JPEG:
		ret = encoder_channel_init_jpeg(enc_chn);
		break;
	default:
		IMP_LOG_ERR(TAG, "Channel init: Unknow payload type %d\n",
					enc_chn->attr.encAttr.enType);
		break;
	}

	return ret;
}

static int encoder_channel_exit(EncChannel *enc_chn)
{
	int ret = -1;

	switch (enc_chn->attr.encAttr.enType) {
	case PT_H264:
		ret = encoder_channel_exit_x264(enc_chn);
		break;
	case PT_JPEG:
		ret = encoder_channel_exit_jpeg(enc_chn);
		break;
	default:
		IMP_LOG_ERR(TAG, "Channel exit: Unknow payload type %d\n",
					enc_chn->attr.encAttr.enType);
		break;
	}

	return ret;
}

static int channel_buffer_init(EncChannel *enc_chn)
{
	int size = enc_chn->nr_stream_buffer * sizeof(FrameStream *);

	enc_chn->frame_stream = malloc(size);
	if (enc_chn->frame_stream == NULL)
		return -1;
	memset(enc_chn->frame_stream, 0, size);

	return 0;
}

static int channel_buffer_exit(EncChannel *enc_chn)
{
	int i;

	for (i = 0; i < enc_chn->nr_stream_buffer; i++) {
		if (enc_chn->frame_stream[i]->packCount)
			free(enc_chn->frame_stream[i]);
	}

	free(enc_chn->frame_stream);

	return 0;
}

int IMP_Encoder_CreateChn(int encChn, const IMPEncoderCHNAttr *attr)
{
	if (encChn > NR_MAX_ENC_CHN) {
		IMP_LOG_ERR(TAG, "Invalid Channel Num: %d\n", encChn);
		return -1;
	}

	if (attr == NULL) {
		IMP_LOG_ERR(TAG, "Channel Attr is NULL\n");
		return -1;
	}

	EncChannel *enc_chn = &g_EncChannel[encChn];
	int ret;

	if (enc_chn->index >= 0) {
		IMP_LOG_ERR(TAG, "This channel has already been created\n");
		return -1;
	}

	enc_chn->index = encChn;
	enc_chn->attr = *attr;

	ret = encoder_channel_init(enc_chn);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Channel%d encoder init failed\n", encChn);
		enc_chn->index = -1;
		return ret;
	}

	ret = channel_buffer_init(enc_chn);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Channel%d stream buffer init failed\n", encChn);
		encoder_channel_exit(enc_chn);
		enc_chn->index = -1;
		return ret;
	}

	return 0;
}

int IMP_Encoder_DestroyChn(int encChn)
{
	if (encChn > NR_MAX_ENC_CHN) {
		IMP_LOG_ERR(TAG, "Invalid Channel Num: %d\n", encChn);
		return -1;
	}
	EncChannel *enc_chn = &g_EncChannel[encChn];

	encoder_channel_exit(enc_chn);
	channel_buffer_exit(enc_chn);

	enc_chn->index = -1;

	return 0;
}

int IMP_Encoder_RegisterChn(int encGroup, int encChn)
{
	if (encGroup > NR_MAX_ENC_GROUPS - 1) {
		IMP_LOG_ERR(TAG, "Invalid Encoder Group Num: %d\n", encGroup);
		return -1;
	}

	if (encChn > NR_MAX_ENC_CHN) {
		IMP_LOG_ERR(TAG, "Invalid Channel Num: %d\n", encChn);
		return -1;
	}

	EncChannel *enc_chn = &g_EncChannel[encChn];
	if (enc_chn->index < 0) {
		IMP_LOG_ERR(TAG, "Encoder Channel%d hasn't been created\n", encChn);
		return -1;
	}

	Encoder *encoder = get_encoder();
	EncGroup *enc_group = &encoder->encGroup[encChn];
	enc_group->channel[encChn] = enc_chn;
	enc_group->nr_enc_chn++;

	return 0;
}

int IMP_Encoder_UnRegisterChn(int encChn)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_Encoder_StartRecvPic(int encChn)
{
	if (encChn > NR_MAX_ENC_CHN) {
		IMP_LOG_ERR(TAG, "Invalid Channel Num: %d\n", encChn);
		return -1;
	}

	EncChannel *enc_chn = &g_EncChannel[encChn];
	if (enc_chn->index < 0) {
		IMP_LOG_ERR(TAG, "Encoder Channel%d hasn't been created\n", encChn);
		return -1;
	}

	enc_chn->recv_pic = 1;

	return 0;
}

int IMP_Encoder_StopRecvPic(int encChn)
{
	if (encChn > NR_MAX_ENC_CHN) {
		IMP_LOG_ERR(TAG, "Invalid Channel Num: %d\n", encChn);
		return -1;
	}

	EncChannel *enc_chn = &g_EncChannel[encChn];
	if (enc_chn->index < 0) {
		IMP_LOG_ERR(TAG, "Encoder Channel%d hasn't been created\n", encChn);
		return -1;
	}

	enc_chn->recv_pic = 0;

	return 0;
}

int IMP_Encoder_Query(int encChn, IMPEncoderCHNStat *stat)
{
	IMP_LOG_ERR(TAG, "%s() is not supported yet.\n", __func__);
	return -1;
}

int IMP_Encoder_GetStream(int encChn, IMPEncoderStream *stream, bool blockFlag)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_Encoder_ReleaseStream(int encChn, IMPEncoderStream *stream)
{
	printf("%s\n", __func__);

	return 0;
}

int IMP_Encoder_PoolingStream(int encChn, uint32_t timeoutMsec)
{
	return 0;
}

int IMP_Encoder_RequestIDR(int encChn)
{
	if (encChn > NR_MAX_ENC_CHN) {
		IMP_LOG_ERR(TAG, "Invalid Channel Num: %d\n", encChn);
		return -1;
	}

	EncChannel *enc_chn = &g_EncChannel[encChn];
	if (enc_chn->index < 0) {
		IMP_LOG_ERR(TAG, "Encoder Channel%d hasn't been created\n", encChn);
		return -1;
	}

	enc_chn->idr_request = 1;

	return 0;
}
