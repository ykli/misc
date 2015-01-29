/*
 * Encoder test.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/Basic.h>
#include <CUnit/Console.h>

#include <fcntl.h>
#include <unistd.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>

#include "../src/libimp/include/system/system.h"
#include "../src/libimp/include/emulator/imp_emu_framesource.h"

#define TAG "EncoderTest"

#define SENSOR_FRAME_RATE		2
#define MAX_FRAMES_TO_SAVE		10
#define STREAM_BUFFER_SIZE		(2 * 1024 * 1024)
#define STREAM_FILE_PATH		"/tmp/stream.264"

static int emu_framesource_init(void)
{
	int ret;

	/* Configure Dev attr */
	IMPFSDevAttr imp_dev_attr;
	imp_dev_attr.inFrmRate = SENSOR_FRAME_RATE;
	imp_dev_attr.nrVBs = 4;
	ret = IMP_EmuFrameSource_SetDevAttr(&imp_dev_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_SetDevAttr() error: %d\n", ret);
		return -1;
	}

	/* Check Dev attr */
	IMPFSDevAttr imp_dev_attr_check;
	ret =IMP_EmuFrameSource_GetDevAttr(&imp_dev_attr_check);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_GetDevAttr() error: %d\n", ret);
		return -1;
	}
	IMP_LOG_DBG(TAG, "FS DevAttr.inFrmRate=%d\n", imp_dev_attr_check.inFrmRate);
	IMP_LOG_DBG(TAG, "FS DevAttr.nrVBs=%d\n", imp_dev_attr_check.nrVBs);

	/* Configure channel 0 attr */
	IMPFSChnAttr imp_chn_attr;
	imp_chn_attr.picWidth = 1280;
	imp_chn_attr.picHeight = 720;
	imp_chn_attr.pixFmt = PIX_FMT_NV12;
	imp_chn_attr.outFrmRate = imp_dev_attr.inFrmRate;
	ret = IMP_EmuFrameSource_SetChnAttr(0, &imp_chn_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_SetChnAttr(0) error: %d\n", ret);
		return -1;
	}

	/* Check channel 0 attr */
	IMPFSChnAttr imp_chn_attr_check;
	ret = IMP_EmuFrameSource_GetChnAttr(0, &imp_chn_attr_check);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_GetChnAttr(1) error: %d\n", ret);
		return -1;
	}
	IMP_LOG_DBG(TAG, "FS Chn0 ChnAttr.picWidth=%d\n", imp_chn_attr_check.picWidth);
	IMP_LOG_DBG(TAG, "FS Chn0 ChnAttr.picHeight=%d\n", imp_chn_attr_check.picHeight);
	IMP_LOG_DBG(TAG, "FS Chn0 ChnAttr.pixFmt=%d\n", imp_chn_attr_check.pixFmt);
	IMP_LOG_DBG(TAG, "FS Chn0 ChnAttr.outFrmRate=%d\n", imp_chn_attr_check.outFrmRate);

	/* Enable channels */
	ret = IMP_EmuFrameSource_EnableChn(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_EnableChn(0) error: %d\n", ret);
		return -1;
	}

	/* Enable Device */
	ret = IMP_EmuFrameSource_EnableDev();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_EnableDev error: %d\n", ret);
		return -1;
	}

	IMP_LOG_DBG(TAG, "%s(): OK.\n", __func__);

	return 0;
}

static int encoder_init(void)
{
	int ret;

	/* Creat Encoder Group */
	ret = IMP_Encoder_CreateGroup(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(0) error: %d\n", ret);
		return -1;
	}

	/* Init Channel attr */
	IMPEncoderAttr enc_attr;
	enc_attr.enType = PT_H264;
	enc_attr.attrH264.bufSize = STREAM_BUFFER_SIZE;
	enc_attr.attrH264.profile = 0;
	enc_attr.attrH264.byFrame = 1;
	enc_attr.attrH264.picWidth = 1280;
	enc_attr.attrH264.picHeight = 720;

	IMPEncoderRcAttr rc_attr;
	rc_attr.rcMode = ENC_RC_MODE_H264VBR;
	rc_attr.attrH264Vbr.maxGop = 25;
	rc_attr.attrH264Vbr.inFrmRate = SENSOR_FRAME_RATE;
	rc_attr.attrH264Vbr.outFrmRate = SENSOR_FRAME_RATE;
	rc_attr.attrH264Vbr.maxBitRate = 1800;
	rc_attr.attrH264Vbr.maxQp = 34;
	rc_attr.attrH264Vbr.minQp = 26;

	IMPEncoderCHNAttr channel_attr;
	channel_attr.encAttr = enc_attr;
	channel_attr.rcAttr = rc_attr;

	/* Create Channel */
	ret = IMP_Encoder_CreateChn(0, &channel_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(0) error: %d\n", ret);
		return -1;
	}

	/* Resigter Channel */
	ret = IMP_Encoder_RegisterChn(0, 0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(0, 0) error: %d\n", ret);
		return -1;
	}

	return 0;
}

static int bind_modules(void)
{
	int ret;
	IMPChannel framesource_chn0 = { DEV_ID_EMU_FS, 0, 0};
	IMPChannel encoder = { DEV_ID_ENC, 0, 0};

	ret = IMP_System_Bind(&framesource_chn0, &encoder);
	if (ret < 0)
		return ret;

	return 0;
}

static int unbind_modules(void)
{
	return 0;
}

static int encoder_exit(void)
{
	return 0;
}
static int emu_framesource_exit(void)
{
	return 0;
}

static int open_stream_file(void)
{
	int fd = open(STREAM_FILE_PATH, O_RDWR | O_CREAT);
	return fd;
}

static int save_stream(int fd, IMPEncoderStream *stream)
{
	int ret, i, nr_pack = stream->packCount;

	for (i = 0; i < nr_pack; i++) {
		ret = write(fd, (void *)stream->pack[i].virAddr,
					stream->pack[i].length);
		if (ret != stream->pack[i].length) {
			IMP_LOG_ERR(TAG, "stream write error:%s\n", strerror(errno));
			return -1;
		}
	}

	return 0;
}

static void close_stream_file(int fd)
{
	close(fd);
}

static int do_get_stream(void)
{
	int ret;
	ret = IMP_Encoder_StartRecvPic(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic() failed\n");
		return -1;
	}

	int i, stream_fd = open_stream_file();

	for (i = 0; i < MAX_FRAMES_TO_SAVE; i++) {
		IMPEncoderStream stream;
		IMP_Encoder_PoolingStream(0, 0);

		IMPEncoderCHNStat chn_stat;
		IMP_Encoder_Query(0, &chn_stat);
		stream.packCount = chn_stat.curPacks;

		IMP_Encoder_GetStream(0, &stream, 1);
		ret = save_stream(stream_fd, &stream);
		if (ret < 0) {
			close(stream_fd);
			return ret;
		}

		IMP_Encoder_ReleaseStream(0, &stream);
	}

	close_stream_file(stream_fd);

	ret = IMP_Encoder_StopRecvPic(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic() failed\n");
		return -1;
	}

	return 0;
}

static void test1_base_stream(void)
{
	int ret;
	ret = IMP_System_Init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Init() failed\n");
		CU_FAIL();
		return;
	}

	ret = emu_framesource_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Emu FrameSource init failed\n");
		goto error;
	}

	ret = encoder_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder init failed\n");
		goto error;
	}

	ret = bind_modules();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Bind modules failed\n");
		goto error;
	}

	ret = IMP_EmuFrameSource_StreamOn();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOn failed\n");
		goto error;
	}

	//get stream here.
	ret = do_get_stream();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "get stream failed\n");
 		goto error;
	}

	ret = IMP_EmuFrameSource_StreamOff();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
 		goto error;
	}

	ret = unbind_modules();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Bind modules failed\n");
		goto error;
	}

	ret = encoder_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Bind modules failed\n");
		goto error;
	}

	ret = emu_framesource_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Emu FrameSource init failed\n");
		goto error;
	}

	ret = IMP_System_Exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Exit() failed\n");
		goto error;
	}

	CU_PASS();

	return;
error:
	ret = IMP_System_Exit();
	if (ret < 0)
		IMP_LOG_ERR(TAG, "IMP_System_Exit() failed\n");
	CU_FAIL();
}

CU_TestInfo EncoderTest[] = {
	{"Base stream test(EmuFramesource + Encoder)", test1_base_stream},
	CU_TEST_INFO_NULL,
};
