/*
 * Ingenic IMP emulation Encoder.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
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

#include <system/system.h>
#include <system/module.h>
#include <system/group.h>
#include <system/device.h>
#include <emulator/imp_emu_encoder.h>

#define TAG "emu-Encoder"
#define NR_MAX_STREAM_BUFFER 4
#define MAX_STREAM_BUFFER_SIZE (2 * 1024 * 1024)

typedef struct {
	int index;
	IMPEmuEncoderCHNAttr attr;

	int cur_wrbuf_idx;
	int cur_rdbuf_idx;
	sem_t wr_sem;
	sem_t rd_sem;

	int nr_stream_buffer;
	EmuStreamBuffer streamBuffer[NR_MAX_STREAM_BUFFER];
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
	Device *dev = alloc_device("EmuEncoder", sizeof(Encoder));

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
		do_channel_process(enc_channel, frame);
	}

	return 0;
}

int EmuEncoderInit(void)
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
	grp->nr_channels = 0;		/* Encoder doesn't have sub-module. */
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

int IMP_EmuEncoder_CreateChn(int chnNum, IMPEmuEncoderCHNAttr *chnAttr)
{
	if (chnNum > NR_MAX_ENC_CHN) {
		IMP_LOG_ERR(TAG, "Invalid Channel Num: %d\n", chnNum);
		return -1;
	}

	if (chnAttr == NULL) {
		IMP_LOG_ERR(TAG, "Channel Attr is NULL\n");
		return -1;
	}

	EncChannel *enc_chn = &g_EncChannel[chnNum];
	int i;

	if (enc_chn->index >= 0) {
		IMP_LOG_ERR(TAG, "This channel has already been created\n");
		return -1;
	}

	enc_chn->index = chnNum;
	enc_chn->cur_wrbuf_idx = 0;
	enc_chn->cur_rdbuf_idx = 0;
	memcpy(&enc_chn->attr, chnAttr, sizeof(IMPEmuEncoderCHNAttr));

	for (i = 0; i < enc_chn->nr_stream_buffer; i++) {
		EmuStreamBuffer *stream_buf = &enc_chn->streamBuffer[i];
		stream_buf->buf = valloc(MAX_STREAM_BUFFER_SIZE);
	}

	sem_init(&enc_chn->rd_sem, 0, 0);
	sem_init(&enc_chn->wr_sem, 0, enc_chn->nr_stream_buffer);

	return 0;
}

int IMP_EmuEncoder_DestroyChn(int chnNum)
{
	if (chnNum > NR_MAX_ENC_CHN) {
		IMP_LOG_ERR(TAG, "Invalid Channel Num: %d\n", chnNum);
		return -1;
	}

	EncChannel *enc_chn = &g_EncChannel[chnNum];
	int i;

	for (i = 0; i < enc_chn->nr_stream_buffer; i++) {
		EmuStreamBuffer *stream_buf = &enc_chn->streamBuffer[i];
		free(stream_buf->buf);
	}

	sem_destroy(&enc_chn->rd_sem);
	sem_destroy(&enc_chn->wr_sem);

	enc_chn->index = -1;

	return 0;
}

int IMP_EmuEncoder_RegisterChn(int groupNum, int chnNum)
{
	if (groupNum > NR_MAX_ENC_GROUPS - 1) {
		IMP_LOG_ERR(TAG, "Invalid Encoder Group Num: %d\n", groupNum);
		return -1;
	}

	if (chnNum > NR_MAX_ENC_CHN) {
		IMP_LOG_ERR(TAG, "Invalid Channel Num: %d\n", chnNum);
		return -1;
	}

	EncChannel *enc_chn = &g_EncChannel[chnNum];
	if (enc_chn->index < 0) {
		IMP_LOG_ERR(TAG, "Encoder Channel%d hasn't been created\n", chnNum);
		return -1;
	}

	Encoder *encoder = get_encoder();
	EncGroup *enc_group = &encoder->encGroup[chnNum];
	enc_group->channel[chnNum] = enc_chn;
	enc_group->nr_enc_chn++;

	return 0;
}

int IMP_EmuEncoder_UnregisterChn(int chnNum)
{
	return 0;
}

int IMP_EmuEncoder_GetStream(int chnNum, EmuStreamBuffer *stream, int blockFlag)
{
	EncChannel *enc_chn = &g_EncChannel[chnNum];
	int ret;

	if (blockFlag) {
		ret = sem_trywait(&enc_chn->rd_sem);
		if (ret < 0)
			return -1;
	} else {
		sem_wait(&enc_chn->rd_sem);
	}

	int buf_index = enc_chn->cur_rdbuf_idx % enc_chn->nr_stream_buffer;

	stream->buf = enc_chn->streamBuffer[buf_index].buf;
	stream->length = enc_chn->streamBuffer[buf_index].length;
	stream->timestamp = enc_chn->streamBuffer[buf_index].timestamp;

	enc_chn->cur_rdbuf_idx++;

	return 0;
}

int IMP_EmuEncoder_ReleaseStream(int chnNum, EmuStreamBuffer *stream)
{
	EncChannel *enc_chn = &g_EncChannel[chnNum];

	sem_post(&enc_chn->wr_sem);

	return 0;
}
