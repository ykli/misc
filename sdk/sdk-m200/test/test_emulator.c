/*
 * Ingenic IMP SDK test main file.
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

#define TAG "EmulatorTest"

#if 1

/* Fake functions */
static inline int isp_init() { return 0;}
static inline int isp_vb_init() { return 0;}
static inline int start_isp() { return 0;}
static inline int encoder_config() { return 0;}
static inline int encoder_enable_channel() { return 0;}
static inline int start_encoder() { return 0;}
static inline int ImpStreamOff() { return 0;}
static inline int save_stream() { return 0;}
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

typedef struct {
	int width;
	int hight;
	int fps;
} FrameSourceInputAttr;

typedef struct {
	FrameSourceInputAttr InputAttr;
	int NumOfChannel;
	
} IMP_FrameSourceDevAttr;

typedef struct {
	int fps;
} FrameSourceChnAttr;

#if 0
get_group()
{

}

get_device()
{

}
#endif
#if 0
int get_framesource()
{
	return 0;
}
#endif
int IMP_FrameSourceGetMaxChn()
{
	return 0;
}

static int ImpFrameSourceInit()
{
	int ret;

	/* Configure Dev attr */
	IMPFSDevAttr imp_dev_attr;
	imp_dev_attr.inFrmRate = 25;
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

static int ImpEncoderInit()
{
	int ret;

	ret = IMP_EmuEncoder_CreateGroup(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuEncoder_CreateGroup(0) error: %d\n", ret);
		return -1;
	}

	ret = IMP_EmuEncoder_CreateGroup(1);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_EmuEncoder_CreateGroup(1) error: %d\n", ret);
		return -1;
	}

	encoder_config();
	encoder_enable_channel();
	start_encoder();
	IMP_LOG_DBG(TAG, "%s(): OK.\n", __func__);

	return 0;
}

static int ImpEncoderExit()
{
	return 0;
}

static int ImpBindModules()
{
	IMPChannel framesource1, framesource2;
	IMPChannel encoder1, encoder2;

	framesource1.devID = DEV_ID_EMU_FS;
	framesource1.grpID = 0;
	framesource1.chnID = 0;

	framesource2.devID = DEV_ID_EMU_FS;
	framesource2.grpID = 0;
	framesource2.chnID = 1;

	encoder1.devID = DEV_ID_EMU_ENC;
	encoder1.grpID = 0;
	encoder1.chnID = 0;

	encoder2.devID = DEV_ID_EMU_ENC;
	encoder2.grpID = 1;
	encoder2.chnID = 0;

	IMP_System_Bind(&framesource1, &encoder1);
	IMP_System_Bind(&framesource2, &encoder2);

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
	while (1)
		sleep(1);

	int nr_frame_to_encode = 10;
	while (nr_frame_to_encode--) {
		int timeout_cnt = 0;
		ret = Select();
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "select Encoder fd error: %s\n", strerror(errno));
			goto error;

		} else if (ret == 0) {
			IMP_LOG_ERR(TAG, "select Encoder fd timeout %d times\n", timeout_cnt);
			continue;

		} else {
//			IMP_VENC_GetStream();
			save_stream();
//			IMP_VENC_ReleaseStream();

			timeout_cnt = 0;
		}
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
