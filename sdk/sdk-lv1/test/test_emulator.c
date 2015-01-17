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

#include <imp_sys.h>
#include <imp_log.h>

#define TAG "EmulatorTest"

#if 1

/* Fake functions */
static inline int isp_init() { return 0;}
static inline int isp_vb_init() { return 0;}
static inline int start_isp() { return 0;}
static inline int encoder_config() { return 0;}
static inline int encoder_enable_channel() { return 0;}
static inline int start_encoder() { return 0;}
static inline int ImpStreamOn() { return 0;}
static inline int ImpStreamOff() { return 0;}
static inline int save_stream() { return 0;}
static inline int bind() { return 0;}
static inline int Select() { return 0;}

static int ImpSystemInit()
{
	IMP_LOG_DBG(TAG, "ImpSystemInit\n");

	IMP_SYS_Init();

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

static int config_channel(int chn_num)
{
#if 0
	pool = creat_pool();

	set_channel_pool(chn_num, pool);
	set_channel_attr(chn_num);
#endif
	return 0;
}

static int ImpFrameSourceInit()
{
	int nr_chn = 2, i;

	/* open v4l2, and query the capacity*/
	isp_init(); /* Open, S_INPUT */

	isp_vb_init(); /* queue buffer */

	for (i = 0; i < nr_chn; i++) {
		config_channel(i); /* Size, output size, format */
	}

	start_isp(); /* Stream On */
	/* Init ISP here. */
	return 0;
}

static int ImpFrameSourceExit()
{
	return 0;
}

static int ImpEncoderInit()
{
	ImpEmuEncoderCreateGroup(0);
	ImpEmuEncoderCreateGroup(1);

	encoder_config();
	encoder_enable_channel();
	start_encoder();
	/* Init Encoder here. */
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

	IMP_SYS_Bind(&framesource1, &encoder1);
	IMP_SYS_Bind(&framesource2, &encoder2);

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
