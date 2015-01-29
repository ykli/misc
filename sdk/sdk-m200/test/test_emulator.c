/*
 * Ingenic IMP SDK emulator test.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 */

#include <CUnit/CUnit.h>
#include <CUnit/Automated.h>
#include <CUnit/Basic.h>
#include <CUnit/Console.h>

#include <unistd.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>

#include "../src/libimp/include/system/system.h"
#include "../src/libimp/include/emulator/imp_emu_framesource.h"
#include "../src/libimp/include/emulator/imp_emu_encoder.h"
#include "../src/libimp/include/emulator/imp_emu_fakedev.h"

#define TAG "EmulatorTest"

#if 1

/* Fake functions */
static inline int isp_init() { return 0;}
static inline int isp_vb_init() { return 0;}
static inline int start_isp() { return 0;}
static inline int encoder_config() { return 0;}
static inline int encoder_enable_channel() { return 0;}
static inline int start_encoder() { return 0;}
static inline int bind() { return 0;}
static inline int Select() { return 0;}

static int ImpSystemInit()
{
	IMP_LOG_DBG(TAG, "ImpSystemInit\n");

	IMP_System_Init();

	return 0;
}

static int ImpSystemExit()
{
	IMP_LOG_DBG(TAG, "ImpSystemExit\n");
	return 0;
}


static int save_stream(EmuStreamBuffer *stream)
{
	IMP_LOG_DBG(TAG, "stream bufaddr:%p len=%d ts=%lld\n",
				stream->buf, stream->length, stream->timestamp);

	return 0;
}

int IMP_FrameSourceGetMaxChn()
{
	return 0;
}

static int ImpFrameSourceInit()
{
	int ret;

	/* Configure Dev attr */
	IMPFSDevAttr imp_dev_attr;
	imp_dev_attr.inFrmRate = 2;
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

	/* Configure channel 1 attr */
	imp_chn_attr.picWidth = 640;
	imp_chn_attr.picHeight = 360;
	imp_chn_attr.pixFmt = PIX_FMT_YUYV422;
	imp_chn_attr.outFrmRate = imp_dev_attr.inFrmRate;
	ret = IMP_EmuFrameSource_SetChnAttr(1, &imp_chn_attr);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_SetChnAttr(0) error: %d\n", ret);
		return -1;
	}

	/* Check channel 0 attr */
	ret = IMP_EmuFrameSource_GetChnAttr(1, &imp_chn_attr_check);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_GetChnAttr(1) error: %d\n", ret);
		return -1;
	}
	IMP_LOG_DBG(TAG, "FS Chn1 ChnAttr.picWidth=%d\n", imp_chn_attr_check.picWidth);
	IMP_LOG_DBG(TAG, "FS Chn1 ChnAttr.picHeight=%d\n", imp_chn_attr_check.picHeight);
	IMP_LOG_DBG(TAG, "FS Chn1 ChnAttr.pixFmt=%d\n", imp_chn_attr_check.pixFmt);
	IMP_LOG_DBG(TAG, "FS Chn1 ChnAttr.outFrmRate=%d\n", imp_chn_attr_check.outFrmRate);

	/* Enable channels */
	ret = IMP_EmuFrameSource_EnableChn(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_EnableChn(0) error: %d\n", ret);
		return -1;
	}

	ret = IMP_EmuFrameSource_EnableChn(1);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuFrameSource_EnableChn(1) error: %d\n", ret);
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

static int ImpFrameSourceExit()
{
	return 0;
}

static int ImpStreamOn()
{
	IMP_EmuFrameSource_StreamOn();
	return 0;
}
static int ImpStreamOff()
{
	IMP_EmuFrameSource_StreamOff();
	return 0;
}
static int ImpEncoderInit()
{
	int ret;

	ret = IMP_EmuEncoder_CreateGroup(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuEncoder_CreateGroup(0) error: %d\n", ret);
		return -1;
	}

	IMPEmuEncoderCHNAttr chn_attr;
	chn_attr.encAttr = 1;
	chn_attr.encRcAttr = 2;
	IMP_EmuEncoder_CreateChn(0, &chn_attr);

	IMP_EmuEncoder_RegisterChn(0, 0);

	IMP_LOG_DBG(TAG, "%s(): OK.\n", __func__);

	return 0;
}

static int ImpEncoderExit()
{
	return 0;
}

static int ImpFakeDevInit(void)
{
	IMP_EmuFakedev_CreateGroup(0, 0, NULL);
	IMP_EmuFakedev_CreateGroup(1, 0, NULL);
	return 0;
}

static int ImpBindModules()
{
	IMPChannel framesource_chn0 = {DEV_ID_EMU_FS, 0, 0};
	IMPChannel framesource_chn1 = {DEV_ID_EMU_FS, 0, 1};
	IMPChannel encoder = { DEV_ID_EMU_ENC, 0, 0};
	IMPChannel fakedev0 = { DEV_ID_EMU_FAKE(0), 0, 0 };
	IMPChannel fakedev1 = { DEV_ID_EMU_FAKE(1), 0, 0 };

	IMP_System_Bind(&framesource_chn0, &fakedev0);
	IMP_System_Bind(&fakedev0, &encoder);
	IMP_System_Bind(&framesource_chn1, &fakedev1);

	system_bind_dump();

	/* Bind modules here. */
	return 0;
}
static int ImpUnBindModules()
{
	return 0;
}

static void test_case_1() /* SYS+emuFS+emuENC */
{
	int ret;

	ret = ImpSystemInit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP system init failed\n");
		goto error;
	}

	ret = ImpFrameSourceInit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP ISP init failed\n");
		goto error;
	}

	ret = ImpEncoderInit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP Encoder init failed\n");
		goto error;
	}

	ret = ImpFakeDevInit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP FakeDev init failed\n");
		goto error;
	}

	ret = ImpBindModules();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP Encoder init failed\n");
		goto error;
	}

	ret = ImpStreamOn();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP stream on error\n");
		goto error;
	}

	int nr_frame_to_encode = 10;
	while (nr_frame_to_encode--) {
		EmuStreamBuffer stream;
		IMP_EmuEncoder_GetStream(0, &stream, 0);
		save_stream(&stream);
		IMP_EmuEncoder_ReleaseStream(0, &stream);
	}

	ret = ImpStreamOff();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP stream off error\n");
		goto error;
	}

	ImpUnBindModules();
	ImpEncoderExit();
	ImpFrameSourceExit();
	ImpSystemExit();

	CU_PASS();

	return;
error:
	CU_FAIL();
}
#else
static void test_case_1()
{
	CU_FAIL();
}
#endif
CU_TestInfo EmulatorTest[] = {
	{"SYS+emuFS+emuENC", test_case_1},
	CU_TEST_INFO_NULL,
};
